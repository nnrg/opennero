from common import *
from OpenNero import getSimContext
from NERO.NeroEnvironment import *
from NERO.RTNEATAgent import *
from NERO.Turret import *
from constants import *
import subprocess
import os
import sys

import random

class NeroModule:
    def __init__(self):
        global rtneat
        # initialize the rtNEAT algorithm parameters
        # input layer has enough nodes for all the observations plus a bias
        # output layer has enough values for all the actions
        # population size matches ours
        # 1.0 is the weight initialization noise
        rtneat = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS + 1, NEAT_ACTIONS, pop_size, 1.0)
        self.environment = None
        self.agent_id = None
        self.weights = Fitness()
        self.lt = 10
        self.dta = 50
        self.dtb = 50
        self.dtc = 50
        self.ff =  0
        self.ee =  0
        self.hp = 50
        self.flag_loc = Vector3f(0,0,0)
        self.flag_id = -1
        self.num_to_add = pop_size

    def setup_map(self):
        """
        setup the test environment
        """
        global XDIM, YDIM, HEIGHT, OFFSET
        if self.environment:
            error("Environment already created")
            return
        
        #Test for wx python
        try:
            import wx
        except ImportError:
            print "Please install wx"
            openWiki('wx')()
            return False

        startScript('NERO/menu.py')
        
        self.environment = NeroEnvironment()

        set_environment(self.environment)
        
        # flag placement
        self.flag_id = addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag", type = OBJECT_TYPE_FLAG)

        # world walls
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2,0,HEIGHT+OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall0", type = OBJECT_TYPE_OBSTACLE  )
        addObject("data/shapes/cube/Cube.xml", Vector3f(0, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall1", type = OBJECT_TYPE_OBSTACLE  )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall2", type = OBJECT_TYPE_OBSTACLE  )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM, HEIGHT +OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall3", type = OBJECT_TYPE_OBSTACLE  )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1, YDIM,HEIGHT), label="World Wall4", type = OBJECT_TYPE_OBSTACLE )

        # Add the surrounding Environment
        addObject("data/terrain/NeroWorld.xml", Vector3f(XDIM/2, YDIM/2, 0), scale=Vector3f(1, 1, 1), label="NeroWorld", type = OBJECT_TYPE_LEVEL_GEOM)
        
        return True

    def change_flag(self, new_loc):
        self.flag_loc = Vector3f(new_loc[0],new_loc[1],new_loc[2])
        
        removeObject(self.flag_id)
        
        self.flag_id = addObject("data/shapes/cube/BlueCube.xml", self.flag_loc, label="Flag", type = OBJECT_TYPE_FLAG)

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        global rtneat
        disable_ai()
        # Create RTNEAT Objects
        set_ai("rtneat",rtneat)
        enable_ai()
        # Generate all initial rtNEAT Agents
        dx = random.randrange(XDIM/20) - XDIM/40
        dy = random.randrange(XDIM/20) - XDIM/40
        dx = random.randrange(XDIM/20) - XDIM/40
        dy = random.randrange(XDIM/20) - XDIM/40
        id = addObject("data/shapes/character/steve_red_armed.xml",Vector3f(XDIM/2 + dx,YDIM/3 + dy,2),type = OBJECT_TYPE_AGENT)
        self.num_to_add -= 1

   #The following is run when the Save button is pressed

    def save_rtneat(self, location, pop):
        import os
        location = os.path.relpath("/") + location
        global rtneat
        rtneat.save_population(str(location))

    #The following is run when the Load button is pressed
    def load_rtneat(self, location , pop):
        import os
        global rtneat
        location = os.path.relpath("/") + location
        if os.path.exists(location):
            rtneat = RTNEAT(str(location), "data/ai/neat-params.dat", pop_size)
    
    def set_speedup(self, speedup):
        self.speedup = speedup
        if self.environment:
            self.environment.speedup = speedup
    
    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        self.weights[key] = (value-100.0)/100.0
        self.environment.clear_averages()
        print key, value
        
    def ltChange(self,value):
        self.lt = value
        self.environment.clear_averages()
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
        self.environment.clear_averages()
        print 'Hit points:',value

    def getNumToAdd(self):
        return self.num_to_add

    #This is the function ran when an agent already in the field causes the generation of a new agent
    def addAgent(self,pos):
        self.num_to_add -= 1
        addObject("data/shapes/character/steve_red_armed.xml",Vector3f(pos[0],pos[1],pos[2]),type = OBJECT_TYPE_AGENT)

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
    if strn == "deploy": return
    if len(strn) < 2: return
    mod = getMod()
    loc,val = strn.split(' ')
    vali = 1
    if strn.isupper(): vali = int(val)
    if loc == "SG": mod.set_weight(Fitness.STAND_GROUND,vali)
    if loc == "ST": mod.set_weight(Fitness.STICK_TOGETHER,vali)
    if loc == "TD": mod.dtaChange(vali)
    if loc == "AE": mod.set_weight(Fitness.APPROACH_ENEMY,vali)
    if loc == "ED": mod.dtbChange(vali)
    if loc == "AF": mod.set_weight(Fitness.APPROACH_FLAG,vali) 
    if loc == "FD": mod.dtcChange(vali)
    if loc == "HT": mod.set_weight(Fitness.HIT_TARGET,vali)
    if loc == "LT": mod.ltChange(vali)
    if loc == "FF": mod.ffChange(vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_rtneat(val,1)
    if loc == "load1": mod.load_rtneat(val,1)
    if loc == "save2": mod.save_rtneat(val,2)
    if loc == "load2": mod.load_rtneat(val,2)

def ServerMain():
    print "Starting mod NERO"
