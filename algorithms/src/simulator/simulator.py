import sys, os, time
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
from src.dto.ArenaStringParser import ArenaStringParser
from src.simulator.robot_sprite import RobotSprite
import pygame
from src.dto.constants import *
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform
from src.agent.agent import Agent
from src.simulator.SimulationDisplay import SimulationDisplay

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
        self.arena = ArenaStringParser.parse_arena_string(arena_string)

        # send copies of arena and robot info so that simulator and agent do not modify a mutual copy
        self.agent = Agent(arena_string, self.robot_info.copy(), agent_task, END_COORD, waypoint)

    def step(self):
        # calculate agent percepts
        obstacle_list = [] # for fastest path the agent already knows all obstacle locations
        # get agent next move
        agent_output = self.agent.step(obstacle_list, self.robot_info)
        print(agent_output.get_message())
        move_command = agent_output.get_move_command()
        if move_command == None:
            self.quit()

        # update internal representation of robot
        new_orientation = OrientationTransform.calc_orientation_after_turn(self.robot_info.get_orientation(), move_command.get_turn_angle())
        self.robot_info.set_orientation(new_orientation)
        unit_move = OrientationTransform.orientation_to_unit_displacement(new_orientation)
        move = unit_move.multiply(move_command.get_cells_to_advance())
        self.robot_info.set_coord(self.robot_info.get_coord().add(move))
        # update pygame display
        self.update_display()

    def run(self):
        i = 0
        while i<99:
            self.events()
            self.step()
            time.sleep(0.5)
            i+=1

    def update_display(self):
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

            # Control robot using arrow keys
            # if event.type == pygame.KEYDOWN:
            #     if event.key == pygame.K_LEFT:
            #         new_coord = Coord(self.robot_info.get_coord().get_x() - 1, self.robot_info.get_coord().get_y())
            #         self.move(new_coord, Orientation.WEST)
            #         print("move left")
            #     if event.key == pygame.K_RIGHT:
            #         new_coord = Coord(self.robot_info.get_coord().get_x() + 1, self.robot_info.get_coord().get_y())
            #         self.move(new_coord, Orientation.EAST)
            #         print("move right")
            #     if event.key == pygame.K_UP:
            #         new_coord = Coord(self.robot_info.get_coord().get_x(), self.robot_info.get_coord().get_y()+1)
            #         self.move(new_coord, Orientation.NORTH)
            #         print("move up")
            #     if event.key == pygame.K_DOWN:
            #         new_coord = Coord(self.robot_info.get_coord().get_x(), self.robot_info.get_coord().get_y()-1)
            #         self.move(new_coord, Orientation.SOUTH)
            #         print("move down")

g = Simulator()
# Read the arena text file and store it as a list ==========================================
f = open("sample_arena.txt", "r") #import the arena file (this is for testing, for the actual we will have to import from RPi)
g.init(AgentTask.FAST, f.read(), WAYPOINT)
g.run()
