import sys, os, re, json
path_of_directory_head = os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))))
sys.path.append(path_of_directory_head)
import pygame
from src.simulator.SimulationDisplay import SimulationDisplay
from src.dto.constants import Orientation
from src.dto.RobotInfo import RobotInfo
from src.dto.coord import Coord
from src.dto.ArenaStringParser import ArenaStringParser

if len(sys.argv) < 2:
    sys.exit(f'Usage: {sys.argv[0]} [path to log file]')

log_fname = sys.argv[1]

pygame.init()
pygame.display.set_caption('Arena Simulator')

robot_info = RobotInfo(Coord(1, 1), Orientation.NORTH)
sim_display = SimulationDisplay(robot_info)

with open(log_fname, "r") as f:
    while True:
        
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RIGHT:
                    while True:
                        line = f.readline()

                        if not line:
                            break

                        if line:
                            json_obj = re.search('{.+}', line)
                            if '"type":"status"' in line:
                                data = json.loads(json_obj[0])
                                print(data)

                                _robot_info = data["data"]["robot_info"]
                                internal_arena = data["data"]["internal_arena"]

                                robot_info.get_coord().set_x(_robot_info["x"])
                                robot_info.get_coord().set_y(_robot_info["y"])
                                robot_info.set_orientation(Orientation[_robot_info["orientation"]])

                                sim_display.draw(ArenaStringParser.parse_p1_p2(internal_arena["P1"], internal_arena["P2"]), robot_info)
                                pygame.display.update()
                                break
