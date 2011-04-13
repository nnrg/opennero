# Dimensions of the arena
XDIM = 800
YDIM = 800

# Height of the wall
HEIGHT = 20
OFFSET = -HEIGHT/2

MAX_MOVEMENT_SPEED = 12
MAX_VISION_RADIUS = 300

# Population size
pop_size = 20

# number of steps per lifetime
lifetime = 1000

OBJECT_TYPE_OBSTACLE  = (1 << 0) # object type for walls
OBJECT_TYPE_TEAM_0 = (1 << 1) # object type for team 1
OBJECT_TYPE_TEAM_1 = (1 << 2) # object type for team 2 and turrets during training
OBJECT_TYPE_FLAG = (1 << 3) # object type for the flag
OBJECT_TYPE_LEVEL_GEOM = 0 # object type for the level geometry

############################
### SENSOR CONFIGURATION ###
############################

NERO_SENSOR_CONFIG = """
node 1 1 1 1 FriendRadarSensor 3 90 -90 15 0
node 2 1 1 1 FriendRadarSensor 3 90 -90 15 1
node 3 1 1 1 EnemyRadarSensor 2 180 102 90 -90 300
node 4 1 1 1 EnemyRadarSensor 1 108 87 90 -90 300
node 5 1 1 1 EnemyRadarSensor 1 93 72 90 -90 300
node 6 1 1 1 EnemyRadarSensor 2 78 0 90 -90 300 
node 7 1 1 1 EnemyRadarSensor 5 3 177 90 -90 300
node 8 1 1 1 TargettingRaySensor 1 90 0 30
node 9 1 1 1 RaySensor 1 16777216 16777220 180 0 50
node 10 1 1 1 RaySensor 1 16777216 16777220 135 0 50
node 11 1 1 1 RaySensor 1 16777216 16777220 90 0 50
node 12 1 1 1 RaySensor 1 16777216 16777220 45 0 50
node 13 1 1 1 RaySensor 1 16777216 16777220 0 0 50
node 14 1 1 1 FlagRadarSensor 2 180 102 90 -90 300
node 15 1 1 1 FlagRadarSensor 1 108 87 90 -90 300
node 16 1 1 1 FlagRadarSensor 1 93 72 90 -90 300
node 17 1 1 1 FlagRadarSensor 2 78 0 90 -90 300
node 18 1 1 1 FlagRadarSensor 5 3 177 90 -90 300
node 19 1 1 1 EnemyLineOfFireSensor 1 180 0 92 45 65 90 -90 65
node 20 1 1 1 EnemyLineOfFireSensor 1 180 0 135 88 65 90 -90 65
"""

# FriendRadarSensor
FRIEND_RADAR_SENSORS = [0, 1]

# EnemyRadarSensor
ENEMY_RADAR_SENSORS = [(90,12),(18,-3),(3,-18),(-12,-90),(-87,87)]

# Wall Ray Sensors
WALL_RAY_SENSORS = [90,45,0,-45,-90]

# Flag Radar Sensors
FLAG_RADAR_SENSORS = [(90,12),(18,-3),(3,-18),(-12,-90),(-87,87)]

# Number of sensors
N_SENSORS = len(FRIEND_RADAR_SENSORS) + len(FLAG_RADAR_SENSORS) + len(ENEMY_RADAR_SENSORS) + len(WALL_RAY_SENSORS) + 1

# Number of actions
N_ACTIONS = 2

# Network bias value
NEAT_BIAS = 0.3

#############################
### FITNESS CONFIGURATION ###
#############################

FITNESS_STAND_GROUND = "Stand ground"
FITNESS_STICK_TOGETHER = "Stick together"
FITNESS_APPROACH_ENEMY = "Approach enemy"
FITNESS_APPROACH_FLAG = "Approach flag"
FITNESS_HIT_TARGET = "Hit target"
FITNESS_AVOID_FIRE = "Avoid fire"
FITNESS_DIMENSIONS = [FITNESS_STAND_GROUND, FITNESS_STICK_TOGETHER, 
    FITNESS_APPROACH_ENEMY, FITNESS_APPROACH_FLAG, FITNESS_HIT_TARGET, 
    FITNESS_AVOID_FIRE]

FITNESS_INDEX = dict([(f,i) for i,f in enumerate(FITNESS_DIMENSIONS)])

import itertools
DISPLAY_HINTS = itertools.cycle([None, 'fitness', 'time alive', 'hit points', 'genome id', 'species id', 'champion', 'rank'])
DISPLAY_HINT = DISPLAY_HINTS.next()

def getDisplayHint():
    return DISPLAY_HINT

def nextDisplayHint():
    global DISPLAY_HINT
    DISPLAY_HINT = DISPLAY_HINTS.next()
    return DISPLAY_HINT
