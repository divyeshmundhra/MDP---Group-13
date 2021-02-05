from constants import Orientation

class Image:
    def __init__(self, image_id, coord, orientation):
        self.image_id = image_id
        self.coord = coord
        self.orientation = orientation

    def get_coord(self):
        return self.coord

    def get_orientation(self):
        return self.orientation