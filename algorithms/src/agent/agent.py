from src.dto.coord import Coord
from src.dto.MoveCommand import MoveCommand
from src.dto.AgentOutput import AgentOutput
from src.dto.RobotInfo import RobotInfo
from src.dto.OrientationTransform import OrientationTransform as OT
from src.dto.arena import Arena
from src.dto.ArenaStringParser import ArenaStringParser

# from src.agent import FastestPathAlgo, ExplorationAlgo
from src.agent.FastestPathAlgo import FastestPathAlgo
from src.agent.ExplorationAlgo import ExplorationAlgo

from src.agent.ExploreDangerousAlgo import ExploreDangerousAlgo
from src.agent.LeftWallHug import LeftWallHuggingAlgo
from src.agent.ImageRecognition import RightWallHuggingAlgo

from src.dto.constants import AgentTask, START_COORD
from threading import Timer

class Agent:
    def __init__(self, arena_string: str, robot_info: RobotInfo, task: AgentTask, end_coord: Coord, waypoint_coord: Coord):
        self.robot_info = robot_info
        self.expected_robot_info = None # expected robot info after move if move succeeds
        self.task = task # task = enum: fastest path or exploration
        self.end_coord = end_coord
        self.waypoint_coord = waypoint_coord
        self.reached_waypoint = False
        self.exploration_complete = False
        self.dangerous_exploration_path = None
        self.algo = None # initialized 3 lines below

        self.image_rec_back_to_start = False
        self.image_rec_going_to_next_obstacle = False # False for RWH, True for FP
        self.RWH_start_coord = START_COORD # initialise this to the starting coord
        self.target_obstacle = None
        
        self.LWH_start_coord = START_COORD
        self.lwh_back_to_start = False
        self.commencing_exploration = False

        t = Timer(270, self.timeout)
        self.time_ran_out = False
        t.start()

        if self.task == AgentTask.FAST:
            self.arena = ArenaStringParser.parse_arena_string(arena_string)
            self.algo = FastestPathAlgo()
            self.arena.set_all_explored()
        elif self.task == AgentTask.EXPLORE:
            self.arena = Arena()
            # self.algo = ExplorationAlgo()
            self.algo = LeftWallHuggingAlgo()
            self.reached_waypoint = True
        else:
            self.arena = Arena()
            self.algo = RightWallHuggingAlgo()
            self.reached_waypoint = True

    def timeout(self):
        self.time_ran_out = True

    def calc_percepts(self, obstacles_coord_list: list, no_obs_coord_list: list, move_q_size: int = 0) -> None:
        # update real robot info
        if self.expected_robot_info:
            if move_q_size == 0:
                # robot has finished move, set position to expected position after move
                self.robot_info.set_coord(self.expected_robot_info.get_coord())
            self.robot_info.set_orientation(self.expected_robot_info.get_orientation())
        if self.task == AgentTask.EXPLORE or self.task == AgentTask.IMAGEREC:
            self.update_arena(obstacles_coord_list, no_obs_coord_list)
        elif not self.reached_waypoint and self.robot_info.get_coord().is_equal(self.waypoint_coord):
            self.reached_waypoint = True

    def step(self) -> AgentOutput:
        cur_coord = self.robot_info.get_coord()

        if self.time_ran_out:
            message = f'4.5 minutes reached, returning to start!'
            self.exploration_complete = True
            target_coord = FastestPathAlgo().get_next_step(self.arena,self.robot_info, START_COORD)
            move_command = self.calculate_move(cur_coord, target_coord)
            return AgentOutput(
                move_command,
                message
            )

        if self.exploration_complete:
            if self.robot_info.get_coord().is_equal(self.waypoint_coord):
                self.waypoint_coord = self.dangerous_exploration_path.pop(0) if self.dangerous_exploration_path else None
                # look around
                return AgentOutput(
                    MoveCommand(360, 0),
                    'Looking around to see unexplored dangerous cell'
                )
            target_coord = FastestPathAlgo().get_next_step(self.arena,self.robot_info,self.waypoint_coord)
        else:
            target_coord = self.think()

        if target_coord == None:
            if self.task == AgentTask.FAST:
                message = f'No valid path!'
                move_command = None
            elif self.task == AgentTask.EXPLORE:
                message = f'Explored all non-dangerous cells!'
                self.exploration_complete = True
                # self.fill_remaining_unexplored_with_obstacles()
                self.dangerous_exploration_path = ExploreDangerousAlgo(self.arena, self.robot_info).calculate_cheapest_path()
                self.dangerous_exploration_path.append(START_COORD)
                self.waypoint_coord = self.dangerous_exploration_path.pop(0)
                target_coord = FastestPathAlgo().get_next_step(self.arena,self.robot_info, self.waypoint_coord)
                move_command = self.calculate_move(cur_coord, target_coord)
            else:
                # Image rec target coord returns None if the FP has reached its goal
                # Once we use FP to reach the next obstacle, we need to align it with the wall before we can start RWH
                if self.RWH_start_coord == None:
                    message = f'Image Recognition Right Wall Hugging complete!'
                    move_command = None
                else:
                    message = f'Reached dangerous obstacle, commencing right wall hugging!'
                    next_step = self.algo.align_right_wall(self.arena, self.robot_info, self.target_obstacle)
                    move_command = self.calculate_move(cur_coord, next_step)
                
        elif self.task == AgentTask.FAST and self.robot_info.get_coord().is_equal(self.end_coord):
            message = f'Fastest path complete!'
            move_command = None

        else:
            move_command = self.calculate_move(cur_coord, target_coord)
            message = f'Target: {target_coord.get_x()}, {target_coord.get_y()}, TURN: {move_command.get_turn_angle()} degs, then \
                MOVE: {move_command.get_cells_to_advance()} cells forwards'

        return AgentOutput(
            move_command,
            message
        )

    def update_arena(self, obstacles_coord_list: list, no_obs_coord_list: list) -> None:
        self.mark_robot_visited_cells(self.robot_info.get_coord())
        
        for coord, distance in no_obs_coord_list:
            # mark seen clear cells as explored
            self.arena.get_cell_at_coord(coord).set_is_explored(True).decrement_is_obstacle(delta=4-(distance-2))
        for coord, distance in obstacles_coord_list:
            # mark seen obstacles as explored
            cell = self.arena.get_cell_at_coord(coord)
            cell.increment_is_obstacle(delta=4-(distance-2)).set_is_explored(True)
        self.arena.update_dangerous_cells()

    def think(self) -> Coord:
        cur_coord = self.robot_info.get_coord()
        if self.task == AgentTask.FAST:
            waypoint = None if self.reached_waypoint else self.waypoint_coord
            next_step = self.algo.get_next_step(self.arena, self.robot_info, self.end_coord, waypoint)

        elif self.task == AgentTask.EXPLORE:
            if cur_coord.is_equal(self.LWH_start_coord) and not self.lwh_back_to_start: # first time robot has passed the start coord
                self.lwh_back_to_start = True
            elif cur_coord.is_equal(self.LWH_start_coord) and self.lwh_back_to_start:
                self.commencing_exploration = True

            if self.commencing_exploration:
                next_step = ExplorationAlgo().get_next_step(self.arena, self.robot_info)
            else:
                next_step = self.algo.get_next_step(self.arena, self.robot_info)
                
        else:
            if cur_coord.is_equal(self.RWH_start_coord) and not self.image_rec_back_to_start: # first time robot has passed the start coord
                self.image_rec_back_to_start = True
            elif cur_coord.is_equal(self.RWH_start_coord) and self.image_rec_back_to_start: # when the image_rec_back_to_start flag has been set as True, it means if the coords are equal again, this is the second time it is passing the coords
                self.image_rec_going_to_next_obstacle = True
                self.RWH_start_coord, self.target_obstacle = self.arena.get_nearest_obstacle_adj_coord(cur_coord)
                self.image_rec_back_to_start = False # once you reset the start coord, need to set the flag back to false

            if self.RWH_start_coord == None:
                return None

            if self.image_rec_going_to_next_obstacle: # need this because image_rec_back_to_start doesn't tell you if we are currently running RWH or going to nearest obstacle using FP
                next_step = FastestPathAlgo().get_next_step(self.arena,self.robot_info, self.RWH_start_coord)
                if self.robot_info.get_coord().is_equal(self.RWH_start_coord): # once you have reached the new RWH_start_coord, start running RWH instead of FP
                    self.image_rec_going_to_next_obstacle = False
                    return None
            else:
                next_step = self.algo.get_next_step(self.arena, self.robot_info)

        return next_step

    def calculate_move(self, current_coord, target_coord) -> MoveCommand:
        displacement = target_coord.subtract(current_coord)
        target_orientation = OT.displacement_to_orientation(displacement)
        turn_angle = OT.calc_degree_of_turn(self.robot_info.get_orientation(), target_orientation)
            
        # update expected robot info
        self.expected_robot_info = RobotInfo(target_coord, target_orientation)

        return MoveCommand(
            turn_angle,
            displacement.manhattan_distance()
        )

    def fill_remaining_unexplored_with_obstacles(self):
        # rationale, these unexplored cells are dangerous and are trapped by dangerous cells
        # they are thus probably obstacles
        for cell in self.arena.list_unexplored_cells():
            cell.set_is_explored(True)
            # cell.increment_is_obstacle()
            cell.set_is_dangerous(True)

    def mark_robot_visited_cells(self,center_value):
        adj = self.arena.get_eight_adjacent_in_arena(center_value)
        adj.append(center_value)
        for cd in adj:
            self.arena.get_cell_at_coord(cd).set_is_visited(True)
            self.arena.get_cell_at_coord(cd).set_is_explored(True)

    def get_arena(self) -> Arena:
        return self.arena

    def get_robot_info(self) -> RobotInfo:
        return self.robot_info

    def get_agent_output_list(self) -> list:
        # Calculate fastest path of coords
        coords_list = []        
        cl_to_waypoint = self.algo.get_next_step(self.arena, self.robot_info, self.end_coord, self.waypoint_coord, full_path=True)
        last_displacement = cl_to_waypoint[-1].subtract(cl_to_waypoint[-2])
        last_orientation = OT.displacement_to_orientation(last_displacement)
        robot_info_at_waypoint = RobotInfo(cl_to_waypoint[-1], last_orientation)
        cl_to_end = self.algo.get_next_step(self.arena, robot_info_at_waypoint, self.end_coord, None, full_path=True)
        coords_list.extend(cl_to_waypoint)
        coords_list.extend(cl_to_end)

        return self.calc_agent_output_full_path(coords_list)

    def calc_agent_output_full_path(self, coords_list: list):
        
        # Calculate output list
        agent_output_list = []
        for target_coord in coords_list:
            if target_coord == None:
                message = f'No valid path!'
                move_command = None
            elif self.task == AgentTask.FAST and self.robot_info.get_coord().is_equal(self.end_coord):
                message = f'Fastest path complete!'
                move_command = None
            else:
                robot_info = self.expected_robot_info if self.expected_robot_info else self.robot_info # no eri at first step
                move_command = self.calculate_move(robot_info.get_coord(), target_coord)
                message = f'Target: {target_coord.get_x()}, {target_coord.get_y()}, TURN: {move_command.get_turn_angle()} degs, then \
                MOVE: {move_command.get_cells_to_advance()} cells forwards'
            
            agent_output_list.append(AgentOutput(move_command, message))
        
        # merge move_commands in same direction
        shortened_agent_output_list = []
        if len(agent_output_list) >= 2:
            i, j = 0, 1
            while i < len(agent_output_list)-2:
                cur = agent_output_list[i]
                cur_cmd = cur.get_move_command()
                for j in range(i, len(agent_output_list)-1):
                    print('i ', i)
                    next_cmd = agent_output_list[j].get_move_command()
                    if not next_cmd:
                        break
                    if next_cmd.get_turn_angle() == 0:
                        # if same direction, merge move commands
                        cur_cmd.set_cells_to_advance(cur_cmd.get_cells_to_advance() + next_cmd.get_cells_to_advance())
                        cur.set_message(agent_output_list[j].get_message())
                        print(cur_cmd.get_turn_angle(), cur_cmd.get_cells_to_advance())
                    else:
                        # if not same direction, append move command
                        shortened_agent_output_list.append(cur)
                        print('appended cur', cur_cmd.get_turn_angle(), cur_cmd.get_cells_to_advance())
                        i = j + 1
                        break
                i += 1
            shortened_agent_output_list.append(cur)
            print('appended cur', cur_cmd.get_turn_angle(), cur_cmd.get_cells_to_advance())

        return shortened_agent_output_list

    def get_expected_robot_info(self) -> RobotInfo:
        return self.expected_robot_info
