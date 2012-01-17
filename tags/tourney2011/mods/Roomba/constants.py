# initialize global variables #
STEP_DT = 0.1
AGENT_X = 10
AGENT_Y = 10
MAX_SPEED = 1 # max per-step translational speed
ANGULAR_SPEED = 90 # max angles in degrees agent can turn in one step
TIME_PER_STEP = 0.01 # min time between steps in seconds
STEPS_PER_EPISODE = 100 # max number of steps per episode
MAX_DISTANCE = 1000000 # max possible distance of objects from agent
MIN_DISTANCE = 1 # min distance from object for agent to visit it

ROOMBA_RAD = 6  # Physical Radius of Roomba, for vacuuming and collisions

FIXED_SENSORS = ['wall bump', 'self.x', 'self.y', 'closest.x', 'closest.y']
N_FIXED_SENSORS = len(FIXED_SENSORS)
S_IN_BLOCK = ['x','y','present?','reward']
N_S_IN_BLOCK = len(S_IN_BLOCK)

XDIM = 200.0
YDIM = 200.0
HEIGHT = 20.0
OFFSET = -HEIGHT/2

OBJECT_TYPE_ROOMBA = (1 << 0)
OBJECT_TYPE_WALLS = (1 << 1)
OBJECT_TYPE_FLOOR = (1 << 2)
OBJECT_TYPE_MARKER = (1 << 3)

MOE = 0      # Margin Of Error
WAIT_TIME = 0.1

FURNITURE_LIST = [ 
    (0.880, 0.060),
    (0.962, 0.060),
    (0.962, 0.134),
    (0.710, 0.215),
    (0.878, 0.215),
    (0.968, 0.215),
    (0.710, 0.288),
    (0.878, 0.288),
    (0.968, 0.288),
    (0.885, 0.431),
    (0.885, 0.473),
    (0.964, 0.431),
    (0.964, 0.473),    
    (0.710, 0.624),
    (0.878, 0.624),
    (0.968, 0.624),
    (0.710, 0.695),
    (0.878, 0.695),
    (0.968, 0.695),
    (0.878, 0.850),
    (0.230, 0.730),
    (0.300, 0.730),
    (0.240, 0.450),
    (0.240, 0.373),
    (0.286, 0.595),
    (0.286, 0.373),
    (0.353, 0.595),
    (0.353, 0.373),
    (0.081, 0.900),
    (0.456, 0.900),
    (0.500, 0.900),
    (0.720, 0.900)]
    
CHAIR_LIST = [
    (0.794, 0.110),
    (0.789, 0.407),
    (0.788, 0.882),
    (0.470, 0.769)]
