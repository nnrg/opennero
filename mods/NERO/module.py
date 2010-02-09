from common import *
from OpenNero import getSimContext
from NERO.NeroEnvironment import *
from NERO.RTNEATAgent import *
from NERO.Turret import *

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
        self.sg =  0
        self.st =  0
        self.ae =  1
        self.af =  0
        self.ht =  0
        self.vf =  0
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
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2,0,HEIGHT+OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall0", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(0, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall1", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall2", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM, HEIGHT +OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall3", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1, YDIM,HEIGHT), label="World Wall4", type = OBSTACLE)

        # Add the surrounding Environment
        addObject("data/terrain/WaynesWorld.xml", Vector3f(XDIM/2, YDIM/2, 0), scale=Vector3f(1, 1, 1), label="WaynesWorld", type = OBSTACLE)

        # add trees
        addObject("data/shapes/tree/Tree.xml", Vector3f(120, 10, 0), label="Tree0", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(783, 7, 0), label="Tree1", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(96, 31, 0), label="Tree2", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(746, 35, 0), label="Tree3", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(91, 48, 0), label="Tree4", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(687, 46, 0), label="Tree5", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(26, 71, 0), label="Tree6", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(691, 72, 0), label="Tree7", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(45, 87, 0), label="Tree8", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(772, 94, 0), label="Tree9", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(109, 116, 0), label="Tree10", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(678, 110, 0), label="Tree11", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(66, 134, 0), label="Tree12", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(700, 130, 0), label="Tree13", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(71, 148, 0), label="Tree14", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(775, 150, 0), label="Tree15", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(14, 174, 0), label="Tree16", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(673, 169, 0), label="Tree17", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(98, 188, 0), label="Tree18", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(767, 189, 0), label="Tree19", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(62, 215, 0), label="Tree20", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(761, 209, 0), label="Tree21", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(120, 230, 0), label="Tree22", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(786, 226, 0), label="Tree23", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(123, 250, 0), label="Tree24", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(760, 252, 0), label="Tree25", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(102, 275, 0), label="Tree26", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(751, 265, 0), label="Tree27", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(54, 296, 0), label="Tree28", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(724, 285, 0), label="Tree29", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(37, 311, 0), label="Tree30", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(788, 308, 0), label="Tree31", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(20, 328, 0), label="Tree32", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(714, 325, 0), label="Tree33", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(99, 348, 0), label="Tree34", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(730, 351, 0), label="Tree35", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(28, 371, 0), label="Tree36", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(679, 366, 0),label="Tree37", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(86, 393, 0), label="Tree38", type = OBSTACLE)
        addObject("data/shapes/tree/Tree.xml", Vector3f(778, 389, 0), label="Tree39", type = OBSTACLE)


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
    def save_rtneat(self):
        global rtneat, rtneat2
        rtneat.save_population("../rtneat.gnm")
        rtneat2.save_population("../rtneat2.gnm")

    #The following is run when the Load button is pressed
    def load_rtneat(self):
        import os
        global rtneat, rtneat2
        if os.path.exists("rtneat.gnm") and os.path.exists("rtneat2.gnm"):
            rtneat = RTNEAT("rtneat.gnm", "data/ai/neat-params.dat", pop_size)
            rtneat2= RTNEAT("rtneat2.gnm","data/ai/neat-params.dat", pop_size)

    #The following functions are used to let the client update the fitness function
    def sgChange(self,value):
        self.sg = value

    def stChange(self,value):
        self.st = value

    def aeChange(self,value):
        self.ae = value

    def afChange(self,value):
        self.af = value

    def htChange(self,value):
        self.ht = value

    def vfChange(self,value):
        self.vf = value

    def ltChange(self,value):
        self.lt = value

    def dtaChange(self,value):
        self.dta = value

    def dtbChange(self,value):
        self.dtb = value

    def dtcChange(self,value):
        self.dtc = value

    def ffChange(self,value):
        self.ff = value

    def eeChange(self,value):
        self.ee = value

    def hpChange(self,value):
        self.hp = value

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod


def ServerMain():
    print "Starting mod NERO"
