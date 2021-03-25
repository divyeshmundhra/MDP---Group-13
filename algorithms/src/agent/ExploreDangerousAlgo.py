from src.dto.coord import Coord
from src.dto.constants import SURE_VIEW_RANGE
from src.agent.FastestPathAlgo import FastestPathAlgo
from src.agent.DecisionNode import DecisionNode

class ExploreDangerousAlgo():
    # this gets called when robot explored all non-dangerous unexplored cells \
    # but dangerous cells still remain
    def __init__(self, arena, robot_info):
        self.arena = arena
        self.robot_info = robot_info
        self.unexplored_coords = arena.list_unexplored_coords()

    def calculate_cheapest_path(self):
        # path = self.bfs() # solution is super slow
        path = self.greedy() # slightly suboptimal, but much faster
        return path

    def greedy(self):
        unvisited = self.unexplored_coords.copy()
        coord = self.robot_info.get_coord()
        path = []
        
        while unvisited:
            # sort unexplored cells by distance
            ue_distance = []
            for ue in unvisited:
                ue_distance.append((ue, ue.subtract(self.robot_info.get_coord()).manhattan_distance()))
            coord = sorted(ue_distance, key=lambda x: x[1])[0][0]

            for distance_vantages in self.calculate_vantage_points(coord):
                found_vantage = False
                for vantage in distance_vantages:
                    if not self.arena.coord_is_valid(vantage):
                        continue
                    if self.arena.get_cell_at_coord(vantage).is_dangerous():
                        continue
                    if self.robot_info.get_coord().is_equal(vantage):
                        continue
                    found_vantage = True
                    break
                if found_vantage:
                    break
            if vantage: # pylint: disable=undefined-loop-variable
                path.append(vantage) # pylint: disable=undefined-loop-variable
            else:
                raise Exception(f'ExploreDangerousAlgo: unexplored cell {coord.get_x(), coord.get_y()}cannot be viewed from any angle')
            unvisited.remove(coord)

        return path

    def bfs(self):
        # build bfs tree
        root = DecisionNode(self.robot_info.get_coord(), self.robot_info.get_orientation(), exact_cost=0, parent=None)
        prev_layer_nodes = [root]
        for ue in self.unexplored_coords:
            layer_nodes = []
            for distance_vantages in self.calculate_vantage_points(ue):
                valid_vantage_at_distance = False
                for vantage in distance_vantages:
                    if not self.arena.coord_is_valid(vantage):
                        continue
                    if self.arena.get_cell_at_coord(vantage).is_dangerous():
                        continue
                    for prev_node in prev_layer_nodes:
                        fp = FastestPathAlgo().get_fastest_path(self.arena, self.robot_info, end=vantage)
                        cost = fp[-1].get_exact_cost() + prev_node.get_exact_cost() # we ignore final turning cost actually
                        cur_node = DecisionNode(ue, fp[-1].get_orientation(), exact_cost=cost, parent=prev_node)
                        layer_nodes.append(cur_node)
                    valid_vantage_at_distance = True
                    # break # this break avoids checking multiple valid vantage points at SAME distance
                if valid_vantage_at_distance:
                    # the point of this is to not check FURTHER distances \
                    # if some vantage point is found near the ue at a shorter distance
                    break
            prev_layer_nodes = layer_nodes
            node = min(layer_nodes, key=lambda x: x.get_exact_cost())

        # get optimal path
        path = []
        while not node is None and not node.get_parent() is root:
            path.append(node.get_coord())
            node = node.get_parent()
        path.reverse()

        return path

    def calculate_vantage_points(self, ue: Coord) -> list:
        # vantage points are cells where robot can see unexplored cells
        vantage_points_matrix = []
        for i in range(SURE_VIEW_RANGE):
            vantage_points = []
            for disp in [(0,i+2), (i+2,0), (0,-i-2), (-i-2,0), (-1,i+2), (1,i+2), (i+2,1), (i+2,-1), (1,-i-2), (-1,-i-2), (-i-2,-1), (-i-2,1)]:
                coord = Coord(disp[0], disp[1])
                vantage_points.append(ue.add(coord))
            vantage_points_matrix.append(vantage_points)
        return vantage_points_matrix
