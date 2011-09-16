from math import *
from OpenNero import *
from common import *
from constants import *
from Tower.environment import TowerEnvironment
import Tower.agent

def count_neurons(constraints):
    """
    Count the number of neurons required to represent the given feature vector
    constraints. For continuous values, scale to a single neuron. For discrete
    values, use (max - min) neurons for a 1-of-N encoding.
    """
    n_neurons = 0
    for i in range(len(constraints)):
        if constraints.discrete(i):
            n_neurons += int(constraints.max(i) - constraints.min(i) + 1)
        else:
            n_neurons += 1
    return n_neurons
    
def input_to_neurons(constraints, input):
    """
    Convert to the ANN coding required to represent the given feature vector. 
    For continuous values, scale to a single neuron. For discrete
    values, use (max - min) neurons for a 1-of-N encoding.
    """
    neurons = []
    for i in range(len(constraints)):
        if constraints.discrete(i):
            section_size = int(constraints.max(i) - constraints.min(i) + 1)
            section = [0 for x in range(section_size)]
            index = int(input[i] - constraints.min(i))
            section[index] = 1
            neurons.extend(section)
        else:
            delta = constraints.max(i) - constraints.min(i)
            neurons.append(neurons[neuron_i] - constraints.min(i)) / delta
    assert(len(neurons) == count_neurons(constraints))
    return neurons

def neurons_to_output(constraints, neurons):
    """
    Convert each continuous value from a neuron output to its range, and each 
    discrete value from it's max-of-N output encoding (where N = Max - Min).
    """
    result = constraints.get_instance()
    neuron_i = 0
    for result_i in range(len(constraints)):
        assert(neuron_i < len(neurons))
        assert(neuron_i >= result_i)
        if constraints.discrete(result_i):
            # section of the neurons coding for this output
            section_size = int(constraints.max(result_i) - constraints.min(result_i) + 1)
            section_values = neurons[neuron_i:(neuron_i + section_size)]
            # the maximally activated neuron in this section
            max_neuron = max(section_values)
            max_index = section_values.index(max_neuron)
            # the result output
            result[result_i] = constraints.min(result_i) + max_index
            neuron_i += section_size
        else:
            delta = constraints.max(result_i) - constraints.min(result_i)
            result[result_i] = constraints.min(i) + neurons[neuron_i] * delta
            neuron_i += 1
    return result

class TowerMod:
    # initializer
    def __init__(self):
        print 'Creating TowerMod'
        self.epsilon = 0.5
        self.speedup = 0.0
        self.shortcircuit = False
        self.environment = None
        self.agent_id = None # the ID of the agent
        self.marker_map = {} # a map of cells and markers so that we don't have more than one per cell
        self.marker_states = {} # states of the marker agents that run for one cell and stop
        self.agent_map = {} # agents active on the map
        self.wall_ids = [] # walls on the map

    def __del__(self):
        print 'Deleting TowerMod'

    def mark_maze(self, r, c, marker):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        if (r,c) in self.marker_map:
            removeObject(self.marker_map[(r,c)])
        # remember the ID of the marker
        self.marker_map[(r,c)] = addObject(marker, Vector3f( (r+1) * GRID_DX, (c+1) * GRID_DY, -1))

    def mark_maze_blue(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/BlueCube.xml")

    def mark_maze_green(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/GreenCube.xml")

    def mark_maze_yellow(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/YellowCube.xml")

    def mark_maze_white(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/WhiteCube.xml")

    def unmark_maze_agent(self, r, c):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        if (r,c) in self.agent_map:
            removeObject(self.agent_map[(r,c)])
            del self.marker_states[self.agent_map[(r,c)]]
            del self.agent_map[(r,c)]

    def mark_maze_agent(self, agent, r1, c1, r2, c2):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        self.unmark_maze_agent(r2,c2)
        # add a new marker object
        agent_id = addObject(agent, Vector3f( (r1+1) * GRID_DX, (c1+1) * GRID_DY, 2) )
        self.marker_states[agent_id] = ((r1, c1), (r2, c2))
        self.agent_map[(r2,c2)] = agent_id

    # add a set of coordinate axes
    def addAxes(self):
        getSimContext().addAxes()

    def add_maze(self):
        """Add a randomly generated maze"""
        if self.environment:
            print "ERROR: Environment already created"
            return
        self.set_environment(TowerEnvironment())

    def set_environment(self,env):
        self.environment = env
        self.environment.epsilon = self.epsilon
        self.environment.speedup = self.speedup
        self.environment.shortcircuit = self.shortcircuit
        for id in self.wall_ids: # delete the walls
            removeObject(id)
        del self.wall_ids[:] # clear the ids
        set_environment(env)

        self.block_ids = []

        addObject("data/shapes/cube/WhiteCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 0 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.25,.25,4))
        addObject("data/shapes/cube/WhiteCube.xml", Vector3f(2 * GRID_DX, 2 * GRID_DY, 0 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.25,.25,4))
        addObject("data/shapes/cube/WhiteCube.xml", Vector3f(3 * GRID_DX, 2 * GRID_DY, 0 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.25,.25,4))

    def num_towers(self):
        return num_towers

    def start_tower(self,n):
        """ start the tower demo """
        self.num_towers = n
        disable_ai()
        if self.environment.__class__.__name__ != 'TowerEnvironment':
            self.set_environment(TowerEnvironment())
        if len(self.environment.block_states) == 0:
         self.agent_id = addObject("data/shapes/character/SydneyQLearning.xml", Vector3f(GRID_DX, GRID_DY, 2), type=AGENT_MASK,scale=Vector3f(3,3,3))
        else:
            for obj in self.environment.states: self.environment.reset(obj)
        enable_ai()

    def control_fps(self,key):
        FirstPersonAgent.key_pressed = key

    def set_epsilon(self, epsilon):
        self.epsilon = epsilon
        print 'Epsilon set to', self.epsilon
        if self.environment:
            self.environment.epsilon = epsilon
            
    def set_speedup(self, speedup):
        self.speedup = speedup
        print 'Speedup set to', self.speedup
        # speed up between 0 (delay set to 1 second) and 1 (delay set to 0)
        getSimContext().delay = 1.0 - speedup
    
    def set_shortcircuit(self, shortcircuit):
        self.shortcircuit = shortcircuit
        print 'Short-circuit set to', self.shortcircuit
        if self.environment:
            self.environment.shortcircuit = shortcircuit

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = TowerMod()
    return gMod
