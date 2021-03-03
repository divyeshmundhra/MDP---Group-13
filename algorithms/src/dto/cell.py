class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.visited = False
        self.obstacle = 0
        self.danger = None # danger when too close to wall, thus danger of collision

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
        return self.obstacle > 0

    def increment_is_obstacle(self):
        if self.obstacle < 4:
            self.obstacle += 1
        return self

    def decrement_is_obstacle(self):
        if self.obstacle > -3:
            self.obstacle -= 1
        return self

    def is_dangerous(self):
        return self.danger

    def set_is_dangerous(self, danger):
        self.danger = danger
        return self
