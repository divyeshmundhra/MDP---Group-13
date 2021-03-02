import sys, os, time, math
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
from src.dto.constants import SENSOR_CONSTANTS
from src.dto.coord import Coord
from src.dto.arena import Arena
from src.dto.OrientationTransform import OrientationTransform as OT

class SensorParser():
    @staticmethod
    def main_sensor_parser(sensor_dict, robot_info):
        sensor_data = sensor_dict['data']
        cur_coord = robot_info.get_coord()
        orientation = robot_info.get_orientation()
        # cur_coord = Coord(6, 6)
        # orientation = Orientation.EAST
        obstacleList = []
        exploredList = []
        i = 0
        for key, val in SENSOR_CONSTANTS.items():
            sensor_abs_degree = (
                val['direction'] + OT.orientation_to_degree[orientation]
            ) % 360
            string1 = 'displacement_'+str(orientation.value)
            displacement_per_step = OT.orientation_to_unit_displacement(
                OT.degree_to_orientation[sensor_abs_degree])
            obstacleList.extend(SensorParser.individual_sensor_parser(
                cur_coord, val[string1], displacement_per_step, sensor_data[key], val['range'])[0])
            exploredList.extend(SensorParser.individual_sensor_parser(
                cur_coord, val[string1], displacement_per_step, sensor_data[key], val['range'])[1])
            i += 1
        # for item in obstacleList:
        #     for j in item:
        #         print(j.get_x(), j.get_y())
        # print("Break")
        # for i in exploredList:
        #     for z in i:
        #         print(z.get_x(), z.get_y())
        return obstacleList, exploredList

    @staticmethod
    def individual_sensor_parser(currentPos,sensor_displacement,displacement_per_step,sensor_value,view_range):
        obstacle = []
        empty = []
        arena = Arena()

        # 1 if robot doesn't see any obstacle
        flag=0

        # if the sensor sees no obstacle
        if not sensor_value:
            sensor_value = view_range
            flag=1

        for i in range(1,sensor_value-1):
            cd = currentPos.add(sensor_displacement).add(
                displacement_per_step.multiply(i))
            if(arena.coord_is_valid(cd)):
                empty.append(cd)
        if (flag==0):
            detectedobstacle = currentPos.add(sensor_displacement).add(
                displacement_per_step.multiply(sensor_value-1))
            if(arena.coord_is_valid(detectedobstacle)):
                obstacle.append(detectedobstacle)

        return obstacle, empty
