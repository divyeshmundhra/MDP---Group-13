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
    
    def add(self, operand: Coord) -> Coord:
        return Coord(
            self.get_x() + operand.get_x(),
            self.get_y() + operand.get_y()
        )
 
    def subtract(self, operand: Coord) -> Coord:
        return Coord(
            self.get_x() - operand.get_x(),
            self.get_y() - operand.get_y()
        )