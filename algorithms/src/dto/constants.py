from enum import Enum
from src.dto.coord import Coord
# Defining colours
blue = [0, 0, 255]
white = [255, 255, 255]
black = [0, 0, 0]
light_grey = [170, 170, 170]
grey = [100, 100, 100]
red = [255, 0, 0]
green = [0, 255, 0]
yellow = [255, 255, 0]
purple = [138, 43, 226]

# Defining map dimensions
MAP_ROW = 20
MAP_COL = 15
TILE_SIZE = 20
DIS_X = 300
DIS_Y = 400

# Defining start, end and waypoint coord
START_COORD = Coord(1, 1)
END_COORD = Coord(13, 18)
WAYPOINT = Coord(2, 2)

class ArenaDisplayMode(Enum):
    OBSERVED = 0
    TRUE = 1


ARENA_DISPLAY_MODE = ArenaDisplayMode.OBSERVED


class Orientation(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

START_ORIENTATION = Orientation.NORTH

class AgentTask(Enum):
    FAST = 0
    EXPLORE = 1


class TimeCosts:
    QUARTER_TURN = 1
    MOVE_ONE_UNIT = 1
    # extendable: predefined times for larger turns and moves? linear equation to calculate time for larger turns and moves?


SENSOR_CONSTANTS = {
    "FORWARD_FRONT_LEFT": {
        "range": 3,
        "direction": 0,  # degrees from forward face of robot
        "displacement_0": Coord(-1, 1),
        "displacement_1": Coord(1, 1),
        "displacement_2": Coord(1, -1),
        "displacement_3": Coord(-1, -1),
    },
    "FORWARD_FRONT_MID": {
        "range": 3,
        "direction": 0,
        "displacement_0": Coord(0, 1),
        "displacement_1": Coord(1, 0),
        "displacement_2": Coord(0, -1),
        "displacement_3": Coord(-1, 0)
    },
    "FORWARD_FRONT_RIGHT": {
        "range": 3,
        "direction": 0,
        "displacement_0": Coord(1, 1),
        "displacement_1": Coord(1, -1),
        "displacement_2": Coord(-1, -1),
        "displacement_3": Coord(-1, 1)
    },
    "LEFT_FRONT": {
        "range": 3,
        "direction": 270,
        "displacement_0": Coord(-1, 1),
        "displacement_1": Coord(1, 1),
        "displacement_2": Coord(1, -1),
        "displacement_3": Coord(-1, -1)
    },
    "RIGHT_FRONT": {
        "range": 3,
        "direction": 90,
        "displacement_0": Coord(1, 1),
        "displacement_1": Coord(1, -1),
        "displacement_2": Coord(-1, -1),
        "displacement_3": Coord(-1, 1)
    },
    "LEFT_REAR": {
        "range": 3,
        "direction": 270,
        "displacement_0": Coord(-1, -1),
        "displacement_1": Coord(-1, 1),
        "displacement_2": Coord(1, 1),
        "displacement_3": Coord(1, -1)
    }
}

# class SensorRanges:
#     # (position on robot)_(direction)
#     # LEFT_FW means forward looking sensor on left side of robot (displaced by -1,0 from center of robot)
#     LEFT_FW = 5
#     CENTER_FW = 5
#     RIGHT_FW = 5
#     FRONT_LEFT = 5
#     BACK_LEFT = 5
#     FRONT_RIGHT = 5

# class SensorDisplacements:
#     LEFT_FW = Coord(-1, 1)
#     CENTER_FW = Coord(0, 1)
#     RIGHT_FW = Coord(1, 1)
#     FRONT_LEFT = Coord(-1, 1)
#     BACK_LEFT = Coord(-1, -1)
#     FRONT_RIGHT = Coord(1, 1)
