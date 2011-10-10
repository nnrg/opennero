#import all client and server scripts
from NERO_Battle.module import *
from NERO_Battle.client import *

script_server = None

def ModMain():
    ClientMain()

def ModTick(dt):
    if script_server is None:
        return
    startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()

def Match(team0, team1):
    getMod().load_rtneat(team0, "blue")
    getMod().load_rtneat(team1, "red")
    getMod().set_speedup(100)
    getMod().start_rtneat()
