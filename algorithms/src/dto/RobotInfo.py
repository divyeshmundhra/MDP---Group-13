from src.dto.constants import Orientation
from src.dto.coord import Coord

class RobotInfo:
    def __init__(self, coord, orientation):
        self.coord = coord
        self.orientation = orientation

    def get_coord(self) -> Coord:
        return self.coord
    
    def set_coord(self, coord: Coord) -> None:
        self.coord = coord

    def get_orientation(self) -> Orientation:
        return self.orientation

    def set_orientation(self, orientation: Orientation) -> None:
        self.orientation = orientation

    def copy(self):
        return RobotInfo(self.coord, self.orientation)
