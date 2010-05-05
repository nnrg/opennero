from math import *

from OpenNero import *
from common import *
from creativeit import *
from ForageEnvironment import ForageEnvironment
from KeyboardAgent import KeyboardAgent
from RTNEATAgent import RTNEATAgent

class CreativeITMod:
    
    # initializer
    def __init__(self):
        self.object_ids = set()         # Ids of objects added to the world by the user
        self.object_z = 2               # Z cooridinate of objects added by the user
        self.modify_object_id = dict()  # Id of object whose pos/rot/scale/color is to be modified            
        self.scale_const = 10           # Constant used for scaling objects by moving mouse
        self.color_value_max = 100      # Maximum of color values associated with objects
        self.color_const = 4            # Constant used for adjusting color of objects by moving mouse
        self.tracing = False            # Flag indicating if agent is being traced
        self.agents = {}  # ids of agents
        self.key_object_id = dict()     # Key-id mapping of objects added using keyboard
        self.key_orig_props = dict()    # Original properties of objects added using keyboard
        self.saved_cursor = None        # Saved mouse position used for adjusting radius and speed
        self.radius_const = 0.01        # Constant used for adjusting radius of circle using mouse
        self.speed_const = 0.01         # Constant used for adjusting speed of moving cube using mouse
        self.current_radius = 1.0       # Current radius in terms of fraction of max value
        self.current_speed = 1.0        # Current speed in terms of fraction of max value
        self.circle_function = None     # Function describing the circular path
        self.control = None             # The selected control method for agent
        self.task = None                # The selected task
        self.approach = None            # The selected approach
        # egocentric sensor angles are specified in the range [-1, 1]; multiplying this number
        # by 180 gives the angle in degrees.  cube sensors are evenly distributed around the
        # agent, while wall sensors are in front of the agent.
        self.cube_sensor_angles = [-0.875, -0.625, -0.375, -0.125, 0.125, 0.375, 0.625, 0.875]
        self.wall_sensor_angles = [-0.125, -0.0417, 0.0417, 0.125]
        self.wall_sensor_hbases = [0.0417, 0.0417, 0.0417, 0.0417]  # half base of the activation triangles
        self.num_sensors = len(self.cube_sensor_angles) + len(self.wall_sensor_angles) + 1  # last input is bias
        self.num_actions = 2

        # Bounds for sensors in neural network and advice language. These bounds are
        # used to convert sensor values between network and advice.
        self.sbounds_network = FeatureVectorInfo()
        self.sbounds_advice = FeatureVectorInfo()

        # Networks have better learning bias when cube sensors produce values in the
        # range [-1, 1], but the range [0, 1] is more intuitive in the advice
        # language.  Wall sensors use the same range [0, 1] for both network and advice.
        # The sense() method in the ForageEnvironment class use these network bounds
        # to scale the sensor values.
        for i in range(len(self.cube_sensor_angles)):
            self.sbounds_network.add_continuous(-1, 1)
            self.sbounds_advice.add_continuous(0, 1)
        for i in range(len(self.wall_sensor_angles)):
            self.sbounds_network.add_continuous(0, 1)
            self.sbounds_advice.add_continuous(0, 1)

        # The last sensor is the bias, which always takes the value 1 (upper bound).
        self.sbounds_network.add_continuous(0, 1)
        self.sbounds_advice.add_continuous(0, 1)

        self.agents = {}  # ids of agents
        self.pop_size = 20
        self.environment = ForageEnvironment(self)
        self.rtneat = None
        self.scripted = None
        
        # Mark the position where the agents start.
        self.start_pos = Vector3f(900, -60, 2)
        addObject("data/shapes/cube/RedCube.xml", self.start_pos, Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))        
    
    # add a set of coordinate axes
    def addAxes(self):
        getSimContext().addAxes()

    def control_key(self, key):
        KeyboardAgent.keys.add(key) # tell the keyboard agent that a key has been hit this frame

    def addWall(self, position, rotation = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a wall to the simulation """
        print 'Adding wall position:', position, ' rotation:', rotation, ' scale: ', scale
        id = addObject("data/shapes/wall/BrickWall.xml", position, rotation, scale)
        self.object_ids.add(id)
        self.environment.walls.add(id)
        return id

    def addCube(self, position, rotation = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a cube to the simulation """
        id = addObject("data/shapes/cube/WhiteCube.xml", position, rotation, scale)
        self.object_ids.add(id)
        self.environment.cubes.add(id)
        return id
    
    def removeObject(self, id):
        """ schedule an object to be removed from the simulation """
        self.object_ids.remove(id)
        self.environment.remove_object(id)
        removeObject(id)

    def setAdvice(self, advice):
        """ advice for rtneat agents """
        if self.rtneat != None:
            self.rtneat.advice = Advice(advice, self.rtneat, self.num_sensors, self.num_actions, True, self.sbounds_network, self.sbounds_advice)
        elif self.scripted != None:
            self.scripted.add_advice(advice)
        for state in self.environment.states.values():
            state.has_new_advice = True

    def setObjectPath(self, id, path):
        """ set the path of the specified object """
        self.environment.object_paths[id] = path

    def enable_simdisplay(self):
        """ start displaying the simulation of agents """
        self.environment.simdisplay = True

    def disable_simdisplay(self):
        """ stop displaying the simulation of agents """
        self.environment.simdisplay = False

    def save_trace(self, filename):
        """ save the current trace to the given filename """
        self.environment.save_trace(filename)

    def load_trace(self, filename):
        """ load previously saved trace file """
        self.environment.load_trace(filename)

    def unload_trace(self):
        """ unload previously loaded trace """
        self.environment.unload_trace()

    def enable_backprop(self):
        """ run backprop if trace has been loaded """
        print "enabling backprop ..."
        self.environment.run_backprop = True

    def disable_backprop(self):
        """ stop running backprop """
        print "disabling backprop ..."
        self.environment.run_backprop = False

    def start_rtneat(self, task):
        """ start rtneat to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        RTNEATAgent.INDEX_COUNT = 0  # Initialize variable for assigning indexes to agents
        if task == 'around':
            self.rtneat = RTNEAT('data/ai/cube_population.txt', "data/ai/neat-params.dat", self.pop_size)
        else:
            self.rtneat = RTNEAT("data/ai/neat-params.dat", self.num_sensors, self.num_actions, self.pop_size, 1.0)
        set_ai("neat", self.rtneat)
        enable_ai()
        for i in range(self.pop_size):
            self.agents[i] = addObject("data/shapes/character/SydneyRTNEAT.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))

    def start_scripted(self):
        """ start scripted agent to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.scripted = Scripted(self.num_sensors, self.num_actions)
        self.agents[0] = addObject("data/shapes/character/SydneyScripted.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()

    def start_keyboard(self):
        """ start the keyboard agent to collect demonstration example """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.agents[0] = addObject("data/shapes/character/SydneyKeyboard.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()
        self.environment.start_tracing()

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = CreativeITMod()
    return gMod