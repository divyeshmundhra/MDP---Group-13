from src.dto.coord import Coord
from src.dto.constants import Orientation

class DecisionNode:
    def __init__(self, coord: Coord, orientation: Orientation, exact_cost: int, parent):
        self.coord = coord
        self.orientation = orientation
        self.exact_cost = exact_cost
        self.parent = parent

    def get_coord(self):
        return self.coord

    def get_orientation(self):
        return self.orientation

    def get_exact_cost(self):
        return self.exact_cost

    def get_parent(self):
        return self.parent