from ..dto.cell import Cell
from constants import *

# Read the arena text file and store it as a list ==========================================
f = open("sample_arena.txt", "r") #import the arena file
arena_list = list(f.read())

# Create the grid for the map ==============================================================
class Grid:
    grid = [[0 for x in range(map_col)] for y in range(map_row)] 

    # create Cell objects for each cell in the map and set obstacle to true/false
    i = 0
    for row in range(map_row):
        for col in range(map_col):
            grid[row][col] = Cell(row, col)
            if arena_list[i] == '\n':
                i = i+1 
            if arena_list[i] == '1':
                grid[row][col].setObstacle(True)
            elif arena_list[i] == '0':
                grid[row][col].setObstacle(False)
            i = i+1