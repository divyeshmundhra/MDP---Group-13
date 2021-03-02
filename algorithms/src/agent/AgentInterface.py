import sys, os, time, json, math
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

class AgentInterface:
    def __init__(self):
        self.agent = None
        # display robot internal representation of arena
        pygame.init()
        self.sim_display = None
        pygame.display.set_caption('arena simulator')
        self.robot_sprite = None
        # connection to rpi
        self.context = zmq.Context()
        self.rx = self.context.socket(zmq.SUB) # pylint: disable=no-member
        self.rx.connect("tcp://192.168.13.1:3000")
        self.tx = self.context.socket(zmq.PUSH) # pylint: disable=no-member
        self.tx.connect("tcp://192.168.13.1:3001")
        self.rx.setsockopt_string(zmq.SUBSCRIBE, '') # pylint: disable=no-member
        print("connected")

    def main(self):
        i = 0
        while True:
            data = self.rx.recv_json()
            # print(data)
            if data['type'] == 'sensor':
                self.step(data)
            elif data['type'] == 'start':
                self.step(None) # start signal doesn't come with sensor data
            elif data['type'] == 'init':
                self.init(data)
            print('received message ', i)
            i += 1

    def init(self, init_data):
        arena_string, robot_info, agent_task, end_coord, waypoint = self.parse_init_data(init_data)
        self.agent = Agent(arena_string, robot_info, agent_task, end_coord, waypoint)
        self.sim_display = SimulationDisplay(robot_info)
        self.update_simulation_display()

    def step(self, step_data):
        # feed agent percepts
        coord = self.agent.get_robot_info().get_coord()
        print(f'{coord.get_x()}, {coord.get_y()}, {self.agent.get_robot_info().get_orientation().name}')
        robot_info = self.agent.get_robot_info()
        if step_data:
            obstacle_coord_list, no_obs_coord_list = SensorParser.main_sensor_parser(step_data, robot_info)
        else:
            obstacle_coord_list, no_obs_coord_list = [], []
        self.agent.calc_percepts(obstacle_coord_list, no_obs_coord_list)
        self.update_simulation_display()

        # serialize agent status
        agent_status = self.serialize_agent_status(self.agent.get_robot_info(), self.agent.get_arena())

        # get agent output
        agent_output = self.agent.step()

        # parse agent output
        message, turn_json, advance_json = self.serialize_agent_output(agent_output)

        # send messages
        self.tx.send_json(message)
        self.tx.send_json(agent_status)
        if turn_json:
            self.tx.send_json(turn_json)
        if advance_json:
            self.tx.send_json(advance_json)
        
        # self.update_simulation_display()

    def parse_init_data(self, init_data):
        assert(init_data['type'] == 'init')
        # arena string
        mdf_string = init_data['data']['arena']['P2'] # p1 string ignored since always FFFF..
        arena_string = self.convert_mdf_to_binary(mdf_string) if init_data['data']['task'] == 'EX' else None

        # robot info
        robot_info = RobotInfo(START_COORD, START_ORIENTATION)

        # agent task
        if init_data['data']['task'] == 'FP':
            agent_task = AgentTask.FAST
        elif init_data['data']['task'] == 'EX':
            agent_task = AgentTask.EXPLORE
        else: raise Exception('Invalid agent task: ' + init_data['data']['task'])

        # end_coord
        end_coord = END_COORD

        # waypoint
        waypoint = WAYPOINT

        return arena_string, robot_info, agent_task, end_coord, waypoint

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

        if explored_bin_str:
            explored_hex_str = f"{int(explored_bin_str, 2):X}"
        else:
            explored_hex_str = "0" * 76

        if obstacle_bin_str:
            obstacle_hex_str = f"{int(obstacle_bin_str, 2):X}"
            num_pad_bits = math.ceil(len(obstacle_bin_str) / 4) - len(obstacle_hex_str)
            padded_obstacle_hex_str = "0" * num_pad_bits + obstacle_hex_str
        else:
            padded_obstacle_hex_str = "0" * len(explored_hex_str)

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
        seen_arena = self.agent.get_arena()
        self.sim_display.draw(seen_arena, self.agent.get_robot_info())
        pygame.display.update()

AgentInterface().main()
