import pygame
from src.simulator.robot_sprite import RobotSprite
from src.dto.constants import *
class SimulationDisplay:
    def __init__(self, robot_info):
        self.dis = pygame.display.set_mode((DIS_X,500))
        self.robot_sprite = RobotSprite(self, robot_info)

    def draw(self, arena, robot_info):
        self.dis.fill(white)
        self.draw_grid(arena)
        self.draw_robot(robot_info)

    def draw_grid(self, arena):
        # Display coloured boxes to indicate obstacles, start and end points
        for row in range(MAP_ROW):
            for col in range(MAP_COL):
                coord = Coord(col, row)
                cell = arena.get_cell_at_coord(coord)
                graph_row = MAP_ROW-row-1 # pygame coordinates are inverted (row 0 starts at the top rather than bottom)
                if cell.is_explored():
                    pygame.draw.rect(self.dis, light_grey, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if cell.is_dangerous() and not cell.is_obstacle():
                    pygame.draw.rect(self.dis, grey, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if cell.is_obstacle():
                    pygame.draw.rect(self.dis, black, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])
                    seen_surfaces = cell.get_seen_surfaces()
                    if seen_surfaces[Orientation.NORTH]:
                        pygame.draw.line(self.dis, light_blue, (col*TILE_SIZE, graph_row*TILE_SIZE), (col*TILE_SIZE + TILE_SIZE, graph_row*TILE_SIZE), width=2)
                    if seen_surfaces[Orientation.WEST]:
                        pygame.draw.line(self.dis, light_blue, (col*TILE_SIZE, graph_row*TILE_SIZE), (col*TILE_SIZE, graph_row*TILE_SIZE + TILE_SIZE), width=2)
                    if seen_surfaces[Orientation.SOUTH]:
                        pygame.draw.line(self.dis, light_blue, (col*TILE_SIZE, graph_row*TILE_SIZE + TILE_SIZE), (col*TILE_SIZE + TILE_SIZE, graph_row*TILE_SIZE + TILE_SIZE), width=2)
                    if seen_surfaces[Orientation.EAST]:
                        pygame.draw.line(self.dis, light_blue, (col*TILE_SIZE + TILE_SIZE, graph_row*TILE_SIZE), (col*TILE_SIZE + TILE_SIZE, graph_row*TILE_SIZE + TILE_SIZE), width=4)

                if not cell.is_explored():
                    pygame.draw.rect(self.dis, white, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 0<=row<=2 and 0<=col<=2:
                    pygame.draw.rect(self.dis, red, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if 17<=row<=19 and 12<=col<=14:
                    pygame.draw.rect(self.dis, green, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

                if row == WAYPOINT.get_x() and col == WAYPOINT.get_y():
                    pygame.draw.rect(self.dis, purple, [col*TILE_SIZE, graph_row*TILE_SIZE, TILE_SIZE, TILE_SIZE])

        # Draw the lines for the grid
        for x in range(0,DIS_X+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (x,0), (x,DIS_Y))
        for y in range(0,DIS_Y+TILE_SIZE,TILE_SIZE):
            pygame.draw.line(self.dis, grey, (0,y), (DIS_X,y))

    def draw_robot(self, robot_info):
        self.robot_sprite.move(robot_info)
        self.robot_sprite.update()

        pygame.draw.circle(self.dis, yellow, (self.robot_sprite.x*20+10, 400-self.robot_sprite.y*20-10), 30)  #start at (30,370)

        #takes in the current pos of the robot and faces the direction based on the orientation
        if self.robot_sprite.orientation == Orientation.NORTH:
            pygame.draw.circle(self.dis, blue, (self.robot_sprite.x*20+10, 400-self.robot_sprite.y*20-10-20), 5)
        elif self.robot_sprite.orientation == Orientation.EAST:
            pygame.draw.circle(self.dis, blue, (self.robot_sprite.x*20+10+20, 400-self.robot_sprite.y*20-10), 5)
        elif self.robot_sprite.orientation == Orientation.SOUTH:
            pygame.draw.circle(self.dis, blue, (self.robot_sprite.x*20+10, 400-self.robot_sprite.y*20-10+20), 5)
        elif self.robot_sprite.orientation == Orientation.WEST:
            pygame.draw.circle(self.dis, blue, (self.robot_sprite.x*20+10-20, 400-self.robot_sprite.y*20-10), 5)
