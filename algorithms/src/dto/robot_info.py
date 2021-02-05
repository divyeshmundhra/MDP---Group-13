from src.dto.constants import Orientation

class Robotinfo:
    def __init__(self, coord, orientation):
        self.coord = coord
        self.orientation = orientation

    def get_coord(self):
        return self.coord
    
    def set_coord(self, coord):
        self.coord = coord

    def get_orientation(self) -> Orientation:
        return self.orientation