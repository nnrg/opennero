#import all client and server scripts
from NERO_Battle.module import *
from NERO_Battle.client import *

def ModMain():
    ClientMain()  

script_server = GetScriptServer()

def ModTick(dt):
    startScript("NERO_Battle/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()
