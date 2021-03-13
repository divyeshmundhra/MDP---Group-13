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
    
    # def mark_dangerous_cells_around_obstacle(self, coord, is_dangerous):
    #     for displacement in Arena.EIGHT_ADJACENCY:
    #         adj_coord = coord.add(displacement)
    #         if self.coord_is_valid(adj_coord):
    #             self.get_cell_at_coord(adj_coord).set_is_dangerous(is_dangerous)

    def update_dangerous_cells(self) -> None:
        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                cur = Coord(x, y)
                self.get_cell_at_coord(cur).set_is_dangerous(False)
                for displacement in Arena.EIGHT_ADJACENCY:
                    adj = cur.add(displacement)
                    if not self.coord_is_valid(adj):
                        continue
                    if self.get_cell_at_coord(adj).is_obstacle():
                        self.get_cell_at_coord(cur).set_is_dangerous(True)
                        break
        self.mark_border_cells_dangerous()

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


# Used in Image Rec Algo:
    def all_obstacles_seen(self) -> bool:
        for coord in self.list_known_obstacles():
            if not self.get_cell_at_coord(coord).is_seen():
                return False
        return True
    
    def list_known_obstacles(self) -> list:
        l = []
        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                coord = Coord(x,y)
                if self.get_cell_at_coord(coord).is_obstacle():
                    l.append(coord)
        return l

    def get_nearest_obstacle_adj_coord(self, cur_coord: Coord, adj: bool) -> Coord: # if empty is true, return the adj coord, else return the obstacle itself
        obstacle_coord_list = self.list_known_obstacles()
        unseen_obstacles_list = []
        target = None

        for item in obstacle_coord_list:
            if not self.get_cell_at_coord(item).is_seen():
                if item.get_x() != 0 and item.get_x() != 14 and item.get_y() != 0 and item.get_y() != 19:
                    unseen_obstacles_list.append(item)

        # store a list of all vantages, then take the nearest one
        rwh_vantage_list = []
        for coord in unseen_obstacles_list:
            for rwh_vantage in self.calculate_rwh_vantage(coord): # iterate through the obstacle's vantage points
                if not self.coord_is_valid(rwh_vantage):
                    continue
                if self.get_cell_at_coord(rwh_vantage).is_obstacle():
                    continue
                if self.get_cell_at_coord(rwh_vantage).is_dangerous():
                    continue
                if not self.get_cell_at_coord(rwh_vantage).is_explored():
                    continue

                distance = rwh_vantage.subtract(cur_coord).manhattan_distance()
                rwh_vantage_list.append((distance, rwh_vantage, coord))

        target = sorted(rwh_vantage_list, key=lambda x: x[0])[0]

        if adj:
            return target[1]
        else:
            return target[2]


    def calculate_rwh_vantage(self, ue: Coord) -> list:
        # vantage points are cells where robot will stand next to an obstacle
        rwh_vantage_points = []
        
        for disp in [(-2,0), (0,2), (0,-2), (2,0)]:
            coord = Coord(disp[0], disp[1])
            rwh_vantage_points.append(ue.add(coord))
            
        return rwh_vantage_points