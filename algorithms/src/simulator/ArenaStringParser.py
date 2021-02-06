from src.dto.arena import Arena
from src.dto.constants import *
from src.dto.coord import Coord

# Read the arena text file and store it as a list ==========================================
f = open("sample_arena.txt", "r") #import the arena file (this is for testing, for the actual we will have to import from RPi)
arena_list = list(f.read())

# Create arena object
arena = Arena()

i=0
for y in range(MAP_ROW):
    for x in range(MAP_COL):
        coord = Coord(x, y)
        if arena_list[i] == '\n':
            i = i+1 
        if arena_list[i] == '1':
            arena.get_cell_at_coord(coord).set_obstacle_flag(True)
        elif arena_list[i] == '0':
            arena.get_cell_at_coord(coord).set_obstacle_flag(False)
        i = i+1