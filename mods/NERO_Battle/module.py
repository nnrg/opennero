import sys

import NERO.module
import NERO.constants as constants
import NeroEnvironment
import OpenNero

# this value, between 0 (slowest) and 100 (fastest)
# overrides the default from NERO Training
BATTLE_DEFAULT_SPEEDUP = 80

class NeroModule(NERO.module.NeroModule):
    def __init__(self):
        NERO.module.NeroModule.__init__(self)
        if OpenNero.getAppConfig().rendertype != 'null':
            self.set_speedup(BATTLE_DEFAULT_SPEEDUP)
            print 'setting speedup for on-screen battle'

    def create_environment(self):
        return NeroEnvironment.NeroEnvironment()

    def load_team(self, location, team=constants.OBJECT_TYPE_TEAM_0):
        NERO.module.NeroModule.load_team(self, location, team)
        rtneat = OpenNero.get_ai('rtneat-%s' % team)
        if rtneat:
            rtneat.set_lifetime(sys.maxint)
            rtneat.disable_evolution()
        OpenNero.disable_ai() # don't run until button

    def start_rtneat(self, team=constants.OBJECT_TYPE_TEAM_0):
        NERO.module.NeroModule.start_rtneat(self, team)
        rtneat = OpenNero.get_ai('rtneat-%s' % team)
        if rtneat:
            rtneat.set_lifetime(sys.maxint)
            rtneat.disable_evolution()


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
    # first word is command rest is filename
    loc, val = strn.split(' ',1)
    vali = 1
    if strn.isupper():
        vali = int(val)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "load1": mod.load_team(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "load2": mod.load_team(val, constants.OBJECT_TYPE_TEAM_1)
    if loc == "rtneat":
        mod.deploy('rtneat', constants.OBJECT_TYPE_TEAM_0)
        mod.deploy('rtneat', constants.OBJECT_TYPE_TEAM_1)
    if loc == "qlearning":
        mod.deploy('qlearning', constants.OBJECT_TYPE_TEAM_0)
        mod.deploy('qlearning', constants.OBJECT_TYPE_TEAM_1)
    if loc == "pause": OpenNero.disable_ai()
    if loc == "resume": OpenNero.enable_ai()

def ServerMain():
    print "Starting mod NERO_Battle"
