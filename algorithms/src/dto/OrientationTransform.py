from src.dto.constants import Orientation
from src.dto import Coord

class OrientationTransform:
    orientation_to_degree = {
        Orientation.NORTH: 0,
        Orientation.EAST: 90,
        Orientation.SOUTH: 180,
        Orientation.WEST: 270
    }

    @staticmethod
    def calc_degree_of_turn(source: Orientation, target: Orientation) -> int:
        source_deg = OrientationTransform.orientation_to_degree[source]
        target_deg = OrientationTransform.orientation_to_degree[target]
        return (target_deg - source_deg + 360) % 360
    
    @staticmethod
    def displacement_to_orientation(displacement: Coord) -> Orientation:
        x = displacement.get_x()
        y = displacement.get_y()
        # this if list should be refactored into a constant, along with the one in Arena.py
        if x > 0 and y == 0: return Orientation.EAST
        elif x < 0 and y == 0: return Orientation.WEST
        elif x == 0 and y > 0: return Orientation.NORTH
        elif x == 0 and y < 0: return Orientation.SOUTH
        else: raise Exception('Invalid displacement: displacement_to_orientation only implemented for cardinal directions')
