from src.dto.arena import Arena
from src.dto.constants import *
from src.dto.coord import Coord

class ArenaStringParser:
    @staticmethod
    def parse_arena_string(string: str) -> Arena:
        cl = list(string)
        arena = Arena()

        # set obstacles in arena
        obstacle_list = []
        i=0
        for y in range(MAP_ROW-1, -1, -1): # counting from top to bottom
            for x in range(MAP_COL):
                coord = Coord(x, y)
                if cl[i] == '\n':
                    i = i + 1
                if cl[i] == '1':
                    arena.get_cell_at_coord(coord).set_is_obstacle(True)
                    obstacle_list.append(coord)
                elif cl[i] == '0':
                    arena.get_cell_at_coord(coord).set_is_obstacle(False)
                i = i+1
                if y in [0,MAP_ROW-1] or x in [0,MAP_COL-1]:
                    # cells at edge of arena are too close to the walls
                    arena.get_cell_at_coord(coord).set_is_dangerous(True)

        # set danger flag for cells too close to obstacles
        for obs in obstacle_list:
            displacements = [
                Coord(-1, -1),
                Coord(-1, 0),
                Coord(-1, 1),
                Coord(0, -1),
                Coord(0, 1),
                Coord(1, -1),
                Coord(1, 0),
                Coord(1, 1)
            ]
            for d in displacements:
                dangerous_coord = obs.add(d)
                if dangerous_coord.get_x() < 15 and dangerous_coord.get_y() < 20:
                    arena.get_cell_at_coord(dangerous_coord).set_is_dangerous(True)
        
        return arena
