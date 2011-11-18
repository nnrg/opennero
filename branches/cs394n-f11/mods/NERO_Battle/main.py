import common
import common.menu_utils
import NERO.agent
import NERO.client as client
import NERO.constants as constants
import NERO_Battle.module as module
import OpenNero

script_server = None

def ModMain():
    module.getMod()  # initialize the NERO_Battle module.
    client.ClientMain()

def ModTick(dt):
    global script_server
    if OpenNero.getAppConfig().rendertype == 'null':
        return
    if script_server is None:
        script_server = common.menu_utils.GetScriptServer()
        common.startScript("NERO_Battle/menu.py")
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
