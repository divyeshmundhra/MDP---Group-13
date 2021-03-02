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

    EIGHT_ADJACENCY = [
        Coord(1, 1),
        Coord(0, 1),
        Coord(-1, 1),
        Coord(-1, 0),
        Coord(-1, -1),
        Coord(0, -1),
        Coord(1, -1),
        Coord(1, 0)
    ]

    def __init__(self):
        self.cell_matrix = [[None for y in range(MAP_ROW)] for x in range(MAP_COL)] #initialise the matrix

        for y in range(MAP_ROW): #create Cell objects for each cell in the matrix
            for x in range(MAP_COL):
                coord = Coord(x, y)
                self.cell_matrix[coord.get_x()][coord.get_y()] = Cell(coord)

        self.mark_border_cells_dangerous()

    def coord_is_valid(self, coord: Coord) -> bool:
        return coord.get_x() in range(MAP_COL) and coord.get_y() in range(MAP_ROW)

    def get_cell_at_coord(self, coord):
        return self.cell_matrix[coord.get_x()][coord.get_y()]

    def get_four_adjacent_in_arena(self, coord: Coord) -> list:
        adj = []
        for displacement in Arena.ADJACENCY:
            adj_coord = coord.add(displacement)
            if self.coord_is_valid(adj_coord):
                adj.append(adj_coord)
        return adj

    def get_eight_adjacent_in_arena(self, coord: Coord) -> list:
        adj = []
        for displacement in Arena.EIGHT_ADJACENCY:
            adj_coord = coord.add(displacement)
            if self.coord_is_valid(adj_coord):
                adj.append(adj_coord)
        return adj

    def calculate_adjacent_lists(self, coord) -> dict:
        adj_safe, adj_unb, adj_blocked = [], [], []
        for adj_coord in self.get_four_adjacent_in_arena(coord):
            cell = self.get_cell_at_coord(adj_coord)
            if cell.is_obstacle():
                adj_blocked.append(adj_coord)
            else:
                adj_unb.append(adj_coord)
                if not cell.is_dangerous():
                    adj_safe.append(adj_coord)
        return {
            'safe': adj_safe,
            'unblocked': adj_unb,
            'blocked': adj_blocked
        }
    
    # def get_adjacent_blocked(self, coord) -> list:
    #     return self.calculate_adjacent_lists(coord)['blocked']

    def get_adjacent_safe(self, coord) -> list:
        return self.calculate_adjacent_lists(coord)['safe']

    # def get_adjacent_unblocked(self, coord) -> list:
    #     return self.calculate_adjacent_lists(coord)['unblocked']
    
    def mark_dangerous_cells_around_obstacle(self, coord):
        for displacement in Arena.EIGHT_ADJACENCY:
            adj_coord = coord.add(displacement)
            if self.coord_is_valid(adj_coord):
                self.get_cell_at_coord(adj_coord).set_is_dangerous(True)        

    def get_coverage_percentage(self) -> int:
        explored = 0
        for y in range(MAP_ROW): 
            for x in range(MAP_COL):
                coord = Coord(x, y)
                if self.get_cell_at_coord(coord).is_explored():
                    explored += 1

        return explored / 300 * 100
    
    def list_unexplored_cells(self) -> list:
        # returns CELLS not coords
        l = []
        for row in self.cell_matrix:
            for cell in row:
                if not cell.is_explored():
                    l.append(cell)
        return l

    def set_all_explored(self) -> None:
        for row in self.cell_matrix:
            for cell in row:
                cell.set_is_explored(True)

    def mark_border_cells_dangerous(self) -> None:
        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                if y in [0,MAP_ROW-1] or x in [0,MAP_COL-1]:
                    # cells at edge of arena are too close to the walls
                    self.get_cell_at_coord(Coord(x,y)).set_is_dangerous(True)
