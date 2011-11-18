#import all agent implementations
import NERO.agent

#import all client and server scripts
import NERO_Battle.module as module
import NERO.client as client
import common
import common.menu_utils

script_server = None

def ModMain():
    global script_server
    module.getMod()  # initialize the NERO_Battle module.
    script_server = common.menu_utils.GetScriptServer()
    common.startScript("NERO_Battle/menu.py")
    client.ClientMain()

def ModTick(dt):
    if script_server is None:
        return
    data = script_server.read_data()
    while data:
        module.parseInput(data.strip())
        data = script_server.read_data()

def Match(team0, team1):
    '''Run a single battle between two population files.'''
    mod = module.getMod()
    mod.load_team(team0, constants.OBJECT_TYPE_TEAM_0)
    mod.load_team(team1, constants.OBJECT_TYPE_TEAM_1)
    mod.set_speedup(100)
