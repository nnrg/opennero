#import all agent implementations
import NERO.agent

#import all client and server scripts
import NERO_Battle.module as module
import NERO_Battle.client as client
import common
import common.menu_utils

script_server = None

def ModMain():
    client.ClientMain()

def ModTick(dt):
    if script_server is None:
        return
    common.startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        module.parseInput(data.strip())
        data = script_server.read_data()

def Match(team0, team1):
    getMod().load_rtneat(team0, constants.OBJECT_TYPE_TEAM_0)
    getMod().load_rtneat(team1, constants.OBJECT_TYPE_TEAM_1)
    getMod().set_speedup(100)
    getMod().start_rtneat()
