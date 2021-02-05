class Agent:
    def __init__(self, arena, robot_info):
        self.arena = arena
        self.robot_info = robot_info
    
    def step(self, obstacles_coord_list):
        update_arena(obstacles_coord_list)
        target_coord = think()
        move_command = calculate_move(target_coord)
        agent_output = None # implemented in different branch
        return agent_output
    
    def update_arena(obstacles_coord_list):
        pass

    def think():
        return None
    
    def calculate_move(target_coord):
        return None
