class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.obstacle = None
        
    def get_coord(self):
        return self.coord
    
    def set_coord(self, coord):
        self.coord = coord

    def get_explored_flag(self):
        return self.explored
    
    def set_explored_flag(self, explored):
        self.explored = explored
    
    def get_obstacle_flag(self):
        return self.obstacle

    def set_obstacle_flag(self, obstacle):
        self.obstacle = obstacle