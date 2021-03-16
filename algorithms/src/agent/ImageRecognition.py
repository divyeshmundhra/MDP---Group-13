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
        self.cur_coord = robot_info.get_coord()
        self.cur_direction = robot_info.get_orientation()

        next_step = self.right_wall_hug() # keep trying right wall hugging
        
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

    def align_right_wall(self, arena: Arena, robot_info: RobotInfo, target_obstacle: Coord) -> Coord: 
        self.arena = arena
        self.cur_coord = robot_info.get_coord()
        self.cur_direction = robot_info.get_orientation()

        # check which side the obstacle is on with respect to the robot's current orientation, 
        # then rotate the robot to have its right side facing the obstacle
        if self.check_left_free(True).is_equal(target_obstacle):
            target = self.move_back()
        elif self.check_front_free(True).is_equal(target_obstacle):
            target = self.move_left()
        elif self.check_right_free(True).is_equal(target_obstacle):
            target = self.move_forward()
        return target

    def check_front_free(self, return_coords: bool = False) -> bool:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.NORTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        front_coord = self.cur_coord.add(displacement)
        if return_coords:
            displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree]).multiply(2)
            viewed_cell = self.cur_coord.add(displacement)
            return viewed_cell
        else:
            if self.arena.coord_is_valid(front_coord) and not self.arena.get_cell_at_coord(front_coord).is_dangerous():
                return True
            else:
                return False

    def check_left_free(self, return_coords: bool = False):
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.WEST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        left_coord = self.cur_coord.add(displacement)
        if return_coords:
            displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree]).multiply(2)
            viewed_cell = self.cur_coord.add(displacement)
            return viewed_cell
        else:
            if self.arena.coord_is_valid(left_coord) and not self.arena.get_cell_at_coord(left_coord).is_dangerous():
                return True
            else:
                return False

    def check_right_free(self, return_coords: bool = False):
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.EAST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        right_coord = self.cur_coord.add(displacement)
        if return_coords:
            displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree]).multiply(2)
            viewed_cell = self.cur_coord.add(displacement)
            return viewed_cell
        else:
            if self.arena.coord_is_valid(right_coord) and not self.arena.get_cell_at_coord(right_coord).is_dangerous():
                return True
            else:
                displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree]).multiply(2)
                viewed_cell = self.cur_coord.add(displacement)
                if self.arena.coord_is_valid(viewed_cell) and self.arena.get_cell_at_coord(viewed_cell).is_obstacle():
                    adj_obstacle = viewed_cell
                    surface_orientation = (abs_degree + 180)%360
                    self.arena.get_cell_at_coord(adj_obstacle).set_seen_surface(surface_orientation)
                    self.arena.get_cell_at_coord(adj_obstacle).set_is_seen(True)
                return False

    def move_forward(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.NORTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        front_coord = self.cur_coord.add(displacement)

        obstacle_cell_abs_degree = (abs_degree + 90)%360
        obstacle_cell = self.cur_coord.add(OT.orientation_to_unit_displacement(OT.degree_to_orientation[obstacle_cell_abs_degree]).multiply(2))
        if self.arena.coord_is_valid(obstacle_cell) and self.arena.get_cell_at_coord(obstacle_cell).is_obstacle():
            surface_orientation = (obstacle_cell_abs_degree + 180)%360
            self.arena.get_cell_at_coord(obstacle_cell).set_seen_surface(surface_orientation)
            self.arena.get_cell_at_coord(obstacle_cell).set_is_seen(True)

        return front_coord

    def move_left(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.WEST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        left_coord = self.cur_coord.add(displacement)

        obstacle_cell_abs_degree = (abs_degree + 90)%360
        obstacle_cell = self.cur_coord.add(OT.orientation_to_unit_displacement(OT.degree_to_orientation[obstacle_cell_abs_degree]).multiply(2))
        if self.arena.coord_is_valid(obstacle_cell) and self.arena.get_cell_at_coord(obstacle_cell).is_obstacle():
            surface_orientation = (obstacle_cell_abs_degree + 180)%360
            self.arena.get_cell_at_coord(obstacle_cell).set_seen_surface(surface_orientation)
            self.arena.get_cell_at_coord(obstacle_cell).set_is_seen(True)

        return left_coord

    def move_right(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.EAST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        right_coord = self.cur_coord.add(displacement)

        obstacle_cell_abs_degree = (abs_degree + 90)%360
        obstacle_cell = self.cur_coord.add(OT.orientation_to_unit_displacement(OT.degree_to_orientation[obstacle_cell_abs_degree]).multiply(2))
        if self.arena.coord_is_valid(obstacle_cell) and self.arena.get_cell_at_coord(obstacle_cell).is_obstacle():
            surface_orientation = (obstacle_cell_abs_degree + 180)%360
            self.arena.get_cell_at_coord(obstacle_cell).set_seen_surface(surface_orientation)
            self.arena.get_cell_at_coord(obstacle_cell).set_is_seen(True)

        return right_coord

    def move_back(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.SOUTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        back_coord = self.cur_coord.add(displacement)

        obstacle_cell_abs_degree = (abs_degree + 90)%360
        obstacle_cell = self.cur_coord.add(OT.orientation_to_unit_displacement(OT.degree_to_orientation[obstacle_cell_abs_degree]).multiply(2))
        if self.arena.coord_is_valid(obstacle_cell) and self.arena.get_cell_at_coord(obstacle_cell).is_obstacle():
            surface_orientation = (obstacle_cell_abs_degree + 180)%360
            self.arena.get_cell_at_coord(obstacle_cell).set_seen_surface(surface_orientation)
            self.arena.get_cell_at_coord(obstacle_cell).set_is_seen(True)

        return back_coord 