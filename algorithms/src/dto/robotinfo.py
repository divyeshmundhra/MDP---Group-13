from constants import Orientation

class Robotinfo:
    def __init__(self, coord, orientation):
        self.coord = coord
        self.orientation = orientation

    def get_Coord(self):
        return self.coord
    
    def set_Coord(self, coord):
        self.coord = coord

    def get_Orientation(self):
        return self.orientation