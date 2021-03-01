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

from src.dto.constants import AgentTask, START_COORD


class Agent:
    def __init__(self, arena_string: str, robot_info: RobotInfo, task: AgentTask, end_coord: Coord, waypoint_coord: Coord):
        self.arena = ArenaStringParser.parse_arena_string(arena_string)
        self.robot_info = robot_info
        self.task = task # task = enum: fastest path or exploration
        self.end_coord = end_coord
        self.waypoint_coord = waypoint_coord
        self.reached_waypoint = False
        self.algo = None # initialized 3 lines below
        
        if self.task == AgentTask.FAST:
            self.algo = FastestPathAlgo()
        else:
            self.algo = ExplorationAlgo()
            self.reached_waypoint = True
    
    def step(self, obstacles_coord_list: list, no_obs_coord_list: list, robot_info: RobotInfo) -> AgentOutput:
        self.robot_info = robot_info
        self.update_arena(obstacles_coord_list, no_obs_coord_list)
        target_coord = self.think()
        if target_coord == None:
            # debug code
            # MAP_ROW = 20
            # MAP_COL = 15
            # for y in range(MAP_ROW):
            #     for x in range(MAP_COL):
            #         if not self.arena.get_cell_at_coord(Coord(x,y)).is_explored():
            #             print(x,' ',y)
            # /debug code
            if self.task == AgentTask.FAST:
                message = f'No valid path!'
                move_command = None
            else:
                message = f'Exploration complete!'
                self.fill_remaining_unexplored_with_obstacles()
                explorationDoneAlgo = FastestPathAlgo()
                next_step = explorationDoneAlgo.get_next_step(self.arena,self.robot_info,START_COORD, None)
                move_command = self.calculate_move(next_step)
        elif self.task == AgentTask.FAST and self.robot_info.get_coord().is_equal(self.end_coord):
            message = f'Fastest path complete!'
            move_command = None
        else:
            move_command = self.calculate_move(target_coord)
            message = f'Target: {target_coord.get_x()}, {target_coord.get_y()}, TURN: {move_command.get_turn_angle()} degs, then \
                MOVE: {move_command.get_cells_to_advance()} cells forwards'
        return AgentOutput(
            move_command,
            message
        )

    def update_arena(self, obstacles_coord_list: list, no_obs_coord_list: list) -> None:
        self.arena.get_cell_at_coord(self.robot_info.get_coord()).set_is_visited(True)
        for coord in obstacles_coord_list:
            # mark seen obstacles as explored
            cell = self.arena.get_cell_at_coord(coord)
            cell.set_is_obstacle(True)
            self.arena.mark_dangerous_cells_around_obstacle(coord)
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        for coord in no_obs_coord_list:
            # mark seen clear cells as explored
            self.arena.get_cell_at_coord(coord).set_is_explored(True)
        if not self.reached_waypoint and self.robot_info.get_coord().is_equal(self.waypoint_coord):
            self.reached_waypoint = True

    def think(self) -> Coord:
        if self.task == AgentTask.FAST:
            waypoint = None if self.reached_waypoint else self.waypoint_coord
            next_step = self.algo.get_next_step(self.arena, self.robot_info, self.end_coord, waypoint)
        else:
            next_step = self.algo.get_next_step(self.arena, self.robot_info) # pylint: disable=no-value-for-parameter
        return next_step

    def calculate_move(self, target_coord) -> MoveCommand:
        current_coord = self.robot_info.get_coord()
        displacement = target_coord.subtract(current_coord)
        target_orientation = OrientationTransform.displacement_to_orientation(displacement)
        turn_angle = OrientationTransform.calc_degree_of_turn(self.robot_info.get_orientation(), target_orientation)
        
        return MoveCommand(
            turn_angle,
            displacement.manhattan_distance()
        )

    def fill_remaining_unexplored_with_obstacles(self):
        # rationale, these unexplored cells are dangerous and are trapped by dangerous cells
        # they are thus probably obstacles
        for cell in self.arena.list_unexplored_cells():
            cell.set_is_explored(True)
            # cell.set_is_obstacle(True)
            cell.set_is_dangerous(True)
    def get_arena(self) -> Arena:
        return self.arena
