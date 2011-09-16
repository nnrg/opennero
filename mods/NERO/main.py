#import all client and server scripts
from NERO.module import *
from NERO.client import *

def ModMain():
    ClientMain()

script_server = GetScriptServer()

def ModTick(dt):
    startScript("NERO/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()
