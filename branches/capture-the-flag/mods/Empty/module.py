from Empty.EmptyEnvironment import *
from Empty.EmptyAgent import *

class EmptyModule:
    def __init__(self):
        pass

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = EmptyModule()
    return gMod

def ServerMain():
    print "Starting mod Empty"
