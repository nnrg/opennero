from math import *
from OpenNero import *
from common import *
from constants import *
from mazer import Maze
from Maze.environment import MazeEnvironment, EgocentricMazeEnvironment
from Maze.agent import FirstPersonAgent

class MazeMod:
    # initializer
    def __init__(self):
        print 'Creating MazeMod'
        self.epsilon = 0.5
        self.speedup = 0.0
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

    def delete_maze_objects(self):
        for id in self.wall_ids: # delete the walls
            removeObject(id)
        del self.wall_ids[:] # clear the ids

    def generate_new_maze(self):
        self.delete_maze_objects()
        MazeEnvironment.maze = Maze.generate(ROWS, COLS, GRID_DX, GRID_DY)
        self.add_maze_objects()
        self.reset_maze()

    def add_maze_objects(self):
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

    def set_environment(self,env):
        self.environment = env
        self.environment.epsilon = self.epsilon
        self.environment.speedup = self.speedup
        self.delete_maze_objects()
        set_environment(env)
        self.add_maze_objects()

    def reset_maze(self):
        """ reset the maze by removing the markers and starting the AI """
        # remove the agent
        if self.agent_id is not None:
            removeObject(self.agent_id)
        self.agent_id = None
        reset_ai()

    def stop_maze(self):
        """ stop the maze agent and reset the environment """
        if self.agent_id is not None:
            removeObject(self.agent_id)
        self.agent_id = None
        disable_ai()

    def start_agent(self, xml, env_class):
        """ start an agent """
        disable_ai()
        self.reset_maze()
        if self.environment.__class__.__name__ != env_class.__name__:
            self.set_environment(env_class())
        self.agent_id = addObject(xml, Vector3f(GRID_DX, GRID_DY, 0), type=AGENT_MASK )
        enable_ai()

    def start_dfs(self):
        """ start the depth first search demo """
        self.start_agent("data/shapes/character/SydneyDFS.xml", MazeEnvironment)

    def start_bfs(self):
        """ start the breadth first search demo """
        self.start_agent("data/shapes/character/SydneyBFS.xml", MazeEnvironment)

    def start_astar(self):
        """ start the A* search demo """
        self.start_agent("data/shapes/character/SydneyAStar.xml", MazeEnvironment)

    def start_astar2(self):
        """ start the A* search demo with teleporting agents """
        self.start_agent("data/shapes/character/SydneyAStar2.xml", MazeEnvironment)

    def start_astar3(self):
        """ start the A* search demo with teleporting agents and a front marked by moving agents """
        self.start_agent("data/shapes/character/SydneyAStar3.xml", MazeEnvironment)

    def start_fps(self, env_class = EgocentricMazeEnvironment):
        """ start the FPS navigation demo """
        disable_ai()
        self.reset_maze()
        #if self.environment.__class__.__name__ != env_class.__name__:
        self.set_environment(env_class())
        self.agent_id = addObject("data/shapes/character/SydneyFPS.xml", Vector3f(GRID_DX, GRID_DY, 2), type=AGENT_MASK )
        enable_ai()

    def start_fps_granular(self):
        """ start the FPS navigation demo with a more continous environment """
        disable_ai()
        self.reset_maze()
        #if self.environment.__class__.__name__ != EgocentricMazeEnvironment.__name__:
        self.set_environment(EgocentricMazeEnvironment(granularity=8))
        self.agent_id = addObject("data/shapes/character/SydneyFPS.xml", Vector3f(GRID_DX, GRID_DY, 2), type=AGENT_MASK )
        enable_ai()

    def start_random(self, env_class = MazeEnvironment):
        """ start the random baseline demo """
        self.start_agent("data/shapes/character/SydneyRandom.xml", env_class)

    def start_sarsa(self, env_class = MazeEnvironment):
        """ start the Sarsa RL demo """
        self.start_agent("data/shapes/character/SydneySarsa.xml", env_class)

    def start_qlearning(self, env_class = MazeEnvironment):
        """ start the Q-Learning RL demo """
        self.start_agent("data/shapes/character/SydneyQLearning.xml", env_class)

    def start_customrl(self, env_class = MazeEnvironment):
        """ start the Custom RL demo """
        self.start_agent("data/shapes/character/CustomRLRobot.xml", env_class)

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

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = MazeMod()
    return gMod
