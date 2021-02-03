class Cell:
    def __init__(self, row, col):
        self.row = row
        self.col = col
        self.wall = None

    def getRow(self):
        return self.row

    def getCol(self):
        return self.col

    def setObstacle(self, wall):
        self.wall = wall

    def getObstacle(self):
        return self.wall

    def setIsExplored(self, val):
        self.isExplored = val

    def getIsExplored(self):
        return self.isExplored