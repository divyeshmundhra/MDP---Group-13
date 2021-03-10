from queue import PriorityQueue
# from src.dto import Arena, Coord, RobotInfo, OrientationTransform
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform
from src.dto.arena import Arena

from src.dto.constants import TimeCosts, MAP_ROW, MAP_COL
from src.agent import DecisionNode

class FastestPathAlgo():
    def get_next_step(self, arena: Arena, robot_info: RobotInfo, end: Coord, waypoint: Coord, full_path = False):
        start = robot_info.get_coord()
        current_orientation = robot_info.get_orientation()
        if not arena.coord_is_valid(end):
            raise Exception(f'FastestPath: end coord out of arena: {end.get_x()}, {end.get_y()}')
        end_cell = arena.get_cell_at_coord(end)
        if end_cell.is_obstacle():
            raise Exception(f'FastestPath: cannot move to obstacle cell: {end.get_x()}, {end.get_y()}')

        if waypoint:
            # we haven't passed through it yet
            target = waypoint
        else:
            target = end

        root = DecisionNode.DecisionNode(start, exact_cost=0, parent=None)
        fringe_nodes = PriorityQueue()
        queue_tie_breaker = 0
        fringe_nodes.put((
            FastestPathAlgo.heuristic(root.get_coord(), target),
            queue_tie_breaker,
            root
        ))
        examined = [[False for y in range(MAP_ROW)] for x in range(MAP_COL)]

        cur = fringe_nodes.get()[-1]
        while True:
            # two conditions to exit loop, one may or may not be enabled
            cur_coord = cur.get_coord()
            if not end_cell.is_dangerous():
                # normally exit when target is found
                if cur_coord.is_equal(target):
                    # found target so break and find first step
                    break
            else:
                # if cell is dangerous, move to any adjacent cell to it \
                # (some cells, surrounded by danger, will be left out, but they are likely to be obstacles)
                if cur_coord.subtract(target).manhattan_distance() <= 1 \
                    and cur_coord.subtract(start).manhattan_distance() > 0:
                    # target not found so return None
                    return None
                if fringe_nodes.qsize() >= 3000:
                    # if we've got such a long queue, there's probably no way to the target
                    return None
            examined[cur_coord.get_x()][cur_coord.get_y()] = True
            adjacent_coords = arena.get_adjacent_safe(cur_coord)
            for coord in adjacent_coords:
                if examined[coord.get_x()][coord.get_y()]:
                    # assume heuristic is admissible, otherwise need to assess total_cost \
                    # and remove node from examined set if g+h < existing g
                    continue
                # get costs
                target_orientation = OrientationTransform.displacement_to_orientation(coord.subtract(cur_coord))
                degree_of_turn = OrientationTransform.calc_degree_of_turn(current_orientation, target_orientation)
                turning_cost = int(degree_of_turn/90)*TimeCosts.QUARTER_TURN
                displacement_cost = TimeCosts.MOVE_ONE_UNIT
                cost = turning_cost + displacement_cost
                exact_cost = cur.get_exact_cost() + cost
                estimated_cost = FastestPathAlgo.heuristic(cur_coord, target)
                total_cost = exact_cost + estimated_cost
                fringe_node = DecisionNode.DecisionNode(coord, exact_cost=exact_cost, parent=cur)
                queue_tie_breaker += 1
                fringe_nodes.put((total_cost, queue_tie_breaker, fringe_node))

            if fringe_nodes.empty():
                return None
            cur = fringe_nodes.get()[-1]

        cur_list = []
        while cur.get_parent() and not cur.get_parent().get_coord().is_equal(start):
            cur_list.append(cur.get_coord())
            cur = cur.get_parent()
        cur_list.append(cur.get_coord())

        if full_path:
            cur_list.reverse()
            return cur_list
        else:
            return cur.get_coord()

    @staticmethod
    def heuristic(cur: Coord, target: Coord) -> int:
        return target.subtract(cur).manhattan_distance()
