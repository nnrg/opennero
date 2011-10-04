#import all client and server scripts
from NERO_Battle.module import *
from NERO_Battle.client import *

script_server = None

def ModMain():
    ClientMain()

def ModTick(dt):
    startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()

def Start(port, team0, team1):
    global script_server
    script_server = GetScriptServer(port)
    getMod().load_rtneat(team0, "red")
    getMod().load_rtneat(team1, "blue")
    getMod().set_speedup(100)
    getMod().start_rtneat()
