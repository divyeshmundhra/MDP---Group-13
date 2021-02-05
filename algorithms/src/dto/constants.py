from enum import Enum
# Defining colours
blue=[0,0,255]
white = [255, 255, 255]
black = [0,0,0]
grey = [100,100,100]
red = [255,0,0]
green = [0,255,0]
yellow = [255,255,0]

# Defining map dimensions
map_row = 20 
map_col = 15
tile_size = 20
dis_x = 300
dis_y = 400

class Orientation(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3
        