from src.dto.constants import *
from src.dto.cell import Cell
from src.dto.coord import Coord

class Arena:

    ADJACENCY = [
        Coord(0, 1),
        Coord(1, 0),
        Coord(0, -1),
        Coord(-1, 0)
    ]

    def __init__(self):
        self.cell_matrix = [[None for y in range(MAP_ROW)] for x in range(MAP_COL)] #initialise the matrix

        for y in range(MAP_ROW): #create Cell objects for each cell in the matrix
            for x in range(MAP_COL): 
                coord = Coord(x, y)
                self.cell_matrix[coord.get_x()][coord.get_y()] = Cell(coord)
    
    # def set_cell_at_coord(self, cell, coord):
    #     self.cell_matrix[coord.get_x()][coord.get_y()] = cell

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
