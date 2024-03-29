import sys, os, time, math
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
from src.dto.ArenaStringParser import ArenaStringParser
from src.dto.arena import Arena
import pygame
from src.dto.constants import *
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform
from src.agent.agent import Agent
from src.simulator.SimulationDisplay import SimulationDisplay
from src.simulator.SensorInputSimulation import SensorInputSimulation
from threading import Timer

class Simulator:

    def __init__(self):
        # display
        pygame.init()
        self.sim_display = None
        pygame.display.set_caption('arena simulator')

        # data
        self.robot_info = None
        self.agent = None
        self.arena = None
        self.full_path_step_count = 0
    
    def init(self, agent_task: AgentTask, arena_string: str, waypoint: Coord):
        self.robot_info = RobotInfo(START_COORD, Orientation.NORTH)
        self.sim_display = SimulationDisplay(self.robot_info)
        self.arena = ArenaStringParser.parse_arena_string(arena_string) #used in line 34 and 100
        # self.update_display()
        # arena and robot info are separate so that simulator and agent do not modify a mutual copy
        if agent_task == AgentTask.FAST:
            self.agent = Agent(arena_string, self.robot_info.copy(), agent_task, END_COORD, waypoint)
        else:
            empty_arena_string = open("./algorithms/src/simulator/empty_arena_string.txt", "r").read()
            self.agent = Agent(empty_arena_string, self.robot_info.copy(), agent_task, END_COORD, waypoint)

    def step(self):
        # calculate agent percepts
        sis = SensorInputSimulation(self.robot_info, self.arena)
        obstacle_coord_list, no_obs_coord_list = sis.calculate_percepts()
        for coord, _ in obstacle_coord_list: # distance is not used, hence renamed to _
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        for coord, _ in no_obs_coord_list:
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        self.arena.get_cell_at_coord(self.robot_info.get_coord()).set_is_visited(True)

        if ALGO_RETURNS_FULL_PATH:
            if self.full_path_step_count >= len(self.agent_output_full_path):
                move_command = None
            else:
                agent_output = self.agent_output_full_path[self.full_path_step_count]
                if agent_output.get_message():
                    print(agent_output.get_message())
                move_command = agent_output.get_move_command()
                self.full_path_step_count += 1
        else:
            # get agent next move
            self.agent.calc_percepts(obstacle_coord_list, no_obs_coord_list)
            agent_output = self.agent.step()
            print(agent_output.get_message())
            move_command = agent_output.get_move_command()

        if move_command is None:
            self.quit()

        if self.coverage <= self.arena.get_coverage_percentage():
            print("Coverage limit reached!")
            if self.robot_info.get_coord().is_equal(START_COORD):
                self.quit() # when exploring we need to go back to start point, so don't quit before

        self.update_robot_info(move_command)

        if self.time_ran_out:
            print("Time's up!")
            self.quit()

        # update pygame display
        self.update_display()

    def timeout(self):
        self.time_ran_out = True

    def run(self):
        self.speed = 1 / float(input("Enter robot speed (steps per second) (-1 for default): "))
        self.coverage = int(input("Enter coverage limit (%) (-1 for default): "))
        self.timelimit = float(input("Enter time limit (sec) (-1 for default): "))

        if self.coverage < 0:
            self.coverage = 1000

        if self.speed < 0:
            self.speed = 0.5

        if self.timelimit < 0:
            self.timelimit = 600 #default: 10 mins

        t = Timer(self.timelimit, self.timeout)
        self.time_ran_out = False
        t.start()

        if ALGO_RETURNS_FULL_PATH:
            self.agent_output_full_path = self.agent.get_agent_output_list()

        i = 0
        while i<200:
            self.events()
            self.step()
            time.sleep(self.speed)
            i+=1
        print(f'Run timed-out after {i+1} steps')

    def update_display(self):
        if ARENA_DISPLAY_MODE == ArenaDisplayMode.OBSERVED:
            seen_arena = self.agent.get_arena()
            self.sim_display.draw(seen_arena, self.robot_info)
        else:
            self.sim_display.draw(self.arena, self.robot_info)
        pygame.display.update() 
    
    def update_robot_info(self, move_command):
        # update internal representation of robot
        new_orientation = OrientationTransform.calc_orientation_after_turn(self.robot_info.get_orientation(), move_command.get_turn_angle())
        
        if new_orientation != self.robot_info.get_orientation():
            if abs(new_orientation.value - self.robot_info.get_orientation().value) == 1 or abs(new_orientation.value - self.robot_info.get_orientation().value) == 3:
                self.robot_info.set_orientation(new_orientation)
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()
            elif abs(new_orientation.value - self.robot_info.get_orientation().value) == 2:
                self.robot_info.set_orientation(Orientation((new_orientation.value-1)%3))
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()
                self.robot_info.set_orientation(new_orientation)
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()

        unit_move = OrientationTransform.orientation_to_unit_displacement(new_orientation)
        move = unit_move.multiply(move_command.get_cells_to_advance()) # enable this if robot can move multiple squares
        self.robot_info.set_coord(self.robot_info.get_coord().add(move))

    def quit(self):
        self.arena = self.agent.get_arena() # cheeky patch to let our agent fill in unexplored cells as obstacles
        print('debug number of unexplored cells: ', len(self.arena.list_unexplored_cells()))
        print('debug number of explored cells: ', 300-len(self.arena.list_unexplored_cells()))
        self.print_mdf()
        self.update_display()
        self.update_display()
        print('Quitting...')
        time.sleep(5)
        pygame.quit()
        quit()

    def events(self):
        for event in pygame.event.get():
            # Close the screen by pressing the x
            if event.type==pygame.QUIT:
                self.quit()

    def print_mdf(self):
        explored_bin_str = "11"
        obstacle_bin_str = ""

        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                coord = Coord(x,y)
                if self.arena.get_cell_at_coord(coord).is_explored():
                    explored_bin_str += "1"
                    if self.arena.get_cell_at_coord(coord).is_obstacle():
                        obstacle_bin_str += "1"
                    else:
                        obstacle_bin_str += "0"
                else:
                    explored_bin_str += "0"

        explored_bin_str += "11"

        if len(obstacle_bin_str) % 8 != 0:
            num_pad_bits = 8 - len(obstacle_bin_str) % 8
            obstacle_bin_str += "0" * num_pad_bits 

        explored_hex_str = f"{int(explored_bin_str, 2):X}"
        print(explored_hex_str)

        obstacle_hex_str = f"{int(obstacle_bin_str, 2):X}"
        num_pad_bits = math.ceil(len(obstacle_bin_str) / 4) - len(obstacle_hex_str)
        print("0" * num_pad_bits + obstacle_hex_str)

def input_hex(readmap):
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

g = Simulator()
# Read the arena text file and store it as a list ==========================================
# f = open("./algorithms/src/simulator/sample_arena.txt", "r")
f = open("./algorithms/src/simulator/MDF_string_3.txt", "r")

# load from binary
# g.init(AgentTask.EXPLORE, f.read(), WAYPOINT)

# load from MDF
g.init(AgentTask.IMAGEREC, input_hex(f.read()), WAYPOINT)
g.run()
