import sys, os, time, json, math, threading
import zmq
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
import pygame
from src.agent.SensorParser import SensorParser
from src.agent.agent import Agent
from src.agent.agent import AgentOutput
from src.simulator.SimulationDisplay import SimulationDisplay
from src.dto.RobotInfo import RobotInfo
from src.dto.coord import Coord
from src.dto.constants import AgentTask, START_COORD, END_COORD, WAYPOINT, START_ORIENTATION, MAP_COL, MAP_ROW
from src.dto.MoveCommand import MoveCommand

HOST = "192.168.13.1"
CONNSTR_RX = f'tcp://{HOST}:3000'
CONNSTR_TX = f'tcp://{HOST}:3001'
CONNSTR_CONFIG = f'tcp://{HOST}:3002'

E_INIT = pygame.USEREVENT + 1
E_UPDATE = pygame.USEREVENT + 2

class AgentInterface:
    def __init__(self):
        self.agent = None
        self.agent_task = None
        self.waypoint = None
        self.q_size = 0 # queue of sent moves pending sensor response
        # connection to rpi
        self.context = zmq.Context()
        self.rx = self.context.socket(zmq.SUB) # pylint: disable=no-member
        self.rx.connect(CONNSTR_RX)
        self.tx = self.context.socket(zmq.PUSH) # pylint: disable=no-member
        self.tx.connect(CONNSTR_TX)
        self.rx.setsockopt_string(zmq.SUBSCRIBE, '') # pylint: disable=no-member
        print("Agent interface initialised")

    def main(self):
        i = 0
        while True:
            data = self.rx.recv_json()
            # print(data)

            print('qsize ', self.q_size)
            if data['type'] == 'sensor':
                if self.agent_task == AgentTask.EXPLORE or self.agent_task == AgentTask.IMAGEREC:
                    self.update_percepts(data)
                if self.q_size == 0:
                    self.step()
                print('got sensor data')
            elif data['type'] == 'move_done':
                self.q_size -= 1
                print('got move_done')
            elif data['type'] == 'start':
                self.q_size = 0
                if self.agent_task == AgentTask.FAST:
                    while not self.agent.get_robot_info().get_coord().is_equal(Coord(13, 18)):
                        self.update_percepts(None)
                        self.step()
                else:
                    self.step()
                print('got start')
            elif data['type'] == 'init':
                self.init(data)
                self.q_size = 1
                print('got init')
            elif data['type'] == 'waypoint':
                self.waypoint = Coord(data['data']['x'], data['data']['y'])
                print('got waypoint')
            elif data['type'] == 'ping':
                self.tx.send_json({'type': 'pong'})
            
            print('received message ', i)
            i += 1

    def init(self, init_data):
        arena_string, robot_info, agent_task, end_coord = self.parse_init_data(init_data)
        self.agent_task = agent_task
        self.agent = Agent(arena_string, robot_info, agent_task, end_coord, self.waypoint)
        self.agent.mark_robot_visisted_cells(self.agent.get_robot_info().get_coord()) # temp solution
        ev = pygame.event.Event(E_INIT, {
            'robot_info': robot_info
        })
        pygame.event.post(ev)
        self.update_simulation_display()
    
    def process_sensor_data(self, percept_data):
        # feed agent percepts
        print('\n\nprocessing percepts')
        if self.q_size == 0:
            robot_info = self.agent.get_expected_robot_info()
        elif self.q_size == 1:
            ri_old = self.agent.get_robot_info()
            ri_new = self.agent.get_expected_robot_info()
            robot_info = RobotInfo(ri_old.get_coord(), ri_new.get_orientation() if ri_new else ri_old.get_orientation())
        elif self.q_size < 0:
            raise Exception(f'move q-size: {self.q_size}\nreceived extra move done before sensor data!')
        else:
            raise Exception(f'move q-size: {self.q_size}\n2 last moves were not executed yet! \
                Is there desync between bot and algo?')

        if percept_data:
            obstacle_coord_list, no_obs_coord_list = SensorParser.main_sensor_parser(percept_data, robot_info)
        else:
            obstacle_coord_list, no_obs_coord_list = [], []
        print('obstacle list: ')
        obstacle_str = ''
        for coord in obstacle_coord_list:
            obstacle_str += f'({coord.get_x()}, {coord.get_y()}), '
        print(obstacle_str)
        return obstacle_coord_list, no_obs_coord_list
    
    def update_percepts(self, data):
        if data:
            obstacle_coord_list, no_obs_coord_list = self.process_sensor_data(data)
        else:
            obstacle_coord_list, no_obs_coord_list = [], []
        self.agent.calc_percepts(obstacle_coord_list, no_obs_coord_list, self.q_size)
        self.update_simulation_display()

        # serialize agent status
        agent_status = self.serialize_agent_status(self.agent.get_robot_info(), self.agent.get_arena())
        self.tx.send_json(agent_status)

    def step(self):
        # get agent output
        agent_output = self.agent.step()
        if self.agent_task == AgentTask.EXPLORE or self.agent_task == AgentTask.IMAGEREC:
            if not agent_output.get_move_command().get_turn_angle() == 0: # NONE TYPE HAS NO ATTRIBUTE GET TURN ANGLE
                self.q_size += 2
            else:
                self.q_size += 1
        self.log_agent_expected_move()

        # parse agent output
        message, turn_json, advance_json = self.serialize_agent_output(agent_output)

        # send messages
        self.tx.send_json(message)
        if turn_json:
            self.tx.send_json(turn_json)
        if advance_json:
            self.tx.send_json(advance_json)

    def log_agent_expected_move(self):
        coord = self.agent.get_expected_robot_info().get_coord()
        print(f'Go to {coord.get_x()}, {coord.get_y()}, facing {self.agent.get_expected_robot_info().get_orientation().name}')

    def parse_init_data(self, init_data):
        assert(init_data['type'] == 'init')
        # arena string
        mdf_string = init_data['data']['arena']['P2'] # p1 string ignored since always FFFF..
        arena_string = self.convert_mdf_to_binary(mdf_string) if init_data['data']['task'] == 'FP' else None

        # robot info
        robot_info = RobotInfo(START_COORD, START_ORIENTATION)

        # agent task
        if init_data['data']['task'] == 'FP':
            agent_task = AgentTask.FAST
        elif init_data['data']['task'] == 'EX':
            agent_task = AgentTask.EXPLORE
        elif init_data['data']['task'] == 'IR':
            agent_task = AgentTask.IMAGEREC
        else: raise Exception('Invalid agent task: ' + init_data['data']['task'])

        # end_coord
        end_coord = END_COORD

        return arena_string, robot_info, agent_task, end_coord

    def serialize_agent_status(self, robot_info, arena):
        p1, p2 = self.convert_arena_to_mdf(arena)

        agent_status = {
            'type': 'status',
            'data': {
                'robot_info': {
                    'x': robot_info.get_coord().get_x(),
                    'y': robot_info.get_coord().get_y(),
                    'orientation': robot_info.get_orientation().name
                },
                'internal_arena': {
                    'P1': p1,
                    'P2': p2
                }
            }
        }
        return agent_status

    def serialize_agent_output(self, agent_output: AgentOutput):
        does_turn = True
        does_move = True
        message = {
            'type': 'message',
            'message': agent_output.get_message()
        }
        move_command = agent_output.get_move_command()

        if not move_command:
            return message, None, None

        # format turn angle
        turn_angle = agent_output.get_move_command().get_turn_angle()
        if turn_angle == 0:
            does_turn = False
        elif turn_angle > 180:
            f_turn_angle = 360 - turn_angle
            turn_direction = 'left'
        else:
            f_turn_angle = turn_angle
            turn_direction = 'right'

        # format cells to advance
        cells_to_advance = agent_output.get_move_command().get_cells_to_advance()
        if cells_to_advance == 0:
            does_move = False
        elif cells_to_advance < 0:
            f_cta = abs(cells_to_advance)
            move_direction = 'backward'
        else:
            f_cta = cells_to_advance
            move_direction = 'forward'

        turn_json = {
            'type': 'turn', 
            'data': {
                'turn': f_turn_angle,
                'direction': turn_direction
            }
        } if does_turn else None

        advance_json = {
            'type': 'advance', 
            'data': {
                'advance': f_cta,
                'direction': move_direction
            }
        } if does_move else None

        return message, turn_json, advance_json

    def convert_arena_to_mdf(self, arena):
        explored_bin_str = "11"
        obstacle_bin_str = ""

        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                coord = Coord(x,y)
                if arena.get_cell_at_coord(coord).is_explored():
                    explored_bin_str += "1"
                    if arena.get_cell_at_coord(coord).is_obstacle():
                        obstacle_bin_str += "1"
                    else:
                        obstacle_bin_str += "0"
                else:
                    explored_bin_str += "0"

        explored_bin_str += "11"

        if len(obstacle_bin_str) % 8 != 0:
            num_pad_bits = 8 - len(obstacle_bin_str) % 8
            obstacle_bin_str += "0" * num_pad_bits 

        explored_hex_str = f"{int(explored_bin_str, 2):X}" # explored bin string is never empty (only 0s)

        if obstacle_bin_str:
            obstacle_hex_str = f"{int(obstacle_bin_str, 2):X}"
            num_pad_bits = math.ceil(len(obstacle_bin_str) / 4) - len(obstacle_hex_str)
            padded_obstacle_hex_str = "0" * num_pad_bits + obstacle_hex_str
        else:
            padded_obstacle_hex_str = ""

        return explored_hex_str, padded_obstacle_hex_str
    
    def convert_mdf_to_binary(self, readmap):
        #input hex string
        bin_str = "{:b}".format(int(readmap, 16))
        num_pad_bits = len(readmap) * 4 - len(bin_str)
        readmap_bin = "0" * num_pad_bits + bin_str
        
        #remove the last padding 4 binary bits
        readmap_bin = readmap_bin[:-4]

        #mirror the binary string so that the map will generate correctly
        readmap_bin_1 = [readmap_bin[i:i+15] + '\n' for i in range(0, len(readmap_bin), 15)]
        readmap_bin_2 = ''.join(readmap_bin_1)
        lines = readmap_bin_2.split("\n")
        reordered = lines[::-1]
        readmap_final = "\n".join(reordered)
        return readmap_final

    def update_simulation_display(self):
        ev = pygame.event.Event(E_UPDATE, {
            'arena': self.agent.get_arena(),
            'robot_info': self.agent.get_robot_info()
        })
        pygame.event.post(ev)

def handle_ui():
    pygame.init()
    pygame.display.set_caption('Arena Simulator')

    sim_display = None

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return
            elif event.type == E_INIT:
                sim_display = SimulationDisplay(event.robot_info)
            elif event.type == E_UPDATE:
                sim_display.draw(event.arena, event.robot_info)
                pygame.display.update()

def test_connection():

    connected = False

    time.sleep(0.1)
    while True:
        sock = zmq.Context().socket(zmq.REQ)
        sock.setsockopt(zmq.RCVTIMEO, 1000)
        sock.connect(CONNSTR_CONFIG)
        sock.send_string("ping")
        try:
            sock.recv()
            if not connected:
                print("Connected")
                connected = True
            time.sleep(1)
        except zmq.error.Again:
            print("Connection timeout")
            connected = False

threading.Thread(target=AgentInterface().main, daemon=True).start()
threading.Thread(target=test_connection, daemon=True).start()
handle_ui()
