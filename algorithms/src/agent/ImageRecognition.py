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

    def get_next_step(self, arena: Arena, robot_info: RobotInfo) -> Coord:
        self.arena = arena
        self.cur_coord = robot_info.get_coord()
        self.cur_direction = robot_info.get_orientation()
        next_step = self.right_wall_hug()

        sis = SensorInputSimulation(robot_info, self.arena)
        self.obstacle_coord_list, no_obs_coord_list = sis.calculate_percepts()

        if self.arena.get_cell_at_coord(next_step).is_visited(): #returned to start
            raise Exception('Right Wall Hugging: returned to initial position')
            # target = self.get_nearest_obstacle()
            # next_step = FastestPathAlgo().get_next_step(arena, robot_info, end=target, waypoint=None)
            return None

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

    # def get_nearest_obstacle(self):
    #     self.q = []

    #     for coord in self.obstacle_coord_list:
    #         if self.arena.get_cell_at_coord(coord).is_visited():
    #             q.append(coord)

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
                adj_obstacle = viewed_cell
                surface_orientation = (abs_degree + 180)%360
                self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
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
                adj_obstacle = viewed_cell
                surface_orientation = (abs_degree + 180)%360
                self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
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