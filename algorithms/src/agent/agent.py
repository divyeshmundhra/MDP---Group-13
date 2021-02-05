from src.dto import Coord, MoveCommand, AgentOutput, Arena, RobotInfo, OrientationTransform
from src.agent import FastestPathAlgo, ExplorationAlgo
from src.dto.constants import AgentTask

class Agent:
    def __init__(self, arena: Arena, robot_info: RobotInfo, task: AgentTask, end_coord: Coord, waypoint_coord: Coord):
        self.arena = arena
        self.robot_info = robot_info
        self.task = task # task = enum: fastest path or exploration
        self.end_coord = end_coord
        self.waypoint_coord = waypoint_coord
    
    def step(self, obstacles_coord_list: list) -> AgentOutput:
        self.update_arena(obstacles_coord_list)
        target_coord = self.think()
        move_command = self.calculate_move(target_coord)
        message = '' # implement messaging later
        return AgentOutput.AgentOutput(
            move_command,
            message
        )
    
    def update_arena(self, obstacles_coord_list: list) -> None:
        for coord in obstacles_coord_list:
            cell = self.arena.get_cell_at_coord(coord)
            cell.set_obstacle_flag(True)

    def think(self) -> Coord:
        if self.task == AgentTask.FAST:
            algo = FastestPathAlgo.FastestPathAlgo
        else:
            algo = ExplorationAlgo.ExplorationAlgo
        return algo.get_next_step(self.arena, self.robot_info, self.end_coord, self.waypoint_coord)

    def calculate_move(self, target_coord) -> MoveCommand:
        current_coord = self.robot_info.get_coord()
        displacement = target_coord.subtract(current_coord)
        target_orientation = OrientationTransform.OrientationTransform.displacement_to_orientation(displacement)
        turn_angle = OrientationTransform.OrientationTransform.calc_degree_of_turn(self.robot_info.get_orientation(), target_orientation)
        return MoveCommand.MoveCommand(
            turn_angle,
            displacement.manhattan_distance()
        )
