from map import Grid
import pygame

pygame.init()

blue=(0,0,255)
white = [255, 255, 255]
black = [0,0,0]
grey = [100,100,100]
# TODO standardise this across all files
map_row = 20 
map_col = 15

dis=pygame.display.set_mode((300,500))
dis.fill(white)
pygame.display.set_caption('arena simulator')

game_over=False
while not game_over:
    for event in pygame.event.get():
        if event.type==pygame.QUIT:
            game_over=True

    # Display coloured boxes to indicate obstacles, start and end points
    for row in range(map_row):
        for col in range(map_col):
            if Grid.grid[row][col].getObstacle() == True:
                pygame.draw.rect(dis, black, [col*20, row*20, 20, 20])

    # Draw the lines for the grid
    for x in range(0,320,20):
        pygame.draw.line(dis, grey, (x,0), (x,400))
    for y in range(0,420,20):
        pygame.draw.line(dis, grey, (0,y), (400,y))

    pygame.display.update()


pygame.quit()
quit()