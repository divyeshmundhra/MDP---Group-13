from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform as OT
from src.dto.constants import *
from src.agent.FastestPathAlgo import FastestPathAlgo
from src.simulator.SensorInputSimulation import SensorInputSimulation

class LeftWallHuggingAlgo():
    def __init__(self):
        self.arena = None
        self.cur_coord = None
        self.cur_direction = None
        self.move_towards_island = False
        self.lwh_last_four = []
        self.lwh_looped = False

    def get_next_step(self, arena: Arena, robot_info: RobotInfo) -> Coord:
        self.arena = arena
        self.cur_coord = robot_info.get_coord()
        self.cur_direction = robot_info.get_orientation()

        # check for looping in LWH
        
        if len(self.lwh_last_four) == 4 and self.lwh_last_four[0].is_equal(self.cur_coord) and not self.lwh_last_four[1].is_equal(self.lwh_last_four[3]):
            self.lwh_looped = True

        if self.lwh_looped:
            next_step = self.move_back()
            self.lwh_looped = False
            self.lwh_last_four = []
        else:
            next_step = self.left_wall_hug()

        if len(self.lwh_last_four) >= 4:
            self.lwh_last_four.pop(0)

        self.lwh_last_four.append(self.cur_coord)   

        return next_step

    def left_wall_hug(self) -> Coord:
        if self.check_left_free():
            target = self.move_left()
        elif self.check_front_free():
            target = self.move_forward()
        elif self.check_right_free():
            target = self.move_right()
        else:
            target = self.move_back()
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
                return False

    def move_forward(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.NORTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        front_coord = self.cur_coord.add(displacement)
        return front_coord

    def move_left(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.WEST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        left_coord = self.cur_coord.add(displacement)
        return left_coord

    def move_right(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.EAST]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        right_coord = self.cur_coord.add(displacement)
        return right_coord

    def move_back(self) -> Coord:
        abs_degree = (self.cur_direction.value*90 + OT.orientation_to_degree[Orientation.SOUTH]) % 360
        displacement = OT.orientation_to_unit_displacement(OT.degree_to_orientation[abs_degree])
        back_coord = self.cur_coord.add(displacement)
        return back_coord 