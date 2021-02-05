from constants import Orientation

class Image:
    def __init__(self, id, coord, orientation):
        self.id = id
        self.coord = coord
        self.orientation = orientation

    def getCoord(self):
        return self.coord

    def getOrientation(self):
        return self.orientation