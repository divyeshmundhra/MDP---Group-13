import sys, os
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
from src.simulator.ArenaStringParser import Grid
from src.simulator.robot_sprite import RobotSprite
import pygame
from src.dto.constants import *
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo

class Simulator:
    def __init__(self):
        pygame.init()
        self.dis = pygame.display.set_mode((DIS_X,500))
        pygame.display.set_caption('arena simulator')

    def run(self):
        running=True
        while running:
            self.events()
            self.update()
            self.display()
    
    def init_robot(self):
        start_coord = Coord(1, 1)
        self.robot_info = RobotInfo(start_coord, Orientation.NORTH)
        self.robot_sprite = RobotSprite(self, self.robot_info)

    def draw_grid(self):
        # Display coloured boxes to indicate obstacles, start and end points
        for row in range(MAP_ROW):
            for col in range(MAP_COL):
                coord = Coord(col, row)
                if Grid.arena.get_cell_at_coord(coord).get_obstacle_flag() == True:
                    pygame.draw.rect(self.dis, black, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 0<=row<=2 and 12<=col<=14:
                    pygame.draw.rect(self.dis, red, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 17<=row<=19 and 0<=col<=2:
                    pygame.draw.rect(self.dis, green, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

        # Draw the lines for the grid
        for x in range(0,DIS_X+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (x,0), (x,DIS_Y))
        for y in range(0,DIS_Y+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (0,y), (DIS_X,y))

    def display(self):
        self.dis.fill(white)
        self.draw_grid()
        self.robot_sprite.draw(self.dis)
        pygame.display.update() 

    def update(self):
        self.robot_sprite.update()

    def quit(self):
        pygame.quit()
        quit()

    def events(self):
        for event in pygame.event.get():
            # Close the screen by pressing the x
            if event.type==pygame.QUIT:
                self.quit()

            # Control robot using arrow keys
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_LEFT:
                    self.robot_sprite.move(dx=-1)
                    print("move left")
                if event.key == pygame.K_RIGHT:
                    self.robot_sprite.move(dx=1)
                    print("move right")
                if event.key == pygame.K_UP:
                    self.robot_sprite.move(dy=1)
                    print("move up")
                if event.key == pygame.K_DOWN:
                    self.robot_sprite.move(dy=-1)
                    print("move down")

        #TODO: call the agent at each step, passing in the current RobotInfo and known ArenaInfo (aka the arena object)
        # agent then uses the ArenaInfo to calculate move_command and pass it back to RobotInfo
        # using the new RobotInfo, update the robot_sprite display


g = Simulator()
while True:
    g.init_robot()
    g.run()