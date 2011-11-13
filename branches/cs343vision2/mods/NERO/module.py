import os
import random

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
        self.ee = 50
        self.hp = 20

        self.flag_loc = None
        self.flag_id = None

        self.set_speedup(constants.DEFAULT_SPEEDUP)
        self.first_person_agent = None

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

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        self.spawnAgent()
        OpenNero.enable_ai()
        
    # this is called when we get a "First Person Agent" button hit
    def start_fps(self):
        print 'start_fps was called'
        if self.first_person_agent is None:
            print 'adding first person agent!'
            self.spawnAgent(agent_xml = 'data/shapes/character/FirstPersonAgent.xml')
            OpenNero.enable_ai()
        else:
            print 'removing first person agent!'
            common.removeObject(self.first_person_agent)

    #The following is run when the Save button is pressed
    def save_rtneat(self, location, pop, team=constants.OBJECT_TYPE_TEAM_0):
        location = os.path.relpath("/") + location
        OpenNero.get_ai("rtneat-%s" % team).save_population(str(location))

    #The following is run when the Load button is pressed
    def load_rtneat(self, location , pop, team=constants.OBJECT_TYPE_TEAM_0):
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            OpenNero.set_ai("rtneat-%s" % team, OpenNero.RTNEAT(
                    str(location), "data/ai/neat-params.dat",
                    constants.pop_size,
                    OpenNero.get_environment().agent_info.reward))

    def set_speedup(self, speedup):
        OpenNero.getSimContext().delay = 1.0 - (speedup/100.0)
        if self.environment:
            self.environment.speedup = (speedup/100.0)

    def set_spawn(self, x, y, team=constants.OBJECT_TYPE_TEAM_0):
        self.spawn_x[team] = x
        self.spawn_y[team] = y

    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        i = constants.FITNESS_INDEX[key]
        value = (value - 100) / 100.0 # value in [-1, 1]
        for team in (constants.OBJECT_TYPE_TEAM_0, constants.OBJECT_TYPE_TEAM_1):
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_weight(i, value)
        print key, value

    def ltChange(self, value):
        self.lt = value
        for team in (constants.OBJECT_TYPE_TEAM_0, constants.OBJECT_TYPE_TEAM_1):
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_lifetime(value)

    def eeChange(self, value):
        self.ee = value
        print 'Explore/exploit:', value

    def hpChange(self, value):
        self.hp = value
        print 'Hit points:', value

    def spawnAgent(self, team = constants.OBJECT_TYPE_TEAM_0, agent_xml = None):
        """
        This is the function ran when an agent already in the field
        causes the generation of a new agent.
        """
        self.curr_team = team
        if agent_xml is None:
            dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
            dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
            color = 'blue'
            if team == constants.OBJECT_TYPE_TEAM_1:
                color = 'red'
            agent_xml = "data/shapes/character/steve_%s_armed.xml" % color
            agent_pos = OpenNero.Vector3f(self.spawn_x[team] + dx, self.spawn_y[team] + dy, 2)
        else:
            agent_pos = OpenNero.Vector3f(self.spawn_x[team], self.spawn_y[team], 2)
        common.addObject(agent_xml, agent_pos, type=team)

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
    if loc == "save1": mod.save_rtneat(val, 1)
    if loc == "load1": mod.load_rtneat(val, 1)
    if loc == "save2": mod.save_rtneat(val, 2)
    if loc == "load2": mod.load_rtneat(val, 2)
    if loc == "deploy": client.toggle_ai_callback()
    if loc == "fps": mod.start_fps()

def ServerMain():
    print "Starting mod NERO"
