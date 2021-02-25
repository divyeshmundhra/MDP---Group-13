from src.dto.constants import *
from src.dto.cell import Cell
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo
from src.dto.arena import Arena
from src.dto.calculate_coords import CalculateCoords

class SensorInput:

    # we don't need the known_arena if we are getting the blocked_coords from the sensor
    def __init__(self, robot_info: RobotInfo, known_arena: Arena, agent_arena: Arena):
        self.robot_info = robot_info
        self.known_arena = known_arena
        self.agent_arena = agent_arena
        self.visible_cells = self.get_visible_cells()
        self.blocked_coords = self.get_blocked_coords()

    def get_visible_cells(self) -> list: # this function calculates the coords of ALL cells in the visible range (including those behind obstacles)
        orientation = self.robot_info.get_orientation()
        visible_cells = []
        cur_coord = self.robot_info.get_coord()

        # Created separate for loops so that the cells will be in order in the list
        
        # front of the robot
        for i in range(8): # visible range is 8 cells
            visible_cells.append(CalculateCoords(cur_coord).displacement_straight(i, orientation))
        for j in range(8):
            visible_cells.append(CalculateCoords(cur_coord).displacement_straight_left(j, orientation))
        for k in range(8):
            visible_cells.append(CalculateCoords(cur_coord).displacement_straight_right(k, orientation))

        # left of the robot
        for l in range(8):
            visible_cells.append(CalculateCoords(cur_coord).displacement_left_up(l, orientation))
        for m in range(8):
            visible_cells.append(CalculateCoords(cur_coord).displacement_left_down(m, orientation))

        # right of the robot
        for n in range(8):
            visible_cells.append(CalculateCoords(cur_coord).displacement_right_top(n, orientation))

        return visible_cells     # TODO: may contain None


    # TODO: write a function to retrieve the list of blocked_coords from the robot sensor
    # this function retrieves the list of blocked coords from the arena that can be seen at current coord
    def get_blocked_coords(self) -> list:
        blocked_coords = []
        count = 0
        blocked = False
        for coord in self.visible_cells:
            if count < 8: # if count is less than 8, it means you're still in the current row -> keep incrementing in every iteration of for loop
                count = count+1
                if coord == None:
                    continue
                if blocked == False and self.known_arena.get_cell_at_coord(coord).is_obstacle():
                    blocked_coords.append(coord)
                    blocked = True
            else: 
                count = 0
                blocked = False

                count = count+1
                if coord == None:
                    continue
                if blocked == False and self.known_arena.get_cell_at_coord(coord).is_obstacle():
                    blocked_coords.append(coord)
                    blocked = True

        return blocked_coords

    def get_cell_type_from_sensor(self, coord: Coord) -> list: # this function sets whether a cell is blocked, explored or not explored (behind obstacles)
        seen_at_coord = []
        seen_at_coord.append(coord)

        for displacement in Arena.ADJACENCY:
            adj_coord = coord.add(displacement.multiply(1))
            if not 0 <= adj_coord.get_x() < MAP_COL or not 0 <= adj_coord.get_y() < MAP_ROW:
                break # out of range
            seen_at_coord.append(adj_coord)

        # from the list of blocked_coords, update the agent's arena (is_obstacle)
        for blocked_coord in self.blocked_coords:
            self.agent_arena.get_cell_at_coord(blocked_coord).set_is_obstacle(True)
            self.agent_arena.get_cell_at_coord(blocked_coord).set_is_explored(True)
            self.agent_arena.mark_dangerous_cells_around_obstacle(blocked_coord)
            seen_at_coord.append(blocked_coord)

        count = 0
        blocked = False
        # determine which cells to set as explored or not explored (based on which cells are in front of / behind obstacles)
        for coord in self.visible_cells:
            if count < 8:
                count = count+1
                if coord == None:
                    continue
                if blocked == False:
                    if self.agent_arena.get_cell_at_coord(coord).is_obstacle():
                        blocked = True
                    self.agent_arena.get_cell_at_coord(coord).set_is_explored(True)
                    seen_at_coord.append(coord)
            else: 
                count = 0
                blocked = False

                count = count+1
                if coord == None:
                    continue
                if blocked == False:
                    if self.agent_arena.get_cell_at_coord(coord).is_obstacle():
                        blocked = True
                    self.agent_arena.get_cell_at_coord(coord).set_is_explored(True)
                    seen_at_coord.append(coord)

        return seen_at_coord

    def calculate_adjacent_lists(self, coord) -> dict:
        adj_safe, adj_unb, adj_blocked = [], [], []
        for adj_coord in self.get_cell_type_from_sensor(coord):
            cell = self.agent_arena.get_cell_at_coord(adj_coord)
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
        