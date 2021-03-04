from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform as OT
from src.dto.constants import *


# Implement right wall hugging algorithm

class RightWallHuggingAlgo():
    def __init__(self):
        self.arena = None
        self.cur_coord = None
        self.cur_direction = None

    def get_next_step(self, robot_info: RobotInfo, arena: Arena) -> Coord:
        self.arena = arena
        self.cur_coord = robot_info.get_coord()
        self.cur_direction = robot_info.get_orientation()
        next_step = self.right_wall_hug()

        if next_step == Coord(1,1): #returned to start
            # if self.arena.get_cell_at_coord(next_step).is_visited(): # returned to starting position
            raise Exception('Right Wall Hugging: returned to initial position')
            return None

        return next_step

    def right_wall_hug(self) -> Coord:
        #     print("after setting seen surface: ", self.arena.get_cell_at_coord(self.obstacle_on_right).get_seen_surfaces())
        #     print("obstacle_on_right: ", self.obstacle_on_right.get_x(), self.obstacle_on_right.get_y())

        if self.check_right_free():
            target = self.move_right()
        elif self.check_front_free():
            target = self.move_forward()
        elif self.check_left_free():
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
            if 0 <= viewed_cell.get_x() < MAP_COL and 0 <= viewed_cell.get_y() < MAP_ROW and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
                self.obstacle = viewed_cell
                self.surface_orientation = (abs_degree + 180)%360
                print("viewed surface: ", self.surface_orientation)
                self.arena.get_cell_at_coord(self.obstacle).set_seen_surface(self.surface_orientation)
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
            if 0 <= viewed_cell.get_x() < MAP_COL and 0 <= viewed_cell.get_y() < MAP_ROW and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
                self.obstacle = viewed_cell
                self.surface_orientation = (abs_degree + 180)%360
                print("viewed surface: ", self.surface_orientation)
                self.arena.get_cell_at_coord(self.obstacle).set_seen_surface(self.surface_orientation)
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
                self.obstacle = viewed_cell
                self.surface_orientation = (abs_degree + 180)%360
                print("viewed surface: ", self.surface_orientation)
                self.arena.get_cell_at_coord(self.obstacle).set_seen_surface(self.surface_orientation)
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