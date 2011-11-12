import os
import random

import NERO.module
import NERO.constants as constants
import NeroEnvironment
import OpenNero
import client


class NeroModule(NERO.module.NeroModule):
    def __init__(self):
        NERO.module.NeroModule.__init__(self)

        # Set up random starting positions for each team. Team 0 starts
        # somewhere in the lower-left quadrant of the field, and team 1 starts
        # somewhere in the upper-right.

        x = constants.XDIM / 2.0
        y = constants.YDIM / 2.0

        def clip(lo, hi):
            return lambda a: max(lo, min(hi, a))
        cx = clip(constants.XDIM * 0.1, constants.XDIM * 0.9)
        cy = clip(constants.YDIM * 0.1, constants.YDIM * 0.9)

        rr = random.randrange

        self.set_spawn(cx(rr(x)), cy(rr(y)), constants.OBJECT_TYPE_TEAM_0)
        self.set_spawn(cx(x + rr(x)), cy(y + rr(y)), constants.OBJECT_TYPE_TEAM_1)

    def create_environment(self):
        return NeroEnvironment.NeroEnvironment()

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        NERO.module.NeroModule.start_rtneat(self)
        self.spawnAgent(constants.OBJECT_TYPE_TEAM_1)

    #The following is run when the Save button is pressed
    def save_rtneat(self, location, team):
        location = os.path.relpath("/") + location
        OpenNero.get_ai("rtneat-%s" % team).save_population(str(location))

    #The following is run when the Load button is pressed
    def load_rtneat(self, location, team):
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            rtneat = OpenNero.RTNEAT(str(location),
                                     "data/ai/neat-params.dat",
                                     constants.pop_size,
                                     OpenNero.get_environment().agent_info.reward)
            # in Battle, the lifetime is basically infinite, unless they get killed
            rtneat.set_lifetime(sys.maxint)
            rtneat.disable_evolution()
            OpenNero.set_ai("rtneat-%s" % team, rtneat)


def delMod():
    NERO.module.gMod = None

def getMod():
    if not NERO.module.gMod:
        NERO.module.gMod = NeroModule()
    return NERO.module.gMod

def parseInput(strn):
    if strn == "deploy" or len(strn) < 2:
        return
    mod = getMod()
    loc, val = strn.split(' ')
    vali = 1
    if strn.isupper():
        vali = int(val)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_rtneat(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "load1": mod.load_rtneat(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "save2": mod.save_rtneat(val, constants.OBJECT_TYPE_TEAM_1)
    if loc == "load2": mod.load_rtneat(val, constants.OBJECT_TYPE_TEAM_1)
    if loc == "deploy": client.toggle_ai_callback()

def ServerMain():
    print "Starting mod NERO_Battle"
