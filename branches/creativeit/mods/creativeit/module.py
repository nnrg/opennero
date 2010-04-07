from math import *

from OpenNero import *
from common.server import *
from creativeit import *
from ForageEnvironment import ForageEnvironment
from KeyboardAgent import KeyboardAgent
from RTNEATAgent import RTNEATAgent

class CreativeITServer(Server):
    
    # initializer
    def __init__(self, serverContext):
        Server.__init__(self, serverContext)
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
    
    # every method that expects to be called from a client
    # should be of the required form:
    # def methodName( self, client, param0, param1, ..., paramN )
    #
    # the 'client' param is implicitly passed by the system to
    # allow the server to recognize which client is making the
    # request for this method to be called
    
    # add a set of coordinate axes
    def addAxes(self, client):
        self.context.sim_context.addAxes()

    def control_key(self, client, key):
        KeyboardAgent.keys.add(key) # tell the keyboard agent that a key has been hit this frame

    def addWall(self, client, position, rotation = Vector3f(0,0,0), velocity = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a wall to the simulation """
        self.environment.walls.add(self._getNextFreeId())
        self._addObject("data/shapes/wall/BrickWall.xml", position, rotation, velocity, scale)

    def addCube(self, client, position, rotation = Vector3f(0,0,0), velocity = Vector3f(0,0,0), scale = Vector3f(1,1,1)):
        """ add a cube to the simulation """
        self.environment.cubes.add(self._getNextFreeId())
        self._addObject("data/shapes/cube/WhiteCube.xml", position, rotation, velocity, scale)
    
    def removeObject(self, client, id):
        """ schedule an object to be removed from the simulation """
        self.environment.remove_object(id)
        self._removeObject(id)

    def setAdvice(self, client, advice):
        """ advice for rtneat agents """
        if self.rtneat != None:
            self.rtneat.advice = Advice(advice, self.rtneat, self.num_sensors, self.num_outputs, True)
        elif self.scripted != None:
            self.scripted.add_advice(advice)
        for state in self.environment.states.values():
            state.has_new_advice = True

    def setObjectPath(self, client, path, id):
        """ set the path of the specified object """
        self.environment.object_paths[id] = path

    def start_tracing(self, client):
        """ start tracing the agents """
        self.environment.start_tracing()

    def stop_tracing(self, client):
        """ stop tracing the agents """
        self.environment.stop_tracing()

    def enable_simdisplay(self, client):
        """ start displaying the simulation of agents """
        self.environment.simdisplay = True

    def disable_simdisplay(self, client):
        """ stop displaying the simulation of agents """
        self.environment.simdisplay = False

    def save_trace(self, client, filename):
        """ save the current trace to the given filename """
        self.environment.save_trace(filename)

    def load_trace(self, client, filename):
        """ load previously saved trace file """
        self.environment.load_trace(filename)

    def unload_trace(self, client):
        """ unload previously loaded trace """
        self.environment.unload_trace()

    def enable_backprop(self, client):
        """ run backprop if trace has been loaded """
        print "enabling backprop ..."
        self.environment.run_backprop = True

    def disable_backprop(self, client):
        """ stop running backprop """
        print "disabling backprop ..."
        self.environment.run_backprop = False

    def start_rtneat(self, client):
        """ start rtneat to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        RTNEATAgent.INDEX_COUNT = 0  # Initialize variable for assigning indexes to agents
        self.rtneat = RTNEAT("data/ai/neat-params.dat", self.num_sensors, self.num_outputs, self.pop_size, 1.0)
        set_ai("neat", self.rtneat)
        enable_ai()
        for i in range(self.pop_size):
            self.agents[i] = self._getNextFreeId()
            #self._addObject("data/shapes/character/CubeRTNEAT.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
            self._addObject("data/shapes/character/SydneyRTNEAT.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))

    
    def start_scripted(self, client):
        """ start scripted agent to find a solution """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.scripted = Scripted(self.num_sensors, self.num_outputs)
        self.agents[0] = self._getNextFreeId()
        #self._addObject("data/shapes/character/CubeScripted.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        self._addObject("data/shapes/character/SydneyScripted.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()


    def start_keyboard(self,client):
        """ start the keyboard agent to teach by demonstration """
        disable_ai()
        set_environment(self.environment)
        self.environment.initialize()
        self.agents[0] = self._getNextFreeId()
        #self._addObject("data/shapes/character/CubeKeyboard.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        self._addObject("data/shapes/character/SydneyKeyboard.xml", Vector3f(900, -60, 2), Vector3f(0,0,0), Vector3f(0,0,0), Vector3f(0.5,0.5,0.5))
        enable_ai()

    
def ServerMain():
    # replace the default server object with our custom server
    RegisterServer(CreativeITServer(ServerContext()))
