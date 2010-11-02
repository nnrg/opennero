from NeroSociety.NSEnvironment import *
from NeroSociety.NSAgent import *

class NSModule:
    def __init__(self):
        pass

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NSModule()
    return gMod

def ServerMain():
    print "Starting mod nerosociety"
