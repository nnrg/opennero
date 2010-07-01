from common import *
from OpenNero import getSimContext
from NERO.NeroEnvironment import *
from NERO.RTNEATAgent import *
from NERO.Turret import *
import subprocess
import os
import sys

import random

XDIM = 800
YDIM = 400
HEIGHT = 20
OFFSET = -HEIGHT/2

NEAT_ACTIONS = 5
NEAT_SENSORS = 15
pop_size = 50

OBSTACLE = 1 #0b0001
AGENT = 2 #0b0010

class NeroModule:
    def __init__(self):
        global rtneat, rtneat2
        rtneat = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS, NEAT_ACTIONS, pop_size, 1.0)
        rtneat2 = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS, NEAT_ACTIONS, pop_size,1.0)
        self.environment = None
        self.agent_id = None
        self.agent_map = {}
        self.weights = Fitness()
        self.lt = 10
        self.dta = 50
        self.dtb = 50
        self.dtc = 50
        self.ff =  0
        self.ee =  0
        self.hp = 50
        self.currTeam = 1
        self.flag_loc = Vector3f(20,20,0)

    def setup_map(self):
        """
        setup the test environment
        """
        global XDIM, YDIM, HEIGHT, OFFSET
        if self.environment:
            error("Environment already created")
            return
        
        self.environment = NeroEnvironment(XDIM, YDIM)

        set_environment(self.environment)
        
        # flag placement
        addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag")

        # world walls
        # addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2,0,HEIGHT+OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall0", type = OBSTACLE )
        # addObject("data/shapes/cube/Cube.xml", Vector3f(0, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall1", type = OBSTACLE )
        # addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall2", type = OBSTACLE )
        # addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM, HEIGHT +OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall3", type = OBSTACLE )
        # addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1, YDIM,HEIGHT), label="World Wall4", type = OBSTACLE)

        # Add the surrounding Environment
        addObject("data/terrain/NeroWorld.xml", Vector3f(XDIM/2, YDIM/2, 0), scale=Vector3f(1, 1, 1), label="NeroWorld", type = OBSTACLE)

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        global rtneat, rtneat2
        disable_ai()
        from NeroEnvironment import NeroEnvironment

        # Create RTNEAT Objects
        set_ai("neat1",rtneat)
        set_ai("neat2", rtneat2)
        enable_ai()
        for i in range(0, 10):
            self.agent_map[(0,i)] = getNextFreeId()
            dx = random.randrange(XDIM/20) - XDIM/40
            dy = random.randrange(XDIM/20) - XDIM/40
            if i % 2 == 0:
                self.currTeam = 1
                addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(XDIM/2 + dx,YDIM/3 + dy,2),type = AGENT)
            else:
                self.currTeam = 2
                addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(XDIM/2 + dx,2*YDIM/3 + dy ,2),type = AGENT)
   
   #The following is run when the Save button is pressed
    def save_rtneat(self, location = "../rtneat.gnm"):
        import os
        global rtneat, rtneat2
        location = os.path.relpath("/") + location
        rtneat.save_population(location)
        
        #rtneat2.save_population("../rtneat2.gnm")

    #The following is run when the Load button is pressed
    def load_rtneat(self, location = "rtneat.gnm"):
        import os
        global rtneat, rtneat2
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            rtneat = RTNEAT(location, "data/ai/neat-params.dat", pop_size)
            #rtneat2= RTNEAT("rtneat2.gnm","data/ai/neat-params.dat", pop_size)
    
    def set_speedup(self, speedup):
        self.speedup = speedup
        if self.environment:
            self.environment.speedup = speedup
    
    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        self.weights[key] = value
        print key, value
        
    def ltChange(self,value):
        self.lt = value
        print 'lifetime:',value

    def dtaChange(self,value):
        self.dta = value
        print 'Distance to approach A:',value

    def dtbChange(self,value):
        self.dtb = value
        print 'Distance to approach B:',value

    def dtcChange(self,value):
        self.dtc = value
        print 'Distance to approach C:',value

    def ffChange(self,value):
        self.ff = value
        print 'Friendly fire:',value

    def eeChange(self,value):
        self.ee = value
        print 'Explore/exploit:',value

    def hpChange(self,value):
        self.hp = value
        print 'Hit points:',value

gMod = None

read = None

def getReader():
    global read
    global subp
    if not read:
        subp = subprocess.Popen(['python', 'NERO/menu.py'],stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.PIPE)
        read = subp.stdout
    return read

def getSubProcess():
    global read
    global subp
    if not read:
        subp = subprocess.Popen(['python', 'NERO/menu.py'],stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.PIPE)
        read = subp.stdout
    return subp

def readerData():
    r = getReader()
    p = getSubProcess()
    #if not p.poll(): return False
    import select
    return select.select([r],[],[],0) == ([r],[],[])

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod

def parseInput(strn):
    print "PARSING:", strn
    print strn.isupper()
    mod = getMod()
    loc,val = strn.split(' ')
    vali = 1
    if strn.isupper(): vali = int(val)
    if loc == "SG": mod.set_weight("sg",vali)
    if loc == "ST": mod.set_weight("st",vali)
    if loc == "TD": mod.dtaChange(vali)
    if loc == "AE": mod.set_weight("ae",vali)
    if loc == "ED": mod.dtbChange(vali)
    if loc == "AF": mod.set_weight("af",vali) 
    if loc == "FD": mod.dtcChange(vali)
    if loc == "HT": mod.set_weight("ht",vali)
    if loc == "FF": mod.ffChange(vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save": mod.save_rtneat(val)
    if loc == "load": mod.load_rtneat(val)

def ServerMain():
    print "Starting mod NERO"
