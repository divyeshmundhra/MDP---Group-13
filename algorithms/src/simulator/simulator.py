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
        obstacle_coord_list, no_obs_coord_list = self.calculate_percepts() 
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

        # update pygame display
        self.update_display()

    def run(self):
        self.speed = 1 / float(input("Enter robot speed (steps per second) (-1 for default): "))
        if self.speed < 0:
            self.speed = 0.5
        i = 0
        while i<99:
            self.events()
            self.step()
            time.sleep(self.speed)
            i+=1
        print(f'Run timed-out after {i+1} steps')
    
    def calculate_percepts(self):
        coord = self.robot_info.get_coord()
        obstacles_coord_list = self.arena.get_adjacent_blocked(coord)
        no_obs_coord_list = self.arena.get_adjacent_unblocked(coord)
        return obstacles_coord_list, no_obs_coord_list

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
f = open("./algorithms/src/simulator/sample_arena.txt", "r") #import the arena file (this is for testing, for the actual we will have to import from RPi)
g.init(AgentTask.EXPLORE, f.read(), WAYPOINT)
g.run()
