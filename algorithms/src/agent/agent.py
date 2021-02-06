from src.dto import Coord, MoveCommand, AgentOutput, Arena, RobotInfo
from src.agent import FastestPathAlgo, ExplorationAlgo
from src.dto.constants import Orientation, AgentTask

class Agent:
    def __init__(self, arena: Arena, robot_info: RobotInfo, task: AgentTask):
        self.arena = arena
        self.robot_info = robot_info
        self.task = task # task = enum: fastest path or exploration
    
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
            algo = FastestPathAlgo.FastestPathAlgo()
        else:
            algo = ExplorationAlgo.ExplorationAlgo()
        return algo.get_next_step(self.arena)
    
    def calculate_move(self, target_coord) -> MoveCommand:
        current_coord = self.robot_info.get_coord()
        displacement = target_coord.subtract(current_coord)
        target_orientation = OrientationTransform.displacement_to_orientation(displacement)
        turn_angle = OrientationTransform.calc_degree_of_turn(self.robot_info.get_orientation(), target_orientation)
        return MoveCommand.MoveCommand(
            turn_angle,
            displacement.manhattan_distance()
        )

class OrientationTransform:
    orientation_to_degree = {
        Orientation.NORTH: 0,
        Orientation.EAST: 90,
        Orientation.SOUTH: 180,
        Orientation.WEST: 270
    }

    @classmethod
    def calc_degree_of_turn(cls, source: Orientation, target: Orientation) -> int:
        source_deg = OrientationTransform.orientation_to_degree[source]
        target_deg = OrientationTransform.orientation_to_degree[target]
        return (target_deg - source_deg + 360) % 360
    
    @classmethod
    def displacement_to_orientation(cls, displacement: Coord) -> Orientation:
        x = displacement.get_x()
        y = displacement.get_y()
        if x > 0 and y == 0: return Orientation.EAST
        elif x < 0 and y == 0: return Orientation.WEST
        elif x == 0 and y > 0: return Orientation.NORTH
        elif x == 0 and y < 0: return Orientation.SOUTH
        else: raise Exception('Invalid displacement: displacement_to_orientation only implemented for cardinal directions')
