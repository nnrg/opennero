from math import *
from OpenNero import *
from common import *
from constants import *
from Maze.environment import MazeEnvironment, ContMazeEnvironment
from Maze.agent import FirstPersonAgent

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

class MazeMod:
    # initializer
    def __init__(self):
        print 'Creating MazeMod'
        self.epsilon = 0.5
        self.speedup = 0.0
        self.shortcircuit = False
        self.environment = None
        self.agent_id = None # the ID of the agent
        self.wall_ids = [] # walls on the map

    def __del__(self):
        print 'Deleting MazeMod'

    def add_maze(self):
        """Add a randomly generated maze"""
        if self.environment:
            print "ERROR: Environment already created"
            return
        self.set_environment(MazeEnvironment())

    def set_environment(self,env):
        self.environment = env
        self.environment.epsilon = self.epsilon
        self.environment.speedup = self.speedup
        self.environment.shortcircuit = self.shortcircuit
        for id in self.wall_ids: # delete the walls
            removeObject(id)
        del self.wall_ids[:] # clear the ids
        set_environment(env)
        for ((r1, c1), (r2, c2)) in MazeEnvironment.maze.walls:
            (x1,y1) = MazeEnvironment.maze.rc2xy(r1,c1)
            (x2,y2) = MazeEnvironment.maze.rc2xy(r2,c2)
            pos = Vector3f( (x1 + x2) / 2, (y1 + y2) / 2, 2.5 )
            z_rotation = 0
            if r1 != r2:
                z_rotation = 90
            self.wall_ids.append(addObject(WALL_TEMPLATE, pos, Vector3f(0, 0, z_rotation), type=OBSTACLE_MASK))
        # world boundaries
        for i in range(1,COLS+1):
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(GRID_DX/2, i * GRID_DY, 2), Vector3f(0, 0, 90), type=OBSTACLE_MASK ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(i * GRID_DX, GRID_DY/2, 2), Vector3f(0, 0, 0), type=OBSTACLE_MASK ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(i * GRID_DX, COLS * GRID_DY + GRID_DY/2, 2), Vector3f(0, 0, 0), type=OBSTACLE_MASK ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(ROWS * GRID_DX + GRID_DX/2, i * GRID_DY, 2), Vector3f(0, 0, 90), type=OBSTACLE_MASK ))
        # goal (red cube)
        self.wall_ids.append(addObject("data/shapes/cube/RedCube.xml", Vector3f(ROWS * GRID_DX, COLS * GRID_DY, 5), Vector3f(45,45,45)))

    def reset_maze(self):
        """ reset the maze by removing the markers and starting the AI """
        # remove the agent
        if self.agent_id is not None:
            removeObject(self.agent_id)
        self.agent_id = None
        reset_ai()

    def start_dfs(self):
        """ start the depth first search demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment(False))
        else:
            self.environment.loop = False
        self.agent_id = addObject("data/shapes/character/SydneyDFS.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_bfs(self):
        """ start the breadth first search demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment(False))
        else:
            self.environment.loop = False
        self.agent_id = addObject("data/shapes/character/SydneyBFS.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar(self):
        """ start the A* search demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment(False))
        else:
            self.environment.loop = False
        self.agent_id = addObject("data/shapes/character/SydneyAStar.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar2(self):
        """ start the A* search demo with teleporting agents """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment(False))
        else:
            self.environment.loop = False
        self.agent_id = addObject("data/shapes/character/SydneyAStar2.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar3(self):
        """ start the A* search demo with teleporting agents and a front marked by moving agents """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment(False))
        else:
            self.environment.loop = False
        self.agent_id = addObject("data/shapes/character/SydneyAStar3.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_fps(self):
        """ start the FPS navigation demo for the natural language experiment """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'ContMazeEnvironment':
            self.set_environment(ContMazeEnvironment())
        else:
            self.environment.loop = True
        self.agent_id = addObject("data/shapes/character/SydneyFPS.xml", Vector3f(GRID_DX, GRID_DY, 2), type=AGENT_MASK )
        enable_ai()

    def start_random(self):
        """ start the random baseline demo """
        disable_ai()
        self.reset_maze()
        # ensure that we have the environment ready
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        else:
            self.environment.loop = True
        enable_ai()
        self.agent_id = addObject("data/shapes/character/SydneyRandom.xml",Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )

    def start_rtneat(self):
        """ start the rtneat learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        else:
            self.environment.loop = True
        agent_info = get_environment().agent_info

        # Create an rtNEAT object appropriate for the environment
        pop_size = 50
        pop_on_field_size = 10

        # Establish the number of inputs and outputs
        # We use 1 neuron for each continuous value, and N neurons for 1-of-N
        # coding for discrete variables

        # For inputs, the number of neurons depends on the sensor constraints
        n_inputs = count_neurons(agent_info.sensors)

        # For outputs, the number of neurons depends on the action constraints
        n_outputs = count_neurons(agent_info.actions)

        print 'RTNEAT, inputs: %d, outputs: %d' % (n_inputs, n_outputs)

        rbound = FeatureVectorInfo() # rewards
        rbound.add_continuous(-sys.float_info.max, sys.float_info.max)

        # create the rtneat object that will manage the population of agents
        rtneat = RTNEAT("data/ai/neat-params.dat", n_inputs, n_outputs, pop_size, 1.0, rbound)
        set_ai("rtneat",rtneat)
        rtneat.set_weight(0,1) # we only have one reward, so we better count it!
        enable_ai()
        for i in range(0, pop_on_field_size):
            self.agent_map[(0,i)] = addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK)

    def start_sarsa(self):
        """ start the rtneat learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        else:
            self.environment.loop = True
        self.agent_id = addObject("data/shapes/character/SydneySarsa.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        enable_ai()

    def start_qlearning(self):
        """ start the q-learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        else:
            self.environment.loop = True
        self.agent_id = addObject("data/shapes/character/SydneyQLearning.xml", Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
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
        gMod = MazeMod()
    return gMod
