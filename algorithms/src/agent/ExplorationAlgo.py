from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.RobotInfo import RobotInfo
# from src.dto.constants import VIEW_RANGE, MAP_ROW, MAP_COL
from src.agent.FastestPathAlgo import FastestPathAlgo
import itertools

class ExplorationAlgo():
    def __init__(self):
        self.arena = None
        self.cur_coord = None
        self.unexplored_stack = []

    def get_next_step(self, arena: Arena, robot_info: RobotInfo, end=None, waypoint=None) -> Coord:
        # implement the flood fill algorithm using a stack
        self.arena = arena
        self.cur_coord = robot_info.get_coord()

        next_target = self.get_nearest_unexplored(self.cur_coord)
        if not next_target:
            # all have been explored
            return None

        fp_algo = FastestPathAlgo()
        next_step = fp_algo.get_next_step(arena, robot_info, end=next_target, waypoint=None)
        if not next_step:
            # unexplored cells are inaccessible, probably an obstacle is falsely set
            raise Exception('ExplorationAlgo: unexplored cell(s) are inacessible') 

        return next_step

    # def add_cardinal_unvisited_cells_to_stack(self) -> None:
    #     adj = []
    #     for displacement in Arena.ADJACENCY:
    #         for i in range(VIEW_RANGE, 0, -1): # prioritize large steps when possible
    #             adj_coord = self.cur_coord.add(displacement.multiply(i))
    #             if not 0 <= adj_coord.get_x() < MAP_COL or not 0 <= adj_coord.get_y() < MAP_ROW:
    #                 break # out of range
    #             adj_cell = self.arena.get_cell_at_coord(adj_coord)
    #             if adj_cell.is_explored() and not adj_cell.is_obstacle() and not adj_cell.is_dangerous():
    #                 adj.append(adj_coord)

    def get_nearest_unexplored(self, root: Coord) -> Coord:
        q = []
        seen_nodes = []
        q.append(root)
        while not len(q) == 0:
            c = q.pop(0) # get first from queue
            seen_nodes.append(c) # mark coord as seen by algo
            cell = self.arena.get_cell_at_coord(c)
            if not cell.is_explored() and not cell.is_dangerous():
                # we know if cell is dangerous if next to obstacle, even if not explored
                return c
            adj_cells = self.arena.get_four_adjacent_in_arena(c)
            for adj in adj_cells:
                seen = False
                for qi in itertools.chain(q, seen_nodes):
                    if adj.is_equal(qi):
                        seen = True
                        break
                if not seen:
                    q.append(adj)

    # def add_cardinal_unexplored_cells_to_stack(self) -> None:
    #     # add cell just outside visible range to stack to be explored
    #     # if that cell happens to be explored, don't go
    #     adj = []
    #     for displacement in Arena.ADJACENCY:
    #         blocked = self.check_for_blockers_in_direction(displacement)
    #         if blocked:
    #             continue
    #         candidate_coord = self.cur_coord.add(displacement.multiply(VIEW_RANGE+1))
    #         if not candidate_coord.get_x() in range(MAP_COL) or not candidate_coord.get_y() in range(MAP_ROW):
    #             # skip if edge cell is outside map
    #             continue
    #         candidate_cell = self.arena.get_cell_at_coord(candidate_coord)
    #         if not candidate_cell.is_explored():
    #             adj.append(candidate_coord)
    #     self.unexplored_stack.extend(adj)

    # def check_for_blockers_in_direction(self, displacement: Coord) -> bool:
    #     for i in range(1, VIEW_RANGE+1): # add 1 to include cell at view range limit
    #         adj_coord = self.cur_coord.add(displacement.multiply(i))
    #         if not 0 <= adj_coord.get_x() < MAP_COL or not 0 <= adj_coord.get_y() < MAP_ROW:
    #             break # out of range
    #         adj_cell = self.arena.get_cell_at_coord(adj_coord)
    #         if adj_cell.is_obstacle() or adj_cell.is_dangerous():
    #             return True
    #     return False
