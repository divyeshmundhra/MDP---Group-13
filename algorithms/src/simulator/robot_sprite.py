from lib import pygame
from src.dto.constants import *

class RobotSprite:
    def __init__(self, game, x, y):
        pygame.sprite.Sprite.__init__(self)
        self.game = game
        self.image = pygame.Surface((30, 30))
        self.image.fill(yellow)
        self.rect = self.image.get_rect()
        self.x = x
        self.y = y

    def move(self, dx=0, dy=0):
        self.x += dx
        self.y += dy

    def update(self):
        self.rect.x = self.x * tile_size
        self.rect.y = self.y * tile_size

    def draw(self, dis):
        pygame.draw.circle(dis, yellow, (self.x,self.y), 30)