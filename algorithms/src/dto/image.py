from constants import Orientation

class Image:
    def __init__(self, id, coord, orientation):
        self.id = id
        self.coord = coord
        self.orientation = orientation

    def get_coord(self):
        return self.coord

    def get_orientation(self):
        return self.orientation