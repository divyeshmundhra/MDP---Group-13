# from lib import pygame
import pygame #change this later
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

    def move(self, dx=0, dy=0):
        self.x += dx
        self.y += dy

    def update(self):
        self.rect.x = self.x * TILE_SIZE
        self.rect.y = self.y * TILE_SIZE

    def draw(self, dis):
        pygame.draw.circle(dis, yellow, (self.x,self.y), 30)