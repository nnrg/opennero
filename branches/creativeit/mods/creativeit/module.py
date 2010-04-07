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
        # egocentric sensor angles are specified in the range [-1, 1]; multiplying this number
        # by 180 gives the angle in degrees.  cube sensors are evenly distributed around the
        # agent, while wall sensors are in front of the agent.
        self.cube_sensor_angles = [-0.125, -0.375, -0.625, -0.875, 0.125, 0.375, 0.625, 0.875]
        self.wall_sensor_angles = [-0.10, -0.06, 0.0, 0.06, 0.10]
        self.wall_sensor_hbases = [0.10, 0.06, 0.02, 0.06, 0.10]  # half base of the activation triangles
        self.num_sensors = len(self.cube_sensor_angles) + len(self.wall_sensor_angles) + 1  # last input is bias
        self.num_outputs = 2
        self.pop_size = 20
        self.environment = ForageEnvironment(self)
        self.rtneat = None
        self.scripted = None
    
    # add a set of coordinate axes
    def addAxes(self):
        getSimContext().addAxes()

    def control_key(self, key):
        KeyboardAgent.keys.add(key) # tell the keyboard agent that a key has been hit this frame

    def addWall(self, position, rotation = Vector3f(0,0,0), velocity = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a wall to the simulation """
        self.environment.walls.add(addObject("data/shapes/wall/BrickWall.xml", position, rotation, velocity, scale))

    def addCube(self, position, rotation = Vector3f(0,0,0), velocity = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a cube to the simulation """
        self.environment.cubes.add(addObject("data/shapes/cube/WhiteCube.xml", position, rotation, velocity, scale))
    
    def removeObject(self, id):
        """ schedule an object to be removed from the simulation """
        self.environment.remove_object(id)
        self._removeObject(id)

    def setAdvice(self, advice):
        """ advice for rtneat agents """
        if self.rtneat != None:
            self.rtneat.advice = Advice(advice, self.rtneat, self.num_sensors, self.num_outputs, True)
        elif self.scripted != None:
            self.scripted.add_advice(advice)
        for state in self.environment.states.values():
            state.has_new_advice = True

    def setObjectPath(self, path, id):
        """ set the path of the specified object """
        self.environment.object_paths[id] = path

    def start_tracing(self):
        """ start tracing the agents """
        self.environment.start_tracing()

    def stop_tracing(self):
        """ stop tracing the agents """
        self.environment.stop_tracing()

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

    def start_rtneat(self):
        """ start rtneat to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        RTNEATAgent.INDEX_COUNT = 0  # Initialize variable for assigning indexes to agents
        self.rtneat = RTNEAT("data/ai/neat-params.dat", self.num_sensors, self.num_outputs, self.pop_size, 1.0)
        set_ai("neat", self.rtneat)
        enable_ai()
        for i in range(self.pop_size):
            self.agents[i] = addObject("data/shapes/character/SydneyRTNEAT.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))

    
    def start_scripted(self):
        """ start scripted agent to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.scripted = Scripted(self.num_sensors, self.num_outputs)
        self.agents[0] = addObject("data/shapes/character/SydneyScripted.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()


    def start_keyboard(self):
        """ start the keyboard agent to teach by demonstration """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.agents[0] = addObject("data/shapes/character/SydneyKeyboard.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = CreativeITMod()
    return gMod