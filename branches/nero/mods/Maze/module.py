from math import *
from OpenNero import *
from common import *
from constants import *
from Maze.environment import MazeEnvironment, ContMazeEnvironment
from Maze.agent import FirstPersonAgent

class MazeMod:
    # initializer
    def __init__(self):
        print 'Creating MazeMod'
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
        print 'Deleting MazeMod'

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
        for ((r1, c1), (r2, c2)) in env.maze.walls:
            (x1,y1) = env.maze.rc2xy(r1,c1)
            (x2,y2) = env.maze.rc2xy(r2,c2)
            pos = Vector3f( (x1 + x2) / 2, (y1 + y2) / 2, 2.5 )
            z_rotation = 0
            if r1 != r2:
                z_rotation = 90
            self.wall_ids.append(addObject(WALL_TEMPLATE, pos, Vector3f(0, 0, z_rotation), type=1))
        # world boundaries
        for i in range(1,COLS+1):
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(GRID_DX/2, i * GRID_DY, 2), Vector3f(0, 0, 90), type=1 ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(i * GRID_DX, GRID_DY/2, 2), Vector3f(0, 0, 0), type=1 ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(i * GRID_DX, COLS * GRID_DY + GRID_DY/2, 2), Vector3f(0, 0, 0), type=1 ))
            self.wall_ids.append(addObject(WALL_TEMPLATE, Vector3f(ROWS * GRID_DX + GRID_DX/2, i * GRID_DY, 2), Vector3f(0, 0, 90), type=1 ))
        # goal (red cube)
        self.wall_ids.append(addObject("data/shapes/cube/RedCube.xml", Vector3f(ROWS * GRID_DX, COLS * GRID_DY, 5), Vector3f(45,45,45)))

    def reset_maze(self):
        """ reset the maze by removing the markers and starting the AI """
        # remove the marker blocks
        for id in self.marker_map.values():
            removeObject(id)
        self.marker_map = {}
        for id in self.agent_map.values():
            removeObject(id)
        self.agent_map = {}
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
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyDFS.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar(self):
        """ start the A* search demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyAStar.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar2(self):
        """ start the A* search demo with teleporting agents """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyAStar2.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_astar3(self):
        """ start the A* search demo with teleporting agents """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyAStar3.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        getSimContext().setObjectAnimation(self.agent_id, 'run')
        enable_ai()

    def start_fps(self):
        """ start the FPS navigation demo for the natural language experiment """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'ContMazeEnvironment':
            self.set_environment(ContMazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyFPS.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        enable_ai()

    def start_random(self):
        """ start the rtneat learning demo """
        disable_ai()
        self.reset_maze()
        # ensure that we have the environment ready
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        enable_ai()
        self.agent_id = addObject("data/shapes/character/SydneyRandom.xml",Vector3f(GRID_DX, GRID_DY, 2) )

    def start_rtneat(self):
        """ start the rtneat learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        # Create RTNEAT object
        pop_size = 50
        pop_on_field_size = 10
        rtneat = RTNEAT("data/ai/neat-params.dat", 8, 3, pop_size, 1.0)
        set_ai("neat",rtneat)
        enable_ai()
        for i in range(0, pop_on_field_size):
            self.agent_map[(0,i)] = addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(GRID_DX, GRID_DY, 2) )

    def start_sarsa(self):
        """ start the rtneat learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneySarsa.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        enable_ai()

    def start_qlearning(self):
        """ start the q-learning demo """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != 'MazeEnvironment':
            self.set_environment(MazeEnvironment())
        self.agent_id = addObject("data/shapes/character/SydneyQLearning.xml", Vector3f(GRID_DX, GRID_DY, 2) )
        enable_ai()

    def control_fps(self,key):
        FirstPersonAgent.key_pressed = key

    def set_epsilon(self, epsilon):
        self.epsilon = epsilon
        print 'Epsilon set to', self.speedup
        if self.environment:
            self.environment.epsilon = epsilon
            
    def set_speedup(self, speedup):
        self.speedup = speedup
        print 'Speedup set to', self.speedup
        if self.environment:
            self.environment.speedup = speedup
    
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
