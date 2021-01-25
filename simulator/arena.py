from map import Grid
from robot import Robot
import pygame
from pygame.locals import *

blue=[0,0,255]
white = [255, 255, 255]
black = [0,0,0]
grey = [100,100,100]
red = [255,0,0]
green = [0,255,0]
# TODO standardise this across all files
map_row = 20 
map_col = 15

class Arena:
    def __init__(self):
        pygame.init()
        self.dis = pygame.display.set_mode((300,500))
        pygame.display.set_caption('arena simulator')

    def run(self):
        running=True
        while running:
            self.events()
            self.update()
            self.display()
    
    def new(self):
        self.player = Robot(self, 30, 370)

    def draw_grid(self):
        # Display coloured boxes to indicate obstacles, start and end points
        for row in range(map_row):
            for col in range(map_col):
                if Grid.grid[row][col].getObstacle() == True:
                    pygame.draw.rect(self.dis, black, [col*20, row*20, 20, 20])

                if 0<=row<=2 and 12<=col<=14:
                    pygame.draw.rect(self.dis, red, [col*20, row*20, 20, 20])

                if 17<=row<=19 and 0<=col<=2:
                    pygame.draw.rect(self.dis, green, [col*20, row*20, 20, 20])

        # Draw the lines for the grid
        for x in range(0,320,20):
            pygame.draw.line(self.dis, grey, (x,0), (x,400))
        for y in range(0,420,20):
            pygame.draw.line(self.dis, grey, (0,y), (400,y))

    def display(self):
        self.dis.fill(white)
        self.draw_grid()
        self.player.draw(self.dis)
        pygame.display.update() 
        #pygame.display.flip()

    def update(self):
        self.player.update()

    def quit(self):
        pygame.quit()
        quit()

    def events(self):
        for event in pygame.event.get():
            if event.type==pygame.QUIT:
                self.quit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_LEFT:
                    self.player.move(dx=-20)
                    print("move left")
                if event.key == pygame.K_RIGHT:
                    self.player.move(dx=20)
                    print("move right")
                if event.key == pygame.K_UP:
                    self.player.move(dy=-20)
                    print("move up")
                if event.key == pygame.K_DOWN:
                    self.player.move(dy=20)
                    print("move down")


g = Arena()
while True:
    g.new()
    g.run()