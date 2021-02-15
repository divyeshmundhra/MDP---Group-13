from enum import Enum
from src.dto.coord import Coord
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

# Defining start, end and waypoint coord
START_COORD = Coord(1,1)
END_COORD = Coord(13,18)
WAYPOINT = Coord(2,2)

class Orientation(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

class AgentTask(Enum):
    FAST = 0
    EXPLORE = 1

class TimeCosts:
    QUARTER_TURN = 1
    MOVE_ONE_UNIT = 1
    # extendable: predefined times for larger turns and moves? linear equation to calculate time for larger turns and moves?
