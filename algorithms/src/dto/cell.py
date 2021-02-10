class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.obstacle = None
        
    def get_coord(self):
        return self.coord
    
    def set_coord(self, coord):
        self.coord = coord

    def is_explored(self):
        return self.explored
    
    def set_is_explored(self, explored):
        self.explored = explored
    
    def is_obstacle(self):
        return self.obstacle

    def set_is_obstacle(self, obstacle):
        self.obstacle = obstacle