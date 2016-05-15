import sys

import OpenNero

import NERO.module
import NERO.constants as constants
import common
import common.menu_utils
import environment

class BattleModule(NERO.module.NeroModule):
    def create_environment(self):
        return environment.BattleEnvironment()

def delMod():
    NERO.module.gMod = None

def getMod():
    if not NERO.module.gMod:
        NERO.module.gMod = BattleModule()
    return NERO.module.gMod

script_server = None
    
def getServer():
    global script_server
    if script_server is None:
        script_server = common.menu_utils.GetScriptServer()
        common.startScript("NERO_Battle/menu.py")
    return script_server

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
