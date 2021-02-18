class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.visited = False
        self.obstacle = None
        self.danger = None # danger when too close to wall, thus danger of collision

    def get_coord(self):
        return self.coord

    def set_coord(self, coord):
        self.coord = coord

    def is_explored(self):
        return self.explored

    def set_is_explored(self, explored):
        self.explored = explored

    def is_visited(self):
        return self.visited

    def set_is_visited(self, visited):
        self.visited = visited

    def is_obstacle(self):
        return self.obstacle

    def set_is_obstacle(self, obstacle):
        self.obstacle = obstacle

    def is_dangerous(self):
        return self.danger

    def set_is_dangerous(self, danger):
        self.danger = danger
