from queue import PriorityQueue
from src.dto import Arena, Coord, RobotInfo, OrientationTransform
from src.dto.constants import TimeCosts
from src.agent import DecisionNode

class FastestPathAlgo():
    def __init__(self):
        pass
    
    @staticmethod
    def get_next_step(arena: Arena, robot_info: RobotInfo, end: Coord, waypoint: Coord) -> Coord:
        start = robot_info.get_coord()
        current_orientation = robot_info.get_orientation()
        if waypoint:
            # we haven't passed through it yet
            target = waypoint
        else:
            target = end

        root = DecisionNode.DecisionNode(start, exact_cost=0, parent=None)
        fringe_nodes = PriorityQueue()
        fringe_nodes.put((
            FastestPathAlgo.heuristic(root.get_coord(), target),
            root
        ))
        examined_nodes = set()

        cur = fringe_nodes.get()[1]
        while not cur.get_coord.is_equal(target):
            cur_coord = cur.get_coord()
            examined_nodes.add(cur_coord)
            adjacent_coords = arena.get_adjacent_unblocked(cur_coord)
            for coord in adjacent_coords:
                if coord in examined_nodes:
                    # assume heuristic is admissible, otherwise need to assess total_cost \
                    # and remove node from examined set if g+h < existing g
                    continue
                # get costs
                target_orientation = OrientationTransform.OrientationTransform.displacement_to_orientation(coord.subtract(cur_coord))
                turning_cost = OrientationTransform.OrientationTransform.calc_degree_of_turn(current_orientation, target_orientation)
                displacement_cost = TimeCosts.MOVE_ONE_UNIT
                cost = turning_cost + displacement_cost
                exact_cost = cur.get_exact_cost() + cost
                estimated_cost = FastestPathAlgo.heuristic(cur_coord, target)
                total_cost = exact_cost + estimated_cost

                fringe_node = DecisionNode.DecisionNode(coord, exact_cost=exact_cost, parent=cur)
                fringe_nodes.put((total_cost, fringe_node))
            cur = fringe_nodes.get()[1]

        while not cur.get_parent().get_coord().is_equal(start):
            cur = cur.get_parent()
    
        return cur.get_coord()

    @staticmethod
    def heuristic(cur: Coord, target: Coord) -> int:
        return target.subtract(cur).manhattan_distance()
