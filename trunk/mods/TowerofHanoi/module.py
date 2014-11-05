import OpenNero
import common
import TowerofHanoi
import TowerofHanoi.agent
import TowerofHanoi.constants as constants
from TowerofHanoi.constants import *
from TowerofHanoi.environment import TowerEnvironment

class TowerMod:
    # initializer
    def __init__(self):
        print 'Creating TowerMod'
        self.environment = None
        self.agent_id = None # the ID of the agent
        self.marker_map = {} # a map of cells and markers so that we don't have more than one per cell
        self.marker_states = {} # states of the marker agents that run for one cell and stop
        self.agent_map = {} # agents active on the map
        self.wall_ids = [] # walls on the map
        self.AGENTS = [
    ('Problem reduction', lambda: getMod().start_agent_problem_reduction(), MODE_PLANNING ),
    ('State-space search', lambda: getMod().start_agent_state_space_search(), MODE_PLANNING ),
    ('Goal stacking - 2 Disks', lambda: getMod().start_agent_strips_2_disk(), MODE_PLANNING ),
    ('Goal stacking - 3 Disks', lambda: getMod().start_agent_strips_3_disk(), MODE_PLANNING ),
    ('Semantic Parser', lambda: getMod().start_agent_nlp(), MODE_NLP ),
]

    def __del__(self):
        print 'Deleting TowerMod'

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
        for id in self.wall_ids: # delete the walls
            common.removeObject(id)
        del self.wall_ids[:] # clear the ids
        OpenNero.set_environment(env)

        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(1 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))
        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(2 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))
        common.addObject("data/shapes/cube/WhiteCube.xml", OpenNero.Vector3f(3 * constants.GRID_DX, 2 * constants.GRID_DY, 0 * constants.GRID_DZ), OpenNero.Vector3f(0,0,0), scale=OpenNero.Vector3f(.25,.25,4))

    def get_num_disks(self):
        return self.num_disks

    def start_agent_problem_reduction(self): #Problem reduction
        """ start the tower demo """
        self.num_disks = 3
        OpenNero.disable_ai()
        self.stop_agent()
        env = TowerEnvironment()
        env.initialize_blocks()
        self.set_environment(env)
        self.agent_id = common.addObject("data/shapes/character/BlocksRobot.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        OpenNero.enable_ai()

    def start_agent_state_space_search(self): #State-space search
        """ start the tower demo """
        self.num_disks = 3
        OpenNero.disable_ai()
        self.stop_agent()
        env = TowerEnvironment()
        env.initialize_blocks()
        self.set_environment(env)
        #self.set_environment(TowerEnvironment())
        self.agent_id = common.addObject("data/shapes/character/BlocksRobot2.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        OpenNero.enable_ai()

    def start_agent_strips_2_disk(self): #2 Disk Goal Stack Planner
        """ start the tower demo """
        self.num_disks = 2
        OpenNero.disable_ai()
        self.stop_agent()
        env = TowerEnvironment()
        env.initialize_blocks()
        self.set_environment(env)
        #self.set_environment(TowerEnvironment())
        self.agent_id = common.addObject("data/shapes/character/BlocksRobot3.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        OpenNero.enable_ai()
        
    def start_agent_strips_3_disk(self): #3 Disk Goal Stack Planner
        """ start the tower demo """
        self.num_disks = 3
        OpenNero.disable_ai()
        self.stop_agent()
        env = TowerEnvironment()
        env.initialize_blocks()
        self.set_environment(env)
        #self.set_environment(TowerEnvironment())
        self.agent_id = common.addObject("data/shapes/character/BlocksRobot4.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        OpenNero.enable_ai()
        
    def start_agent_nlp(self): #Natural Language Processing
        """ start the tower demo """
        self.num_disks = 3
        OpenNero.disable_ai()
        self.stop_agent()
        env = TowerEnvironment()
        env.initialize_blocks()
        self.set_environment(env)
        #self.set_environment(TowerEnvironment())
        self.agent_id = common.addObject("data/shapes/character/BlocksRobot5.xml", OpenNero.Vector3f(constants.GRID_DX, constants.GRID_DY, 2), type=constants.AGENT_MASK, scale=OpenNero.Vector3f(3,3,3))
        OpenNero.enable_ai()
        
    def stop_agent(self):
        if self.agent_id is not None:
            common.removeObject(self.agent_id)
        self.agent_id = None

    def control_fps(self,key):
        FirstPersonAgent.key_pressed = key

    def set_speedup(self, speedup):
        print 'Speedup set to', speedup
        # speed up between 0 (delay set to 1 second) and 1 (delay set to 0)
        OpenNero.getSimContext().delay = 1.0 - speedup

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = TowerMod()
    return gMod
