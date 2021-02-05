from src.dto import Coord

class Arena:
    ADJACENCY = [
        Coord.Coord(0, 1),
        Coord.Coord(1, 0),
        Coord.Coord(0, -1),
        Coord.Coord(-1, 0)
    ]

    def __init__(self):
        self.cell_matrix = [[None for y in range(20)] for x in range(15)]
    
    def set_cell_at_coord(self, cell, coord):
        self.cell_matrix[coord.get_x()][coord.get_y()] = cell

    def get_cell_at_coord(self, coord):
        return self.cell_matrix[coord.get_x()][coord.get_y()]
    
    def get_adjacent_unblocked(self, coord):
        adj = []
        x = coord.get_x()
        y = coord.get_y()
        for coord in Arena.ADJACENCY:
            if not self.get_cell_at_coord(coord).is_obstacle:
                adj.append(coord)
        return adj
