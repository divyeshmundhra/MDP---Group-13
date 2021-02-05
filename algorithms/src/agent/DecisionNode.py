from src.dto import Coord

class DecisionNode:
    def __init__(self, coord: Coord, total_cost: int, parent: DecisionNode):
        self.coord = coord
        self.total_cost = total_cost
        self.parent = parent
    
    def get_coord(self):
        return self.coord
    
    def get_total_cost(self):
        return self.total_cost
    
    def get_parent(self):
        return self.parent
