import sys, os, time
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
from src.dto.ArenaStringParser import ArenaStringParser
from src.dto.arena import Arena
from src.simulator.robot_sprite import RobotSprite
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
        self.robot_sprite = None
        self.agent = None
        self.arena = None
    
    def init(self, agent_task: AgentTask, arena_string: str, waypoint: Coord):
        self.robot_info = RobotInfo(START_COORD, Orientation.NORTH)
        self.sim_display = SimulationDisplay(self.robot_info)
        self.arena = ArenaStringParser.parse_arena_string(arena_string) #used in line 34 and 100
        # self.update_display()
        # arena and robot info are separate so that simulator and agent do not modify a mutual copy
        empty_arena_string = open("./algorithms/src/simulator/empty_arena_string.txt", "r").read()
        self.agent = Agent(empty_arena_string, self.robot_info.copy(), agent_task, END_COORD, waypoint)

    def step(self):
        # calculate agent percepts
        sis = SensorInputSimulation(self.robot_info, self.arena)
        obstacle_coord_list, no_obs_coord_list = sis.calculate_percepts()
        for coord in obstacle_coord_list:
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        for coord in no_obs_coord_list:
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        self.arena.get_cell_at_coord(self.robot_info.get_coord()).set_is_visited(True)
        # get agent next move
        agent_output = self.agent.step(obstacle_coord_list, no_obs_coord_list, self.robot_info)
        print(agent_output.get_message())
        move_command = agent_output.get_move_command()
        if move_command == None:
            self.quit()

        if self.coverage == self.arena.get_coverage_percentage():
            print("Coverage limit reached!")
            self.quit()

        # update internal representation of robot
        new_orientation = OrientationTransform.calc_orientation_after_turn(self.robot_info.get_orientation(), move_command.get_turn_angle())
        
        if new_orientation != self.robot_info.get_orientation():
            if abs(new_orientation.value - self.robot_info.get_orientation().value) == 1 or abs(new_orientation.value - self.robot_info.get_orientation().value) == 3:
                self.robot_info.set_orientation(new_orientation)
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()
                time.sleep(self.speed)
            elif abs(new_orientation.value - self.robot_info.get_orientation().value) == 2:
                self.robot_info.set_orientation(Orientation((new_orientation.value-1)%3))
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()
                time.sleep(self.speed)
                self.robot_info.set_orientation(new_orientation)
                self.robot_info.set_coord(self.robot_info.get_coord())
                self.update_display()
                time.sleep(self.speed)

        unit_move = OrientationTransform.orientation_to_unit_displacement(new_orientation)
        # move = unit_move.multiply(move_command.get_cells_to_advance()) # enable this if robot can move multiple squares
        move = unit_move # enable this if robot can move one square at a time
        self.robot_info.set_coord(self.robot_info.get_coord().add(move))

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
            self.coverage = 100

        if self.speed < 0:
            self.speed = 0.5

        if self.timelimit < 0:
            self.timelimit = 600 #default: 10 mins

        t = Timer(self.timelimit, self.timeout)
        self.time_ran_out = False
        t.start()

        i = 0
        while i<99:
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

    def quit(self):
        pygame.quit()
        quit()

    def events(self):
        for event in pygame.event.get():
            # Close the screen by pressing the x
            if event.type==pygame.QUIT:
                self.quit()

g = Simulator()
# Read the arena text file and store it as a list ==========================================
f = open("./algorithms/src/simulator/sample_arena.txt", "r") #import the arena file (this is for testing, for the actual we will have to import from RPi)
g.init(AgentTask.EXPLORE, f.read(), WAYPOINT)
g.run()
