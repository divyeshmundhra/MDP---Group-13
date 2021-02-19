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

    def get_seen_cells_at_coord(self, coord: Coord) -> list:
        seen_at_coord = []
        # you obviously have explored the cell you're standing on. This should be 3x3 around the robot in the actual run
        seen_at_coord.append(coord)
        for displacement in Arena.ADJACENCY:
            for i in range(1, VIEW_RANGE+1): # add 1 to include cell just outside view range
                adj_coord = coord.add(displacement.multiply(i))
                if not 0 <= adj_coord.get_x() < MAP_COL or not 0 <= adj_coord.get_y() < MAP_ROW:
                    break # out of range
                seen_at_coord.append(adj_coord)
                adj_cell = self.get_cell_at_coord(adj_coord)
                if adj_cell.is_obstacle():
                    break # we can't see behind obstacles
        return seen_at_coord

    def calculate_adjacent_lists(self, coord) -> dict:
        adj_safe, adj_unb, adj_blocked = [], [], []
        for adj_coord in self.get_seen_cells_at_coord(coord):
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
    
    def get_adjacent_blocked(self, coord) -> list:
        return self.calculate_adjacent_lists(coord)['blocked']

    def get_adjacent_safe(self, coord) -> list:
        return self.calculate_adjacent_lists(coord)['safe']

    def get_adjacent_unblocked(self, coord) -> list:
        return self.calculate_adjacent_lists(coord)['unblocked']
    
    def mark_dangerous_cells_around_obstacle(self, coord):
        for displacement in Arena.ADJACENCY:
            adj_coord = coord.add(displacement)
            self.get_cell_at_coord(adj_coord).set_is_dangerous(True)        
