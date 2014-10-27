import math
import itertools

# Dimensions of the arena
XDIM = 800
YDIM = 800

# Height and width of walls
WIDTH = 4
HEIGHT = 20
OFFSET = -HEIGHT / 2

# maximum movement speed in world units
MAX_MOVEMENT_SPEED = 1

# maximum turning rate in radians
MAX_TURNING_RATE = 0.2

# maximum vision radius for most sensors
MAX_VISION_RADIUS = 300

# radius for target sensor 
TARGET_SENSOR_RADIUS = 30

# radius for wall sensor 
WALL_SENSOR_RADIUS = 50

# maximum shot radius
#MAX_SHOT_RADIUS = 600
MAX_SHOT_RADIUS = 200
MAX_FIRE_ACTION_RADIUS = 300 

# maximum distance to center of friend group
MAX_FRIEND_DISTANCE = 15.0

# rate of animation at full speed (in frames per second)
ANIMATION_RATE = 100

# Population size
pop_size = 50

# number of steps per lifetime
DEFAULT_LIFETIME = 1000
DEFAULT_HITPOINTS = 20

# default value of explore/exploit slider (x out of 100)
DEFAULT_EE = 10

# default speedup setting (there will be a 0.1 * (100-DEFAULT_SPEEDUP) / 100 second delay between AI steps)
DEFAULT_SPEEDUP = 100

OBJECT_TYPE_OBSTACLE  = (1 << 0) # object type for walls
OBJECT_TYPE_TEAM_0 = (1 << 1) # object type for team 1
OBJECT_TYPE_TEAM_1 = (1 << 2) # object type for team 2 and turrets during training
OBJECT_TYPE_FLAG = (1 << 3) # object type for the flag
OBJECT_TYPE_LEVEL_GEOM = 0 # object type for the level geometry

TEAMS = (OBJECT_TYPE_TEAM_0, OBJECT_TYPE_TEAM_1)
TEAM_LABELS = {OBJECT_TYPE_TEAM_0: 'blue', OBJECT_TYPE_TEAM_1: 'red'}

############################
### SENSOR CONFIGURATION ###
############################

N_SENSORS = 0 # start at 0, add each sensor bank's length

# EnemyRadarSensor
ENEMY_RADAR_SENSORS = [(90, 12), (18, -3), (3, -18), (-12, -90), (-87, 87)]
SENSOR_INDEX_ENEMY_RADAR = [i for i in range(N_SENSORS, N_SENSORS+len(ENEMY_RADAR_SENSORS))]
N_SENSORS += len(ENEMY_RADAR_SENSORS)

# Wall Ray Sensors
WALL_RAY_SENSORS = [90, 45, 0, -45, -90]
SENSOR_INDEX_WALL_RAY = [i for i in range(N_SENSORS, N_SENSORS+len(WALL_RAY_SENSORS))]
N_SENSORS += len(WALL_RAY_SENSORS)

# Flag Radar Sensors
FLAG_RADAR_SENSORS = [(90, 12), (18, -3), (3, -18), (-12, -90), (-87, 87)]
SENSOR_INDEX_FLAG_RADAR = [i for i in range(N_SENSORS, N_SENSORS+len(FLAG_RADAR_SENSORS))]
N_SENSORS += len(FLAG_RADAR_SENSORS)

# 2 friend sensors - distance and angle to friend center of mass
FRIEND_RADAR_SENSORS = [(0,MAX_FRIEND_DISTANCE),(0,360)]
SENSOR_INDEX_FRIEND_RADAR = [i for i in range(N_SENSORS, N_SENSORS+len(FRIEND_RADAR_SENSORS))]
N_SENSORS += len(FRIEND_RADAR_SENSORS)

# 1 targeting sensor - becomes 1 if and only iff the agent is looking at a target
#TARGETING_SENSORS = [(-2,2)]
TARGETING_SENSORS = [0]
SENSOR_INDEX_TARGETING = [i for i in range(N_SENSORS, N_SENSORS+len(TARGETING_SENSORS))]
N_SENSORS += len(TARGETING_SENSORS)

# Number of actions
#N_ACTIONS = 2
N_ACTIONS = 4
ACTION_INDEX_SPEED = 0
ACTION_INDEX_TURN = 1
ACTION_INDEX_FIRE = 2
ACTION_INDEX_ZERO_FRIEND_SENSORS = 3

# Network bias value
NEAT_BIAS = 1.0

MENU_JAR = 'TrainingWindow.jar'
MENU_CLASS = 'edu.utexas.cs.nn.opennero.gui.TrainingUI'

#############################
### FITNESS CONFIGURATION ###
#############################

FITNESS_STAND_GROUND = 'Stand ground'
FITNESS_STICK_TOGETHER = 'Stick together'
FITNESS_APPROACH_ENEMY = 'Approach enemy'
FITNESS_APPROACH_FLAG = 'Approach flag'
FITNESS_HIT_TARGET = 'Hit target'
FITNESS_AVOID_FIRE = 'Avoid fire'
FITNESS_DIMENSIONS = [FITNESS_STAND_GROUND, FITNESS_STICK_TOGETHER,
    FITNESS_APPROACH_ENEMY, FITNESS_APPROACH_FLAG, FITNESS_HIT_TARGET,
    FITNESS_AVOID_FIRE]
FITNESS_NAMES = dict(zip(FITNESS_DIMENSIONS, [
            'STAND_GROUND', 'STICK_TOGETHER',
            'APPROACH_ENEMY', 'APPROACH_FLAG', 'HIT_TARGET',
            'AVOID_FIRE']))

FITNESS_INDEX = dict([(f,i) for i,f in enumerate(FITNESS_DIMENSIONS)])

SQ_DIST_SCALE = math.hypot(XDIM, YDIM) / 2.0
FITNESS_SCALE = {
    FITNESS_STAND_GROUND: SQ_DIST_SCALE,
    FITNESS_STICK_TOGETHER: SQ_DIST_SCALE,
    FITNESS_APPROACH_ENEMY: SQ_DIST_SCALE,
    FITNESS_APPROACH_FLAG: SQ_DIST_SCALE,
    }

DISPLAY_HINTS = itertools.cycle([None, 'fitness', 'time alive', 'champion', 'hit points', 'id', 'species id'])
DISPLAY_HINT = DISPLAY_HINTS.next()

def getDisplayHint():
    return DISPLAY_HINT

def nextDisplayHint():
    global DISPLAY_HINT
    DISPLAY_HINT = DISPLAY_HINTS.next()
    return DISPLAY_HINT
