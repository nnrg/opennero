#Note this mod is intended for use in competetive open nero tornaments, and therefore lacks many of the key features of the NERO mod

#import all client and server scripts
from Battle.module import *
from Battle.client import *

def ModMain():
    ClientMain()

script_server = ScriptServer()

def ModTick(dt):
    startScript("Battle/menu.py")
    data = script_server.read_data()
    while data:
        parseInput(data.strip())
        data = script_server.read_data()
