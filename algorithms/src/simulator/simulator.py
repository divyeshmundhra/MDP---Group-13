
import sys, os
sys.path.append(os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(__file__)))))

from src.simulator.ArenaStringParser import Grid
from src.simulator.robot_sprite import RobotSprite
from lib import pygame
from lib.pygame.locals import *
from src.dto.constants import *

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
    
    def new(self):
        self.robotold = RobotSprite(self, 30, 370)

    def draw_grid(self):
        # Display coloured boxes to indicate obstacles, start and end points
        for row in range(MAP_ROW):
            for col in range(MAP_COL):
                if Grid.grid[row][col].getObstacle() == True:
                    pygame.draw.rect(self.dis, black, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 0<=row<=2 and 12<=col<=14:
                    pygame.draw.rect(self.dis, red, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 17<=row<=19 and 0<=col<=2:
                    pygame.draw.rect(self.dis, green, [col*TILE_SIZE, row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

        # Draw the lines for the grid
        for x in range(0,DIS_X+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (x,0), (x,DIS_Y))
        for y in range(0,DIS_Y+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (0,y), (DIS_Y,y))

    def display(self):
        self.dis.fill(white)
        self.draw_grid()
        self.robotold.draw(self.dis)
        pygame.display.update() 
        #pygame.display.flip()

    def update(self):
        self.robotold.update()

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
                    self.robotold.move(dx=-TILE_SIZE)
                    print("move left")
                if event.key == pygame.K_RIGHT:
                    self.robotold.move(dx=TILE_SIZE)
                    print("move right")
                if event.key == pygame.K_UP:
                    self.robotold.move(dy=-TILE_SIZE)
                    print("move up")
                if event.key == pygame.K_DOWN:
                    self.robotold.move(dy=TILE_SIZE)
                    print("move down")


g = Simulator()
while True:
    g.new()
    g.run()