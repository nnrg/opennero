import OpenNero
import common
import constants
from BlocksTower.environment import TowerEnvironment
import BlocksTower.agent

class TowerMod:
    # initializer
    def __init__(self):
        print 'Creating TowerMod'
        self.epsilon = 0.5
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
            common.removeObject(self.marker_map[(r,c)])
        # remember the ID of the marker
        self.marker_map[(r,c)] = common.addObject(marker, OpenNero.Vector3f( (r+1) * constants.GRID_DX, (c+1) * constants.GRID_DY, -1))

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
            common.removeObject(self.agent_map[(r,c)])
            del self.marker_states[self.agent_map[(r,c)]]
            del self.agent_map[(r,c)]

    def mark_maze_agent(self, agent, r1, c1, r2, c2):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        self.unmark_maze_agent(r2,c2)
        # add a new marker object
        agent_id = common.addObject(agent, OpenNero.Vector3f( (r1+1) * constants.GRID_DX, (c1+1) * constants.GRID_DY, 2) )
        self.marker_states[agent_id] = ((r1, c1), (r2, c2))
        self.agent_map[(r2,c2)] = agent_id

    # add a set of coordinate axes
    def addAxes(self):
        OpenNero.getSimContext().addAxes()

    def add_maze(self):
        """Add a randomly generated maze"""
        if self.environment:
            print "ERROR: Environment already created"
            return
        self.set_environment(TowerEnvironment())

    def set_environment(self,env):
        self.environment = env
        self.environment.epsilon = self.epsilon
        self.environment.shortcircuit = self.shortcircuit
        for id in self.wall_ids: # delete the walls
            common.removeObject(id)
        del self.wall_ids[:] # clear the ids
        OpenNero.set_environment(env)

        self.block_ids = []

        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(1 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))
        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(2 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))
        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(3 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))

    def shortcircuit(self):
        raise NotImplementedError

    def num_towers(self):
        return num_towers

    def start_tower(self,n):
        """ start the tower demo """
        self.num_towers = n
        OpenNero.disable_ai()
        if self.environment.__class__.__name__ != 'TowerEnvironment':
            self.set_environment(TowerEnvironment())
        if len(self.environment.block_states) == 0:
         self.agent_id = common.addObject("data/shapes/character/BlocksRobot.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        else:
            for obj in self.environment.states: self.environment.reset(obj)
        OpenNero.enable_ai()

    def control_fps(self,key):
        FirstPersonAgent.key_pressed = key

    def set_epsilon(self, epsilon):
        self.epsilon = epsilon
        print 'Epsilon set to', self.epsilon
        if self.environment:
            self.environment.epsilon = epsilon

    def set_speedup(self, speedup):
        print 'Speedup set to', speedup
        # speed up between 0 (delay set to 1 second) and 1 (delay set to 0)
        OpenNero.getSimContext().delay = 1.0 - speedup

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
