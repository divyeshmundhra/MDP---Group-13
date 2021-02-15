from src.dto.coord import Coord

class DecisionNode:
    def __init__(self, coord: Coord, exact_cost: int, parent):
        self.coord = coord
        self.exact_cost = exact_cost
        self.parent = parent
    
    def get_coord(self):
        return self.coord
    
    def get_exact_cost(self):
        return self.exact_cost
    
    def get_parent(self):
        return self.parent
