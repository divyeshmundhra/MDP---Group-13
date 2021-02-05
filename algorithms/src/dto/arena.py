class Arena:
    def __init__(self):
        self.cell_matrix = [[None for y in range(20)] for x in range(15)]
    
    def set_cell_at_coord(self, cell, coord):
        self.cell_matrix[coord.get_x()][coord.get_y()] = cell

    def get_cell_at_coord(self, coord):
        return self.cell_matrix[coord.get_x()][coord.get_y()]