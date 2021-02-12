# from src.dto import Arena, Coord
from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.RobotInfo import RobotInfo

class ExplorationAlgo:
    def __init__(self):
        pass

    @staticmethod
    # def get_next_step(self, arena: Arena) -> Coord:
    def get_next_step(arena: Arena, robot_info: RobotInfo, end: Coord, waypoint: Coord) -> Coord:
        # pass
        coord = Coord(10,10) #just to test
        return coord #just to test