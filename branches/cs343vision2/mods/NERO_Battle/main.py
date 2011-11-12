#import all agent implementations
from NERO.RTNEATAgent import *
from NERO.Turret import *

#import all client and server scripts
import NERO_Battle.module as module
import NERO_Battle.client as client
import common
import common.menu_utils

def ModMain():
    client.ClientMain()

script_server = common.menu_utils.GetScriptServer()

def ModTick(dt):
    common.startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        module.parseInput(data.strip())
        data = script_server.read_data()
