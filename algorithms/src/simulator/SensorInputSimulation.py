from src.dto.constants import *
from src.dto.coord import Coord
from src.dto.RobotInfo import RobotInfo
from src.dto.arena import Arena
from src.dto.OrientationTransform import OrientationTransform as OT

class SensorInputSimulation:
    # Currently we're still returning lists of blocked and unblocked coords. \
    # This should be changed when sensor parser is built

    def __init__(self, robot_info: RobotInfo, arena: Arena):
        self.robot_info = robot_info
        self.arena = arena
        self.obstacles_coord_list = []
        self.no_obs_coord_list = []

    def calculate_percepts(self) -> list: # this function calculates the coords of ALL cells in the visible range (including those behind obstacles)
        orientation = self.robot_info.get_orientation()
        cur_coord = self.robot_info.get_coord()

        # cells within robot 3x3 are obviously explored and not obstacles
        # self.no_obs_coord_list.extend([(coord, 1) for coord in self.arena.get_eight_adjacent_in_arena(cur_coord)]) # distance is 1
        # self.no_obs_coord_list.append((cur_coord, 0)) # distance is 0

        for sensor in SENSOR_CONSTANTS.values():
            sensor_abs_degree = (sensor['direction'] + OT.orientation_to_degree[orientation]) % 360
            displacement_per_step = OT.orientation_to_unit_displacement(OT.degree_to_orientation[sensor_abs_degree])
            sensor_displacement_key = 'displacement_'+str(orientation.value) # key changes based on orientation of robot
            cil = self.get_coords_in_line(sensor[sensor_displacement_key], displacement_per_step, sensor['range'])
            self.update_explored_coord_lists(cil)

        return self.obstacles_coord_list, self.no_obs_coord_list

    def update_explored_coord_lists(self, coords: list) -> None:
        # gets cell at coord and adds it to the self blocked and self unblocked lists
        distance = 2 # detected cells start from 2 units away from center of robot
        for coord in coords:
            cell = self.arena.get_cell_at_coord(coord)
            if cell.is_obstacle():
                self.obstacles_coord_list.append((coord, distance))
                break
            else:
                self.no_obs_coord_list.append((coord, distance))
            distance += 1
    
    def get_coords_in_line(self, sensor_displacement: Coord, displacement_per_step: Coord, view_range: int) -> list:
        all_possible_visible_coords = []
        cur_coord = self.robot_info.get_coord()
        for i in range(view_range):
            coord = cur_coord.add(sensor_displacement).add(displacement_per_step.multiply(i))
            if self.arena.coord_is_valid(coord):
                all_possible_visible_coords.append(coord)
            else:
                break
        return all_possible_visible_coords
