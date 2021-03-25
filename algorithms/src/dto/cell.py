from src.dto.constants import Orientation

class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.visited = False
        self.obstacle = 0
        self.danger = None # danger when too close to wall, thus danger of collision
        self.seen = False
        self.seen_surface = { # only for dangerous cells
                Orientation.NORTH: False,
                Orientation.EAST: False,
                Orientation.SOUTH: False,
                Orientation.WEST: False
            }

    def get_coord(self):
        return self.coord

    def set_coord(self, coord):
        self.coord = coord
        return self

    def is_explored(self):
        return self.explored

    def set_is_explored(self, explored):
        self.explored = explored
        return self

    def is_visited(self):
        return self.visited

    def set_is_visited(self, visited):
        self.visited = visited
        return self

    def is_obstacle(self):
        return self.obstacle > 1

    def increment_is_obstacle(self, delta=1):
        if delta < 1:
            delta = 1
        self.obstacle += delta
        if self.obstacle > 8:
            self.obstacle = 8
        return self

    def decrement_is_obstacle(self, delta=1):
        if delta < 1:
            delta = 1
        self.obstacle -= delta
        if self.obstacle < -7:
            self.obstacle = -7
        return self

    def is_dangerous(self):
        return self.danger

    def set_is_dangerous(self, danger):
        self.danger = danger
        return self

    def is_seen(self):
        return self.seen
    
    def set_is_seen(self, seen):
        self.seen = seen
        return self

    def set_seen_surface(self, face):
        if face == 0:
            orientation = Orientation.NORTH
        elif face == 90:
            orientation = Orientation.EAST
        elif face == 180:
            orientation = Orientation.SOUTH
        elif face == 270:
            orientation = Orientation.WEST
        else: raise Exception(f'Invalid surface orientation')

        if self.obstacle:
            self.seen_surface[orientation] = True

    def get_seen_surfaces(self):
        return self.seen_surface 