from enum import Enum
from src.dto.coord import Coord
# Defining colours
blue=[0,0,255]
white = [255, 255, 255]
black = [0,0,0]
light_grey = [170,170,170]
grey = [100,100,100]
red = [255,0,0]
green = [0,255,0]
yellow = [255,255,0]
purple = [138,43,226]

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

# Define robot vision range (sensor range), we currently assume equal distances in each direction
VIEW_RANGE = 2

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

class SensorRanges:
    # (position on robot)_(direction)
    # LEFT_FW means forward looking sensor on left side of robot (displaced by -1,0 from center of robot) 
    LEFT_FW = 2
    CENTER_FW = 1
    RIGHT_FW = 2
    FRONT_LEFT = 1
    BACK_LEFT = 1
    BACK_RIGHT = 1

