MODE_PLANNING = "Planning"
MODE_NLP = "NLP"

ROWS = 8 # rows in maze
COLS = 8 # columns in maze
GRID_DX = 20.0#20.0 # x-dimension of the grid world
GRID_DY = 10.0#20.0 # y-dimension of the grid world
GRID_DZ = 3.0 * .75 * .5# z-dimension of the grid world
MAX_STEPS = ROWS * COLS * 2  * 90# max number of steps - no need to visit each cell more then twice!
STEP_DELAY = 3.0 # number of seconds to wait between the sense-act-step repeats
NUDGE_X = -5.0 # shift the island in +x by ...
NUDGE_Y = -5.0 # shift the island in +y by ...
WALL_TEMPLATE = "data/shapes/wall/BrickWall.xml"
HISTORY_LENGTH = 5 # number of state-action pairs used to determine if the agent is stuck
OBSTACLE_MASK = 1 #0b0001
AGENT_MASK = 2 #0b0010

