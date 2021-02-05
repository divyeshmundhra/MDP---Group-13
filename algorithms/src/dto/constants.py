from enum import Enum
# Defining colours
blue=[0,0,255]
white = [255, 255, 255]
black = [0,0,0]
grey = [100,100,100]
red = [255,0,0]
green = [0,255,0]
yellow = [255,255,0]

# Defining map dimensions
MAP_ROW = 20
MAP_COL = 15
TILE_SIZE = 20
DIS_X = 300
DIS_Y = 400

class Orientation(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

class AgentTask(Enum):
    FAST = 0
    EXPLORE = 1
