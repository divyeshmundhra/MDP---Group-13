from src.dto.constants import Orientation
from src.dto import Coord

class Image:
    def __init__(self, image_id: int, coord: Coord, orientation: Orientation):
        self.image_id = image_id
        self.coord = coord
        self.orientation = orientation

    def get_image_id(self) -> int:
        return self.image_id

    def get_coord(self) -> Coord:
        return self.coord

    def get_orientation(self) -> Orientation:
        return self.orientation
