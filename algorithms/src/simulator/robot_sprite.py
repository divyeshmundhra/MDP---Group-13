import pygame
from src.dto.constants import *

class RobotSprite:
    def __init__(self, game, robot_info): # Pass the robot object as a parameter so the sprite can retrieve the details
        pygame.sprite.Sprite.__init__(self)
        self.game = game
        self.image = pygame.Surface((30, 30))
        self.image.fill(yellow)
        self.rect = self.image.get_rect()
        self.x = robot_info.get_coord().get_x()
        self.y = robot_info.get_coord().get_y()
        self.orientation = robot_info.get_orientation()

    def move(self, robot_info):
        self.x = robot_info.get_coord().get_x()
        self.y = robot_info.get_coord().get_y()
        self.orientation = robot_info.get_orientation()

    def update(self):
        self.rect.x = self.x * TILE_SIZE
        self.rect.y = self.y * TILE_SIZE

    def draw(self, dis):
        pygame.draw.circle(dis, yellow, (self.x*20+10, 400-self.y*20-10), 30)  #start at (30,370)

        #takes in the current pos of the robot and faces the direction based on the orientation
        if self.orientation == Orientation.NORTH:
            pygame.draw.circle(dis, blue, (self.x*20+10, 400-self.y*20-10-20), 5)
        elif self.orientation == Orientation.EAST:
            pygame.draw.circle(dis, blue, (self.x*20+10+20, 400-self.y*20-10), 5)
        elif self.orientation == Orientation.SOUTH:
            pygame.draw.circle(dis, blue, (self.x*20+10, 400-self.y*20-10+20), 5)
        elif self.orientation == Orientation.WEST:
            pygame.draw.circle(dis, blue, (self.x*20+10-20, 400-self.y*20-10), 5)