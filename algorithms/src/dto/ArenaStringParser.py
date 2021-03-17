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
                    arena.get_cell_at_coord(coord).increment_is_obstacle()
                    obstacle_list.append(coord)
                elif cl[i] == '0':
                    pass # not obstacle assumed
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
                if 0 <= dangerous_coord.get_x() < 15 and 0 <= dangerous_coord.get_y() < 20:
                    arena.get_cell_at_coord(dangerous_coord).set_is_dangerous(True)
        
        return arena

    @staticmethod
    def parse_p1_p2(p1: str, p2: str) -> Arena:
        p1_str= bin(int(p1, 16))[2:]

        if len(p1_str) != 304:
            raise Exception(f'Expected p1 string to be of length 304, got {len(p1_str)} instead')

        # 2:-2 to strip out padding bits
        p1_bin = list(p1_str[2:-2])

        # zfill to put back leading zeros
        p2_bin = list(bin(int(p2, 16))[2:].zfill(len(p2) * 4))

        arena = Arena()

        for y in range(MAP_ROW):
            for x in range(MAP_COL):
                explored = p1_bin.pop(0) == "1"

                if not explored:
                    continue

                coord = Coord(x, y)
                arena.get_cell_at_coord(coord).set_is_explored(True)

                obstacle = p2_bin.pop(0) == "1"

                if obstacle:
                    arena.get_cell_at_coord(coord).increment_is_obstacle()

        return arena
