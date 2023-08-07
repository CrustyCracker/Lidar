from include.constants import *
import pygame
from math import sin, cos, radians
LINE_COLOR = BLUE


class Map():
    def __init__(self, width, height, is_mirrored=False):
        self.data = []
        
        self.width = width
        self.height = height
        if is_mirrored:
            self.mirror = -1
        else:
            self.mirror = 1

    def draw(self, win, line_data):
        self.draw_circle(win)
        self.draw_line(win, line_data)
        
    def draw_line(self, win,  line_data):
        
        point_cords = self.get_line_point(line_data)
        distance, alpha = line_data
        self.get_line_color(distance)

        pygame.draw.line(win, self.get_line_color(distance), CIRCLE_CENTER, point_cords, width = LINE_WIDTH)
        pygame.draw.circle(win, self.get_line_color(distance, 180), point_cords, 3, width = 0 )

    def get_line_point(self, line_data):
        #TODO rescale x
        distance, alpha = line_data
        alpha = radians(alpha)

        length = (distance * HEIGHT)/MAX_LINE_LENGTH
        length = min(length, HEIGHT)

        center_x, center_y = CIRCLE_CENTER
        line_point = (center_x + self.mirror * length * cos(alpha), center_y - (length * sin(alpha)))
        return line_point

    def get_line_color(self, distance, fiflak = 255):

        G = (distance * 255)/MAX_LINE_LENGTH
        G = min(G, 255)

        return (fiflak, G, 0)
        

    def draw_circle(self, win):
        pygame.draw.circle(win, BLACK, CIRCLE_CENTER, CIRCLE_RADIOUS, width = 3, draw_top_right = True, draw_top_left = True)

    

    
    