import math
import random
MOE = 0

def go_xy(sensors, target_x, target_y):
    if (abs(round(sensors[1] - target_x)) > MOE):
        return go_x(sensors, target_x)
    elif (abs(round(sensors[2] - target_y)) > MOE):
        return go_y(sensors, target_y)
    else:
        return random.random() * math.pi
        
def go_x(sensors, target):
    if (sensors[1] > target):
        orient = math.pi
    elif (sensors[1] < target):
        orient = 0
    return orient

def go_y(sensors, target):
    if (sensors[2] >= target):
        orient = -math.pi/2
    elif (sensors[2] < target):
        orient = math.pi/2
    return orient
