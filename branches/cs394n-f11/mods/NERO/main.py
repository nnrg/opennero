#import all agent implementations
from NERO.RTNEATAgent import *
from NERO.Turret import *

#import all client and server scripts
import NERO.module
import NERO.client
import common
import common.menu_utils

def ModMain():
    NERO.client.ClientMain()

script_server = common.menu_utils.GetScriptServer()

def ModTick(dt):
    common.startScript("NERO/menu.py")
    data = script_server.read_data()
    while data:
        NERO.module.parseInput(data.strip())
        data = script_server.read_data()
