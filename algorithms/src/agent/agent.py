# from src.dto import Coord, MoveCommand, AgentOutput, Arena, RobotInfo, OrientationTransform
from src.dto.coord import Coord
from src.dto.MoveCommand import MoveCommand
from src.dto.AgentOutput import AgentOutput
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform
from src.dto.arena import Arena
from src.dto.ArenaStringParser import ArenaStringParser

# from src.agent import FastestPathAlgo, ExplorationAlgo
from src.agent.FastestPathAlgo import FastestPathAlgo
from src.agent.ExplorationAlgo import ExplorationAlgo

from src.dto.constants import AgentTask

class Agent:
    def __init__(self, arena_string: str, robot_info: RobotInfo, task: AgentTask, end_coord: Coord, waypoint_coord: Coord):
        self.arena = ArenaStringParser.parse_arena_string(arena_string)
        self.robot_info = robot_info
        self.task = task # task = enum: fastest path or exploration
        self.end_coord = end_coord
        self.waypoint_coord = waypoint_coord
        self.reached_waypoint = False
    
    def step(self, obstacles_coord_list: list, robot_info: RobotInfo) -> AgentOutput:
        self.robot_info = robot_info
        self.update_arena(obstacles_coord_list)
        target_coord = self.think()
        if target_coord == None:
            message = f'No valid path!'
            move_command = None
        else:
            move_command = self.calculate_move(target_coord)
            message = f'TURN: {move_command.get_turn_angle()} degs, then \
                MOVE: {move_command.get_cells_to_advance()} cells forwards' # implement messaging later
        return AgentOutput(
            move_command,
            message
        )
    
    def update_arena(self, obstacles_coord_list: list) -> None:
        for coord in obstacles_coord_list:
            cell = self.arena.get_cell_at_coord(coord)
            cell.set_is_obstacle(True)
        if not self.reached_waypoint and self.robot_info.get_coord().is_equal(self.waypoint_coord):
            self.reached_waypoint = True

    def think(self) -> Coord:
        if self.task == AgentTask.FAST:
            algo = FastestPathAlgo()
        else:
            algo = ExplorationAlgo()
        waypoint = None if self.reached_waypoint else self.waypoint_coord
        return algo.get_next_step(self.arena, self.robot_info, self.end_coord, waypoint)

    def calculate_move(self, target_coord) -> MoveCommand:
        current_coord = self.robot_info.get_coord()
        displacement = target_coord.subtract(current_coord)
        target_orientation = OrientationTransform.displacement_to_orientation(displacement)
        turn_angle = OrientationTransform.calc_degree_of_turn(self.robot_info.get_orientation(), target_orientation)
        
        return MoveCommand(
            turn_angle,
            displacement.manhattan_distance()
        )
