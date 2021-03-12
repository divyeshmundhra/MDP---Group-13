from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform as OT
from src.dto.constants import *
from src.agent.FastestPathAlgo import FastestPathAlgo
from src.simulator.SensorInputSimulation import SensorInputSimulation

class RightWallHuggingAlgo():
    def __init__(self):
        self.arena = None
        self.cur_coord = None
        self.cur_direction = None
        self.move_towards_island = False

    def get_next_step(self, arena: Arena, robot_info: RobotInfo) -> Coord:
        self.arena = arena
        self.robot_info = robot_info
        self.cur_coord = self.robot_info.get_coord()
        self.cur_direction = self.robot_info.get_orientation()

        next_step = self.right_wall_hug() # keep trying right wall hugging

        # if self.arena.get_cell_at_coord(next_step).is_visited(): # robot has passed through this coord before
        #     # self.move_towards_island = True
        #     print("ENCOUNTERED VISITED COORD AT: ", next_step.get_x(), next_step.get_y())
        # # if self.move_towards_island:
        #     self.waypoint_coord = self.get_nearest_obstacle() # move towards nearest obstacle instead
        #     # self.waypoint_coord = self.dangerous_exploration_path.pop(0)
        #     next_step = FastestPathAlgo().get_next_step(self.arena,self.robot_info, self.waypoint_coord)
            
            # if next_step.is_equal(self.cur_coord):
            #     self.move_towards_island == False
            #     return None
        
        return next_step

    def right_wall_hug(self) -> Coord:
        if self.check_right_free():
            target = self.move_right()
        elif self.check_front_free():
            target = self.move_forward()
        elif self.check_left_free():
            target = self.move_left()
        else:
            target = self.move_back()
        return target

    def align_right_wall(self, arena: Arena, robot_info: RobotInfo) -> Coord:
        self.arena = arena
        self.robot_info = robot_info
        self.cur_coord = self.robot_info.get_coord()
        self.cur_direction = self.robot_info.get_orientation()

        if self.check_front_free():
            if self.check_right_free():
                target = self.move_right()
            elif self.check_front_free():
                target = self.move_forward()
            elif self.check_left_free():
                target = self.move_left()
            else:
                target = self.move_back()
        else:
            if self.check_left_free():
                target = self.move_left()
            else:
                target = self.move_back()
        return target

    def check_front_free(self) -> bool:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.NORTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        front_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        if 0 <= front_coord.get_x() < MAP_COL and 0 <= front_coord.get_y() < MAP_ROW and not self.arena.get_cell_at_coord(front_coord).is_dangerous():
            return True
        else:
            displacement = OT.get_two_cells_away(OT.degree_to_orientation[abs_degree])
            viewed_cell = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
            # if 0 <= viewed_cell.get_x() < MAP_COL and 0 <= viewed_cell.get_y() < MAP_ROW and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
            #     adj_obstacle = viewed_cell
            #     surface_orientation = (abs_degree + 180)%360
            #     self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
            #     self.arena.get_cell_at_coord(adj_obstacle).set_is_seen(True)
            return False

    def check_left_free(self) -> bool:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.WEST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        left_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        if 0 <= left_coord.get_x() < MAP_COL and 0 <= left_coord.get_y() < MAP_ROW and not self.arena.get_cell_at_coord(left_coord).is_dangerous():
            return True
        else:
            displacement = OT.get_two_cells_away(OT.degree_to_orientation[abs_degree])
            viewed_cell = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
            # if 0 <= viewed_cell.get_x() < MAP_COL and 0 <= viewed_cell.get_y() < MAP_ROW and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
            #     adj_obstacle = viewed_cell
            #     surface_orientation = (abs_degree + 180)%360
            #     self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
            #     self.arena.get_cell_at_coord(adj_obstacle).set_is_seen(True)
            return False

    def check_right_free(self):
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.EAST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        right_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        if 0 <= right_coord.get_x() < MAP_COL and 0 <= right_coord.get_y() < MAP_ROW and not self.arena.get_cell_at_coord(right_coord).is_dangerous():
            return True
        else:
            displacement = OT.get_two_cells_away(OT.degree_to_orientation[abs_degree])
            viewed_cell = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
            if 0 <= viewed_cell.get_x() < MAP_COL and 0 <= viewed_cell.get_y() < MAP_ROW and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
                adj_obstacle = viewed_cell
                surface_orientation = (abs_degree + 180)%360
                self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
                self.arena.get_cell_at_coord(adj_obstacle).set_is_seen(True)
            return False

    def move_forward(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.NORTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        front_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        return front_coord

    def move_left(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.WEST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        left_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        return left_coord

    def move_right(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.EAST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        right_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        return right_coord

    def move_back(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.SOUTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        back_coord = OT.get_new_coords_after_displacement(self.cur_coord, displacement)
        return back_coord 


    # def get_nearest_obstacle(self):
    #     obstacle_coord_list = self.arena.list_known_obstacles()
    #     unseen_obstacles_list = []
    #     path = []
    #     target = None

    #     for item in obstacle_coord_list:
    #         if not self.arena.get_cell_at_coord(item).is_seen():
    #             if item.get_x() != 0 and item.get_x() != 14 and item.get_y() != 0 and item.get_y() != 19:
    #                 unseen_obstacles_list.append(item)

    #     # find nearest obstacle first, then find vantage
    #     while unseen_obstacles_list: # iterate through the obstacles
    #         ue_distance = [] 
    #         for ue in unseen_obstacles_list:
    #             ue_distance.append((ue, ue.subtract(self.robot_info.get_coord()).manhattan_distance()))
    #         coord = sorted(ue_distance, key=lambda x: x[1])[0][0] #sort coords by distance

    #         for vantage in self.calculate_vantage_points(coord): # iterate through the obstacle's vantage points
    #             found_vantage = False
                
    #             if not self.arena.coord_is_valid(vantage):
    #                 continue
    #             if self.arena.get_cell_at_coord(vantage).is_dangerous():
    #                 continue
    #             if not self.arena.get_cell_at_coord(vantage).is_explored():
    #                 continue

    #             found_vantage = True
                    
    #             if found_vantage:
    #                 break

    #         if vantage: # pylint: disable=undefined-loop-variable
    #             target = vantage
    #             break
    #             # path.append(vantage) # pylint: disable=undefined-loop-variable
    #         else:
    #             raise Exception(f'ImageRecognition: unexplored cell {coord.get_x(), coord.get_y()}cannot be viewed from any angle')
            
    #         unseen_obstacles_list.remove(coord)

    #     return target

    # def calculate_vantage_points(self, ue: Coord) -> list:
    #     # vantage points are cells where robot will stand next to an obstacle
    #     vantage_points = []
        
    #     for disp in [(-2,0), (0,2), (0,-2), (2,0)]:
    #         coord = Coord(disp[0], disp[1])
    #         vantage_points.append(ue.add(coord))
            
    #     return vantage_points