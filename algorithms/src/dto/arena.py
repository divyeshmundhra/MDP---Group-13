from src.dto.constants import *
from src.dto.cell import Cell
from src.dto.coord import Coord

class Arena:
    def __init__(self):
        self.cell_matrix = [[None for y in range(MAP_ROW)] for x in range(MAP_COL)] #initialise the matrix

        for y in range(MAP_ROW): #create Cell objects for each cell in the matrix
            for x in range(MAP_COL): 
                coord = Coord(x, y)
                self.cell_matrix[coord.get_x()][coord.get_x()] = Cell(coord)
    
    # def set_cell_at_coord(self, cell, coord):
    #     self.cell_matrix[coord.get_x()][coord.get_y()] = cell

    def get_cell_at_coord(self, coord):
        return self.cell_matrix[coord.get_x()][coord.get_y()]