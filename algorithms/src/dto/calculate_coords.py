from src.dto.constants import *
from src.dto.coord import Coord

class CalculateCoords:
    def __init__(self, coord: Coord):
        self.x = coord.get_x()
        self.y = coord.get_y()

    # directly straight
    def displacement_straight(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x , self.y + displacement)
        elif orientation == Orientation.EAST:
            if 0 <= self.x + displacement < MAP_COL:
                return Coord(self.x + displacement, self.y)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x , self.y - displacement)
        elif orientation == Orientation.WEST:
            if 0 <= self.x - displacement < MAP_COL:
                return Coord(self.x - displacement, self.y)

        return None
    
    # top left tile, moving in the forward direction
    def displacement_straight_left(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x - 1 < MAP_COL and 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x-1 , self.y + displacement)
        elif orientation == Orientation.EAST:
            if 0 <= self.x + displacement < MAP_COL and 0 <= self.y + 1 < MAP_ROW:
                return Coord(self.x + displacement, self.y+1)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x+1 < MAP_COL and 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x+1 , self.y - displacement)
        elif orientation == Orientation.WEST:
            if 0 <= self.x - displacement < MAP_COL and 0 <= self.y -1 < MAP_ROW:
                return Coord(self.x - displacement, self.y-1)

        return None

    # top right tile, moving in the forward direction
    def displacement_straight_right(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x+1 < MAP_COL and 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x+1, self.y + displacement)
        elif orientation == Orientation.EAST:
            if 0 <= self.x + displacement < MAP_COL and 0 <= self.y-1 < MAP_ROW:
                return Coord(self.x + displacement, self.y-1)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x - 1 < MAP_COL and 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x-1, self.y - displacement)
        elif orientation == Orientation.WEST:
            if 0 <= self.x - displacement < MAP_COL and 0 <= self.y + 1 < MAP_ROW:
                return Coord(self.x - displacement, self.y+1)

        return None

    # directly left
    def displacement_left(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x - displacement < MAP_COL:
                return Coord(self.x - displacement , self.y)
        elif orientation == Orientation.EAST:
            if 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x, self.y + displacement)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x + displacement < MAP_COL:
                return Coord(self.x + displacement, self.y)
        elif orientation == Orientation.WEST:
            if 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x, self.y - displacement)

        return None

    # top left tile, moving in the left direction
    def displacement_left_up(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x - displacement-1 < MAP_COL:
                return Coord(self.x - displacement -1 , self.y)
        elif orientation == Orientation.EAST:
            if 0 <= self.y + displacement+1 < MAP_ROW:
                return Coord(self.x, self.y + displacement+1)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x + displacement+1 < MAP_COL:
                return Coord(self.x + displacement +1, self.y)
        elif orientation == Orientation.WEST:
            if 0 <= self.y - displacement-1 < MAP_ROW:
                return Coord(self.x, self.y - displacement -1)

        return None

    # bottom left tile, moving in the left direction
    def displacement_left_down(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x - displacement < MAP_COL and 0 <= self.y-1 < MAP_ROW:
                return Coord(self.x - displacement , self.y-1)
        elif orientation == Orientation.EAST:
            if 0 <= self.x - 1 < MAP_COL and 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x-1, self.y + displacement)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x + displacement < MAP_COL and 0 <= self.y+1 < MAP_ROW:
                return Coord(self.x + displacement, self.y+1)
        elif orientation == Orientation.WEST:
            if 0 <= self.x +1 < MAP_COL and 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x+1, self.y - displacement)

        return None

    # directly right
    def displacement_right(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x + displacement < MAP_COL:
                return Coord(self.x + displacement, self.y)
        elif orientation == Orientation.EAST:
            if 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x, self.y - displacement)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x - displacement < MAP_COL:
                return Coord(self.x - displacement, self.y)
        elif orientation == Orientation.WEST:
            if 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x, self.y + displacement)

        return None

    # top right tile, moving in the right direction
    def displacement_right_top(self, displacement: int, orientation: Orientation) -> Coord: # TODO: catch out of bounds error!
        if orientation == Orientation.NORTH:
            if 0 <= self.x + displacement < MAP_COL and 0 <= self.y + 1 < MAP_ROW:
                return Coord(self.x + displacement, self.y+1)
        elif orientation == Orientation.EAST:
            if 0 <= self.x +1 < MAP_COL and 0 <= self.y - displacement < MAP_ROW:
                return Coord(self.x+1, self.y - displacement)
        elif orientation == Orientation.SOUTH:
            if 0 <= self.x - displacement < MAP_COL and 0 <= self.y-1 < MAP_ROW:
                return Coord(self.x - displacement, self.y-1)
        elif orientation == Orientation.WEST:
            if 0 <= self.x - 1 < MAP_COL and 0 <= self.y + displacement < MAP_ROW:
                return Coord(self.x-1, self.y + displacement)
        
        return None