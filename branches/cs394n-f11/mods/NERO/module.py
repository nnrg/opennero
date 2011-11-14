import os
import random

import NERO.agent
import client
import common
import constants
import NeroEnvironment
import OpenNero


class NeroModule:
    def __init__(self):
        self.environment = None
        self.agent_id = None

        self.lt = constants.DEFAULT_LIFETIME

        self.flag_loc = None
        self.flag_id = None

        self.set_speedup(constants.DEFAULT_SPEEDUP)

        self.spawn_x = {}
        self.spawn_y = {}
        self.set_spawn(constants.XDIM / 2, constants.YDIM / 3)

    def setup_map(self):
        """
        setup the test environment
        """
        OpenNero.disable_ai()

        if self.environment:
            error("Environment already created")
            return

        # create the environment - this also creates the rtNEAT object
        self.environment = self.create_environment()
        OpenNero.set_environment(self.environment)

        # world walls
        height = constants.HEIGHT + constants.OFFSET
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, 0, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(1, constants.XDIM, constants.HEIGHT),
            label="World Wall0",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(0, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 0),
            scale=OpenNero.Vector3f(1, constants.YDIM, constants.HEIGHT),
            label="World Wall1",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 0),
            scale=OpenNero.Vector3f(1, constants.YDIM, constants.HEIGHT),
            label="World Wall2",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(1, constants.XDIM, constants.HEIGHT),
            label="World Wall3",
            type=constants.OBJECT_TYPE_OBSTACLE)

        # Add an obstacle wall in the middle
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(1, constants.YDIM / 4, constants.HEIGHT),
            label="World Wall4",
            type=constants.OBJECT_TYPE_OBSTACLE)

        # Add the surrounding Environment
        common.addObject(
            "data/terrain/NeroWorld.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM/2, 0),
            scale=OpenNero.Vector3f(1, 1, 1),
            label="NeroWorld",
            type=constants.OBJECT_TYPE_LEVEL_GEOM)

        return True

    def create_environment(self):
        return NeroEnvironment.NeroEnvironment()

    def change_flag(self, loc):
        if self.flag_id:
            common.removeObject(self.flag_id)
        self.flag_loc = OpenNero.Vector3f(*loc)
        self.flag_id = common.addObject(
            "data/shapes/cube/BlueCube.xml",
            self.flag_loc,
            label="Flag",
            scale=OpenNero.Vector3f(1, 1, 10),
            type=constants.OBJECT_TYPE_FLAG)

    def place_basic_turret(self, loc):
        common.addObject(
            "data/shapes/character/steve_basic_turret.xml",
            OpenNero.Vector3f(*loc),
            type=constants.OBJECT_TYPE_TEAM_1)

    #The following is run when one of the Deploy buttons is pressed
    def start_ai(self, ai_flavor):
        self.spawnAgent(ai_flavor=ai_flavor)
        OpenNero.enable_ai()

    #The following is run when the Save button is pressed
    def save_population(self, location, pop, team=constants.OBJECT_TYPE_TEAM_0):
        location = os.path.relpath("/") + location
        if self.environment.ai_flavor == 'rtneat':
            OpenNero.get_ai("rtneat-%s" % team).save_population(str(location))
        if self.environment.ai_flavor == 'qlearning':
            with open(str(location), 'w') as handle:
                for agent in self.environment.teams[team]:
                    if agent.group == 'Agent':
                        handle.write('%s\n\n' % agent.to_string())

    #The following is run when the Load button is pressed
    def load_population(self, location , pop, team=constants.OBJECT_TYPE_TEAM_0):
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            if self.environment.ai_flavor == 'rtneat':
                OpenNero.set_ai("rtneat-%s" % team, OpenNero.RTNEAT(
                        str(location), "data/ai/neat-params.dat",
                        constants.pop_size,
                        OpenNero.get_environment().agent_info.reward))
            if self.environment.ai_flavor == 'qlearning':
                with open(str(location)) as handle:
                    for _ in range(constants.pop_size):
                        chunk = ''
                        while not chunk.endswith('\n\n'):
                            chunk += handle.read(1)
                        agent = NERO.agent.QLearningAgent()
                        agent.from_string(chunk)
                        agent.team = team
                        self.environment.states[agent] = NeroEnvironment.AgentState(agent)

    def set_speedup(self, speedup):
        OpenNero.getSimContext().delay = 1.0 - (speedup / 100.0)
        if self.environment:
            self.environment.speedup = speedup / 100.0

    def set_spawn(self, x, y, team=constants.OBJECT_TYPE_TEAM_0):
        self.spawn_x[team] = x
        self.spawn_y[team] = y

    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        # value in [-1, 1]
        self.environment.set_weight(key, (value - 100) / 100.0)

    def ltChange(self, value):
        self.lt = value
        for team in constants.TEAMS:
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_lifetime(value)

    def eeChange(self, value):
        print 'Explore/exploit:', value
        self.environment.epsilon = value / 100.0

    def hpChange(self, value):
        print 'Hit points:', value
        self.environment.hitpoints = value

    def spawnAgent(self, team=constants.OBJECT_TYPE_TEAM_0, ai_flavor=None):
        """
        This is the function ran when an agent already in the field
        causes the generation of a new agent.
        """
        if not self.environment:
            return

        if ai_flavor is not None:
            self.environment.start_ai(ai_flavor)

        self.curr_team = team

        ai = 'rtneat'
        if ai_flavor == 'qlearning':
            ai = 'qlearning'

        color = 'blue'
        if team == constants.OBJECT_TYPE_TEAM_1:
            color = 'red'

        dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        common.addObject(
            "data/shapes/character/steve_%s_%s.xml" % (color, ai),
            OpenNero.Vector3f(self.spawn_x[team] + dx, self.spawn_y[team] + dy, 2),
            type=team)


gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod

def parseInput(strn):
    if strn == "deploy" or len(strn) < 2:
        return
    mod = getMod()
    loc, val = strn.split(' ')
    vali = 1
    if strn.isupper():
        vali = int(val)
    if loc == "SG": mod.set_weight(constants.FITNESS_STAND_GROUND, vali)
    if loc == "ST": mod.set_weight(constants.FITNESS_STICK_TOGETHER, vali)
    if loc == "AE": mod.set_weight(constants.FITNESS_APPROACH_ENEMY, vali)
    if loc == "AF": mod.set_weight(constants.FITNESS_APPROACH_FLAG, vali)
    if loc == "HT": mod.set_weight(constants.FITNESS_HIT_TARGET, vali)
    if loc == "VF": mod.set_weight(constants.FITNESS_AVOID_FIRE, vali)
    if loc == "LT": mod.ltChange(vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_population(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "load1": mod.load_population(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "rtneat": client.toggle_ai_callback('rtneat')
    if loc == "qlearning": client.toggle_ai_callback('qlearning')

def ServerMain():
    print "Starting mod NERO"
