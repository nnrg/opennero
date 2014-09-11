import math

MODE_SEARCH = "Search"
MODE_RL = "RL"

ROWS = 8 # rows in maze
COLS = 8 # columns in maze
GRID_DX = 20.0 # x-dimension of the grid world
GRID_DY = 20.0 # y-dimension of the grid world
MAX_STEPS = ROWS * COLS * 6 # max number of steps - no need to visit each cell more then twice! (turns count now!)
STEP_DELAY = 3.0 # number of seconds to wait between the sense-act-step repeats
NUDGE_X = 20.0 # shift the island in +x by ...
NUDGE_Y = 20.0 # shift the island in +y by ...
WALL_TEMPLATE = "data/shapes/wall/BrickWall.xml"
INITIAL_EPSILON = 0.1
INITIAL_DIST = 0
HISTORY_LENGTH = 5 # number of state-action pairs used to determine if the agent is stuck
OBSTACLE_MASK = 1 #0b0001
AGENT_MASK = 2 #0b0010

# maze environment
MAZE_MOVES = [(1,0), (-1,0), (0,1), (0,-1)]
MAZE_NULL_MOVE = len(MAZE_MOVES)

# continuous environment
CONT_MAZE_TURN_BY = 90 # how many degrees to turn by every time
CONT_MAZE_WALK_BY = GRID_DX # how many units to advance by every step forward
CONT_MAZE_ACTIONS = {'FWD':0, 'CW':3, 'CCW':2, 'BCK':1} # in Granular, FWD is N, BCK is S, CW is E and CCW is W
CONT_MAZE_N_ACTIONS = 4 # number of actions
CONT_MAZE_N_RAYS = 4 # number of rays around the agent, starting from the front
CONT_MAZE_MAX_DISTANCE = math.hypot(ROWS*GRID_DX, COLS*GRID_DY) # max distance within the maze
