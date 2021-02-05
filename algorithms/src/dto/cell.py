class Cell:
    def __init__(self, coord):
        self.coord = coord
        self.explored = False
        self.obstacle = None