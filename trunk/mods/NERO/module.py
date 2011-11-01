import os
import random

import common
import constants
import NeroEnvironment
import OpenNero


class NeroModule:
    def __init__(self):
        self.environment = None
        self.agent_id = None
        self.lt = constants.DEFAULT_LIFETIME
        self.dta = 50
        self.dtb = 50
        self.dtc = 50
        self.ff = 0
        self.ee = 0
        self.hp = 50
        self.flag_loc = OpenNero.Vector3f(0,0,0)
        self.flag_id = -1
        self.num_to_add = constants.pop_size
        (self.spawn_x,self.spawn_y) = (constants.XDIM/2, constants.YDIM/3)
        self.set_speedup(constants.DEFAULT_SPEEDUP)

    def setup_map(self):
        """
        setup the test environment
        """
        OpenNero.disable_ai()

        if self.environment:
            error("Environment already created")
            return

        common.startScript('NERO/menu.py')

        # create the environment - this also creates the rtNEAT object
        self.environment = NeroEnvironment()

        OpenNero.set_environment(self.environment)

        # flag placement
        self.flag_id = common.addObject(
            "data/shapes/cube/BlueCube.xml", \
            self.flag_loc, label="Flag", \
            scale = OpenNero.Vector3f(1, 1, 10), \
            type = constants.OBJECT_TYPE_FLAG)

        # world walls
        common.addObject(
            "data/shapes/cube/Cube.xml", \
            OpenNero.Vector3f(constatns.XDIM/2, 0, constants.HEIGHT + constants.OFFSET), \
            OpenNero.Vector3f(0, 0, 90), \
            scale = OpenNero.Vector3f(1, constants.XDIM, constants.HEIGHT), \
            label = "World Wall0", \
            type = constants.OBJECT_TYPE_OBSTACLE )
        common.addObject(
            "data/shapes/cube/Cube.xml", \
            OpenNero.Vector3f(0, constants.YDIM/2, constants.HEIGHT + constants.OFFSET), \
            OpenNoer.Vector3f(0, 0, 0), \
            scale=OpenNero.Vector3f(1, constants.YDIM, constants.HEIGHT), \
            label="World Wall1", \
            type = constants.OBJECT_TYPE_OBSTACLE )
        common.addObject(
            "data/shapes/cube/Cube.xml", \
            OpenNero.Vector3f(constants.XDIM, constants.YDIM/2, constants.HEIGHT + constants.OFFSET), \
            OpenNero.Vector3f(0, 0, 0), \
            scale = OpenNero.Vector3f(1, constants.YDIM, constants.HEIGHT), \
            label = "World Wall2", \
            type = constants.OBJECT_TYPE_OBSTACLE )
        common.addObject(
            "data/shapes/cube/Cube.xml", \
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM, constants.HEIGHT + constants.OFFSET), \
            OpenNero.Vector3f(0, 0, 90), \
            scale = OpenNero.Vector3f(1, constants.XDIM, constants.HEIGHT), \
            label = "World Wall3", \
            type = constants.OBJECT_TYPE_OBSTACLE )

        # Add the surrounding Environment
        common.addObject(
            "data/terrain/NeroWorld.xml", \
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM/2, 0), \
            scale = OpenNero.Vector3f(1, 1, 1), \
            label = "NeroWorld", \
            type = constants.OBJECT_TYPE_LEVEL_GEOM )

        return True

    def change_flag(self, new_loc):
        self.flag_loc = OpenNero.Vector3f(new_loc[0],new_loc[1],new_loc[2])

        removeObject(self.flag_id)

        self.flag_id = common.addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag", scale=OpenNero.Vector3f(1,1,10), type = constants.OBJECT_TYPE_FLAG)

    def place_basic_turret(self, loc):
        common.addObject("data/shapes/character/steve_basic_turret.xml",OpenNero.Vector3f(loc[0],loc[1],loc[2]),type = constants.OBJECT_TYPE_TEAM_1)

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        # Generate an initial rtNEAT Agent
        dx = random.randrange(constants.XDIM/20) - constants.XDIM/40
        dy = random.randrange(constants.XDIM/20) - constants.XDIM/40
        dx = random.randrange(constants.XDIM/20) - constants.XDIM/40
        dy = random.randrange(constants.XDIM/20) - constants.XDIM/40
        id = common.addObject("data/shapes/character/steve_blue_armed.xml",OpenNero.Vector3f(self.spawn_x + dx,self.spawn_y + dy,2),type = constants.OBJECT_TYPE_TEAM_0)
        enable_ai()
        self.num_to_add -= 1

    #The following is run when the Save button is pressed
    def save_rtneat(self, location, pop):
        import os
        location = os.path.relpath("/") + location
        get_ai("rtneat").save_population(str(location))

    #The following is run when the Load button is pressed
    def load_rtneat(self, location , pop):
        import os
        global rtneat
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            rtneat = RTNEAT(str(location), "data/ai/neat-params.dat", pop_size, get_environment().agent_info.reward)
            set_ai("rtneat",rtneat)

    def set_speedup(self, speedup):
        OpenNero.getSimContext().delay = 1.0 - (speedup/100.0)
        if self.environment:
            self.environment.speedup = (speedup/100.0)

    def set_spawn(self, x, y):
        self.spawn_x = x
        self.spawn_y = y

    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        i = constants.FITNESS_INDEX[key]
        value = (value - 100) / 100.0 # value in [-1, 1]
        rtneat = OpenNero.get_ai("rtneat")
        if rtneat:
            rtneat.set_weight(i, value)
        print key, value

    def ltChange(self, value):
        self.lt = value
        rtneat = OpenNero.get_ai("rtneat")
        print 'lifetime:', value
        if rtneat:
            rtneat.set_lifetime(value)
            print 'rtNEAT lifetime:', value

    def dtaChange(self, value):
        self.dta = value
        print 'Distance to approach A:', value

    def dtbChange(self, value):
        self.dtb = value
        print 'Distance to approach B:', value

    def dtcChange(self, value):
        self.dtc = value
        print 'Distance to approach C:', value

    def ffChange(self, value):
        self.ff = value
        print 'Friendly fire:', value

    def eeChange(self, value):
        self.ee = value
        print 'Explore/exploit:', value

    def hpChange(self, value):
        self.hp = value
        print 'Hit points:', value

    def getNumToAdd(self):
        return self.num_to_add

    #This is the function ran when an agent already in the field causes the generation of a new agent
    def addAgent(self):
        dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        common.addObject("data/shapes/character/steve_blue_armed.xml",
                         OpenNero.Vector3f(self.spawn_x + dx, self.spawn_y + dy, 2),
                         type=constants.OBJECT_TYPE_TEAM_0)
        self.num_to_add -= 1

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule.NeroModule()
    return gMod

def parseInput(strn):
    from NERO.client import toggle_ai_callback
    if strn == "deploy": return
    if len(strn) < 2: return
    mod = getMod()
    loc, val = strn.split(' ')
    vali = 1
    if strn.isupper(): vali = int(val)
    if loc == "SG": mod.set_weight(constants.FITNESS_STAND_GROUND, vali)
    if loc == "ST": mod.set_weight(constants.FITNESS_STICK_TOGETHER, vali)
    if loc == "TD": mod.dtaChange(vali)
    if loc == "AE": mod.set_weight(constants.FITNESS_APPROACH_ENEMY, vali)
    if loc == "ED": mod.dtbChange(vali)
    if loc == "AF": mod.set_weight(constants.FITNESS_APPROACH_FLAG, vali)
    if loc == "FD": mod.dtcChange(vali)
    if loc == "HT": mod.set_weight(constants.FITNESS_HIT_TARGET, vali)
    if loc == "VF": mod.set_weight(constants.FITNESS_AVOID_FIRE, vali)
    if loc == "LT": mod.ltChange(vali)
    if loc == "FF": mod.ffChange(vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_rtneat(val, 1)
    if loc == "load1": mod.load_rtneat(val, 1)
    if loc == "save2": mod.save_rtneat(val, 2)
    if loc == "load2": mod.load_rtneat(val, 2)
    if loc == "deploy": toggle_ai_callback()

def ServerMain():
    print "Starting mod NERO"
