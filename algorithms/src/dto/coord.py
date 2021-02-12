class Coord:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def get_x(self):
        return self.x
    
    def set_x(self, x):
        self.x = x
    
    def get_y(self):
        return self.y
    
    def set_y(self, y):
        self.y = y
    
    def add(self, operand):
        return Coord(
            self.get_x() + operand.get_x(),
            self.get_y() + operand.get_y()
        )
 
    def subtract(self, operand):
        return Coord(
            self.get_x() - operand.get_x(),
            self.get_y() - operand.get_y()
        )
    
    def manhattan_distance(self) -> int:
        return abs(self.x) + abs(self.y)
    
    def is_equal(self, coord) -> bool:
        return self.x == coord.get_x and self.y == coord.get_y
