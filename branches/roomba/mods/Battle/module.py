from common import *
from OpenNero import getSimContext
from Battle.NeroEnvironment import *
from Battle.RTNEATAgent import *
from Battle.Turret import *
from Battle.constants import *
import subprocess
import os
import sys

import random

class NeroModule:
    def __init__(self):
        global rtneat, rtneat2
        rtneat = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS + 1, NEAT_ACTIONS, pop_size, 1.0)
        rtneat2 = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS + 1, NEAT_ACTIONS, pop_size,1.0)
        self.XDIM = XDIM
        self.YDIM = YDIM
        self.NEAT_ACTIONS = NEAT_ACTIONS
        self.NEAT_SENSORS = NEAT_SENSORS
        self.environment = None
        self.agent_id = None
        self.agent_map = {}
        self.lt = 0
        self.dta = 50
        self.dtb = 50
        self.dtc = 50
        self.ff =  0
        self.ee =  50
        self.hp = 1
        self.currTeam = 1
        #self.flag_loc = Vector3f(20,20,0)
        self.flag_loc = Vector3f(0,0,0)
        self.flag_id = -1
        self.num_to_add = 0 #pop_size

    def setup_map(self):
        """
        setup the test environment
        """
        global XDIM, YDIM, HEIGHT, OFFSET
        if self.environment:
            error("Environment already created")
            return
        
        startScript("Battle/menu.py")
        self.environment = NeroEnvironment(XDIM, YDIM)

        set_environment(self.environment)
        
        # flag placement
        self.flag_id = addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag")


        # world walls
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2,0,HEIGHT+OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall0", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(0, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall1", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall2", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM, HEIGHT +OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall3", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1, YDIM,HEIGHT), label="World Wall4", type = OBSTACLE)

        # Add the surrounding Environment
        addObject("data/terrain/NeroWorld.xml", Vector3f(XDIM/2, YDIM/2, 0), scale=Vector3f(1, 1, 1), label="NeroWorld")

    def change_flag(self, new_loc):
        if 1 == 1: return
        self.flag_loc = Vector3f(new_loc[0],new_loc[1],new_loc[2])
        print self.flag_id
        removeObject(self.flag_id)
        self.flag_id = addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag")

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        global rtneat, rtneat2
        disable_ai()

        # Create RTNEAT Objects
        set_ai("neat1",rtneat)
        set_ai("neat2", rtneat2)
        enable_ai()

        #while self.getNumToAdd() > 0:
        dx = random.randrange(XDIM/20) - XDIM/40
        dy = random.randrange(XDIM/20) - XDIM/40
        #self.addAgent((XDIM/2 + dx, YDIM/3 + dy, 2))
        for i in range(0, DEPLOY_SIZE):
            dx = random.randrange(XDIM/20) - XDIM/40
            dy = random.randrange(XDIM/20) - XDIM/40
            id = None
            if i % 2 == 0:
                self.currTeam = 1
                id = addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(XDIM/2 + dx,YDIM/3 + dy,2),type = AGENT)
            else:
                self.currTeam = 2
                id = addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(XDIM/2 + dx,2*YDIM/3 + dy ,2),type = AGENT)
            self.agent_map[(0,i)] = id

    #The following is run when the Load button is pressed
    def load_rtneat(self, val, location = "rtneat.gnm"):
        import os
        global rtneat, rtneat2
        location = str(os.path.relpath("/") + location)
        if os.path.exists(location):
            if val ==  1: rtneat = RTNEAT(location, "data/ai/neat-params.dat", pop_size)
            if val ==  2: rtneat2= RTNEAT(location, "data/ai/neat-params.dat", pop_size)
    
    def set_speedup(self, speedup):
        self.speedup = speedup
        if self.environment:
            self.environment.speedup = speedup
            
    def eeChange(self,value):
        self.ee = value
        print 'Explore/exploit:',value

    def getNumToAdd(self):
        return self.num_to_add

    def addAgent(self,pos):
        self.num_to_add -= 1
        self.currTeam += 1
        #self.currTeam = 1
        if self.currTeam == 3: self.currTeam = 1
        addObject("data/shapes/character/SydneyRTNEAT.xml",Vector3f(pos[0],pos[1] * self.currTeam,pos[2]),type = AGENT)

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod

def parseInput(strn):
    from Battle.client import toggle_ai_callback
    strn = str(strn)
    mod = getMod()
    loc,val = strn.split(' ')
    vali = 1
    if strn.isupper(): vali = int(val)
    if loc == "EE": mod.eeChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "load1": mod.load_rtneat(1,val)
    if loc == "load2": mod.load_rtneat(2,val)
    if loc == "deploy": toggle_ai_callback()
