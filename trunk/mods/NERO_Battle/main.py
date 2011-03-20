#import all client and server scripts
from NERO_Battle.module import *
from NERO_Battle.client import *

def ModMain():
    ClientMain()
    #Fight1('/Users/ikarpov/OpenNERO/tourney/DanMoreno.rtf','/Users/ikarpov/OpenNERO/tourney/DanMorenoV2.rtf')

script_server = GetScriptServer()

def ModTick(dt):
    startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()

def Fight1(team1, team2):
    ClientMain()
    getMod().change_flag([XDIM/2, YDIM/2, HEIGHT])
    getMod().set_spawn_1(XDIM/2, 0.20 * YDIM)
    getMod().set_spawn_2(XDIM/2, 0.80 * YDIM)
    getMod().load_rtneat(team1,0)
    getMod().load_rtneat(team2,1)
    toggle_ai_callback()
    
def Fight2(team1, team2):
    ClientMain()
    addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 90), scale=Vector3f(10, 0.6 * YDIM, HEIGHT), label="World Wall4", type = OBJECT_TYPE_OBSTACLE )
    getMod().change_flag([XDIM/2, YDIM/2, HEIGHT])
    getMod().set_spawn_1(XDIM/2, YDIM/2)
    getMod().set_spawn_2(XDIM/2, YDIM/2)
    getMod().load_rtneat(team1,0)
    getMod().load_rtneat(team2,1)
    toggle_ai_callback()
    
def Fight3(team1, team2):
    ClientMain()
    getMod().change_flag([XDIM/2, YDIM/2, HEIGHT])
    getMod().load_rtneat(team1,0)
    getMod().load_rtneat(team2,1)
    toggle_ai_callback()