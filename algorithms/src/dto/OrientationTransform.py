from src.dto.constants import Orientation
from src.dto.coord import Coord

class OrientationTransform:
    orientation_to_degree = {
        Orientation.NORTH: 0,
        Orientation.EAST: 90,
        Orientation.SOUTH: 180,
        Orientation.WEST: 270
    }

    degree_to_orientation = {
        0: Orientation.NORTH,
        90: Orientation.EAST,
        180: Orientation.SOUTH,
        270: Orientation.WEST
    }

    @staticmethod
    def calc_degree_of_turn(source: Orientation, target: Orientation) -> int:
        source_deg = OrientationTransform.orientation_to_degree[source]
        target_deg = OrientationTransform.orientation_to_degree[target]
        return (target_deg - source_deg + 360) % 360

    @staticmethod
    def calc_orientation_after_turn(source: Orientation, turn_degree: int) -> Orientation:
        source_deg = OrientationTransform.orientation_to_degree[source]
        target_deg = (source_deg + turn_degree) % 360
        return OrientationTransform.degree_to_orientation[target_deg]
    
    @staticmethod
    def displacement_to_orientation(displacement: Coord) -> Orientation:
        x = displacement.get_x()
        y = displacement.get_y()
        # this if list should be refactored into a constant, along with the one in Arena.py
        if x > 0 and y == 0: return Orientation.EAST
        elif x < 0 and y == 0: return Orientation.WEST
        elif x == 0 and y > 0: return Orientation.NORTH
        elif x == 0 and y < 0: return Orientation.SOUTH
        else: raise Exception(f'Invalid displacement: tried moving {x}, {y}. displacement_to_orientation only implemented for cardinal directions')
    
    @staticmethod
    def orientation_to_unit_displacement(orientation: Orientation) -> Coord:
        if orientation == Orientation.EAST: return Coord(1, 0)
        elif orientation == Orientation.WEST: return Coord(-1, 0)
        elif orientation == Orientation.NORTH: return Coord(0, 1)
        elif orientation == Orientation.SOUTH: return Coord(0, -1)
        else: raise Exception(f'Invalid orientation: tried moving {orientation.name}. orientation_to_unit_displacement only implemented for cardinal directions')