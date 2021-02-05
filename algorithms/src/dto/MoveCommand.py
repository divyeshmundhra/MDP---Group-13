class MoveCommand:
    def __init__(self, turn_angle, cells_to_advance):
        # turn angle is anchored on robot, i.e. 0 degrees is directly in front of robot, 90 degrees is a right turn
        self.turn_angle = turn_angle
        self.cells_to_advance = cells_to_advance

    def get_turn_angle(self):
        return self.turn_angle
    
    def set_turn_angle(self, turn_angle):
        self.turn_angle = turn_angle
    
    def get_cells_to_advance(self):
        return self.cells_to_advance

    def set_cells_to_advance(self, cells_to_advance):
        self.cells_to_advance = cells_to_advance