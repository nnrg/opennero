from common import *
from OpenNero import getSimContext
from Blocksworld.BlocksEnvironment import *
from Blocksworld.RTNEATAgent import *
from Blocksworld.Turret import *
from constants import *
import subprocess
import os
import sys

import random

class BlocksworldModule:
    def __init__(self):
        global rtneat, rtneat2
        rtneat = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS, NEAT_ACTIONS, pop_size, 1.0)
        rtneat2 = RTNEAT("data/ai/neat-params.dat", NEAT_SENSORS, NEAT_ACTIONS, pop_size,1.0)
        self.environment = None
        self.speedup = 100
        self.agent_id = None
        self.agent_map = {}
        self.weights = Fitness()
        self.lt = 150 * 8
        self.dta = 50
        self.dtb = 50
        self.dtc = 50
        self.ff =  0
        self.ee =  0
        self.hp = 50
        self.currTeam = 1
        self.coin_nears = [{0:0, 1:0, 2:0},{0:0,1:0,2:0}]
        self.coin_nears_ids = [{0:[], 1:[], 2:[]},{0:[],1:[],2:[]}]
        self.coin_locs = {0:Vector3f(XDIM/2,YDIM/2,0), 1:Vector3f(XDIM/2,YDIM/3,0), 2:Vector3f(XDIM/2,2*YDIM/3,0)}
        self.coin_ids = {}
        self.coins = [[0,1,2],[],[]]
        self.fitness = {1:.5,2:.5}
        self.capture = {1:.5,2:.5}
        self.curr_id = 3
        self.num_to_add = 0
        self.team_1_average = 0.0
        self.team_2_average = 0.0
        self.preprefix = "../"
        self.prefix = ["top_results/"]
        #self.prefix = ["altdynb_0/","altdynb_1/","altdynb_2/","altdynb_3/","altdynb_4/","altdynb_5/","altdynb_6/","altdynb_7/","altdynb_8/","altdynb_9/"]
        #self.prefix = ["rules_10/","rules_11/","rules_12/","rules_13/","rules_14/","rules_15/","rules_16/","rules_17/","rules_18/","rules_19/","altalt_0/","altalt_1/","altalt_2/","altalt_3/","altalt_4/","altalt_5/","altalt_6/","altalt_7/","altalt_8/","altalt_9/","altdyn_0/","altdyn_1/","altdyn_2/","altdyn_3/","altdyn_4/","altdyn_5/","altdyn_6/","altdyn_7/","altdyn_8/","altdyn_9/","fitdyn_0/","fitdyn_1/","fitdyn_2/","fitdyn_3/","fitdyn_4/","fitdyn_5/","fitdyn_6/","fitdyn_7/","fitdyn_8/","fitdyn_9/"]#["control_a/","control_b/","control_c/","control_d/","control_e/","control_f/","control_g/","control_h/","contr_i/","control_j/"]
        self.team_1_loc = ""
        self.team_2_loc = ""
        #self.team_locs = ["a_0.gnm","a_1.gnm","a_2.gnm","a_3.gnm","a_4.gnm","a_5.gnm","a_6.gnm","a_7.gnm","a_8.gnm","a_9.gnm","b_0.gnm","b_1.gnm","b_2.gnm","b_3.gnm","b_4.gnm","b_5.gnm","b_6.gnm","b_7.gnm","b_8.gnm","b_9.gnm","c_0.gnm","c_1.gnm","c_2.gnm","c_3.gnm","c_4.gnm","c_5.gnm","c_6.gnm","c_7.gnm","c_8.gnm","c_9.gnm"]
        #self.team_locs = ["pop_1_10.gnm","pop_1_20.gnm","pop_1_30.gnm","pop_1_40.gnm","pop_1_50.gnm","pop_1_60.gnm","pop_1_70.gnm","pop_1_80.gnm","pop_1_90.gnm","pop_1_100.gnm"]
        self.team_locs = ["altalt_0.gnm","altalt_1.gnm","altalt_2.gnm","altalt_3.gnm","altalt_4.gnm","altalt_5.gnm","altalt_6.gnm","altalt_7.gnm","altalt_8.gnm","altalt_9.gnm","altdyn_0.gnm","altdyn_1.gnm","altdyn_2.gnm","altdyn_3.gnm","altdyn_4.gnm","altdyn_5.gnm","altdyn_6.gnm","altdyn_7.gnm","altdyn_8.gnm","altdyn_9.gnm","altfit_0.gnm","altfit_1.gnm","altfit_2.gnm","altfit_3.gnm","altfit_4.gnm","altfit_5.gnm","altfit_6.gnm","altfit_7.gnm","altfit_8.gnm","altfit_9.gnm","dynfit_0.gnm","dynfit_1.gnm","dynfit_2.gnm","dynfit_3.gnm","dynfit_4.gnm","dynfit_5.gnm","dynfit_6.gnm","dynfit_7.gnm","dynfit_8.gnm","dynfit_9.gnm","control_0.gnm","control_1.gnm","control_2.gnm","control_3.gnm","control_4.gnm","control_5.gnm","control_6.gnm","control_7.gnm","control_8.gnm","control_9.gnm","rules_0.gnm","rules_1.gnm","rules_2.gnm","rules_3.gnm","rules_4.gnm","rules_5.gnm","rules_6.gnm","rules_7.gnm","rules_8.gnm","rules_9.gnm"]
        self.run_number = 0
    	self.run_set = False
    	#self.team_1_li = 0
        #self.team_2_li = 0
        #self.prenum = 0

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

        self.environment = BlocksEnvironment()

        set_environment(self.environment)
        
        # coin placement
        for coin in self.coin_locs:
         self.coin_ids[coin] = addObject("data/shapes/cube/BlueCube.xml", self.coin_locs[coin], label="Coin")

        # world walls
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2,0,HEIGHT+OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall0", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(0, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall1", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM, YDIM/2, HEIGHT + OFFSET), Vector3f(0, 0, 0), scale=Vector3f(1,YDIM,HEIGHT), label="World Wall2", type = OBSTACLE )
        addObject("data/shapes/cube/Cube.xml", Vector3f(XDIM/2, YDIM, HEIGHT +OFFSET), Vector3f(0, 0, 90), scale=Vector3f(1,XDIM,HEIGHT), label="World Wall3", type = OBSTACLE )

        # Add the surrounding Environment
        addObject("data/terrain/NeroWorld.xml", Vector3f(XDIM/2, YDIM/2, 0), scale=Vector3f(1, 1, 1), label="NeroWorld")

	temp = os.environ.get("RUN")
	if temp is not None:
		self.run_number = int(temp)
		self.run_set = True

        self.start_rtneat()

        return True
    
    def change_coin(self, new_loc, id, team):
        self.coin_locs[id] = Vector3f(new_loc[0],new_loc[1],new_loc[2])
        
        removeObject(self.coin_ids[id])

        if team == 0: self.coin_ids[id] = addObject("data/shapes/cube/BlueCube.xml", self.coin_locs[id], label="Coin")

        if team == 1: self.coin_ids[id] = addObject("data/shapes/cube/YellowCube.xml", self.coin_locs[id], label="Coin")

        if team == 2: self.coin_ids[id] = addObject("data/shapes/cube/GreenCube.xml", self.coin_locs[id], label="Coin")
   
    def reset_coin(self):
        self.change_coin((XDIM/2,YDIM/2,0),0,0)
        self.change_coin((XDIM/2,YDIM/3,0),1,0)
        self.change_coin((XDIM/2,2*YDIM/3,0),2,0)
        self.coin_nears = [{0:0, 1:0, 2:0},{0:0,1:0,2:0}]
        self.coin_nears_ids = [{0:[], 1:[], 2:[]},{0:[],1:[],2:[]}]
        self.coin_locs = {0:Vector3f(XDIM/2,YDIM/2,0), 1:Vector3f(XDIM/2,YDIM/3,0), 2:Vector3f(XDIM/2,2*YDIM/3,0)}
        self.coins = [[0,1,2],[],[]]
    
    def get_next_1(self,team_1_li,prenum):
	print "prenum:",prenum,"team_1_li", team_1_li
        string =  self.preprefix + self.prefix[prenum] + self.team_locs[team_1_li]
        return string

    def get_next_2(self,team_2_li,prenum):
        string =  self.preprefix + self.prefix[prenum] + self.team_locs[team_2_li]
        return string

    def add_coin(self, new_loc):
        
        self.coin_locs[self.curr_id] = (Vector3f(new_loc[0],new_loc[1],new_loc[2]))
        
        self.coin_ids[self.curr_id] = addObject("data/shapes/cube/BlueCube.xml", self.coin_locs[self.curr_id], label="Coin")

        self.coin_nears[0][self.curr_id] = 0
        self.coin_nears[1][self.curr_id] = 0
        self.coin_nears_ids[0][self.curr_id] = []
        self.coin_nears_ids[1][self.curr_id] = []

        self.coins[0].append(self.curr_id)

        self.curr_id += 1

    #The following is run when the Deploy button is pressed
    def start_rtneat(self):
        """ start the rtneat learning stuff"""
        global rtneat, rtneat2
        disable_ai()

        # Create RTNEAT Objects
        set_ai("neat1",rtneat)
        set_ai("neat2", rtneat2)
		
    	rn = getMod().run_number
    	lt = len(getMod().team_locs) #length of teams folder
    	lt = len(getMod().team_locs) #length of teams folder
    	ld = len(getMod().prefix) #length of not teams folder
    	prenum = rn / (lt*lt)
    	team_2_li = (rn - prenum*lt*lt)/lt
    	team_1_li = rn-prenum*lt*lt-team_2_li*lt
        
        self.load_rtneat(self.preprefix + self.prefix[prenum] + self.team_locs[team_1_li],1)
        self.load_rtneat(self.preprefix + self.prefix[prenum] + self.team_locs[team_2_li],2)
        
        enable_ai()
        # Generate all initial rtNEAT Agents
        dx = random.randrange(XDIM/20) - XDIM/40
        dy = random.randrange(XDIM/20) - XDIM/40
        for i in range(0, DEPLOY_SIZE):
            dx = random.randrange(XDIM/20) - XDIM/40
            dy = random.randrange(XDIM/20) - XDIM/40
            id = None
            if i % 2 == 0:
                self.currTeam = 1
                id = addObject("data/shapes/character/steve_red_armed.xml",Vector3f(TEAM_1_SL_X + dx,TEAM_1_SL_Y + dy,2),type = AGENT)
            else:
                self.currTeam = 2
                id = addObject("data/shapes/character/steve_blue_armed.xml",Vector3f(TEAM_2_SL_X + dx,TEAM_2_SL_Y + dy ,2),type = AGENT)
            self.agent_map[(0,i)] = id

   #The following is run when the Save button is pressed

    def save_rtneat(self, location, pop):
        import os
        location = os.path.relpath("/") + location
        print "LOCATION,", location
        global rtneat, rtneat2
        if pop == 1: rtneat.save_population(str(location))
        if pop == 2: rtneat2.save_population(str(location))

    #The following is run when the Load button is pressed
    def load_rtneat(self, location_orig , pop):
        import os
        global rtneat, rtneat2
        #location = os.path.relpath("/") + location_orig
        location = os.path.relpath(location_orig)
    	print "CURRENT LOCATION: ", os.getcwd()
        print "LOADING FOR POP", pop, " LOCATION:", location
        if os.path.exists(location):
            if pop == 1: 
                rtneat = RTNEAT(str(location), "data/ai/neat-params.dat", pop_size)    
                #self.team_1_loc = str(location_orig)
                set_ai("neat1",rtneat)
                print "successful load"
            if pop == 2:
                rtneat2= RTNEAT(str(location), "data/ai/neat-params.dat", pop_size)
                #self.team_2_loc = str(location_orig)
                set_ai("neat2",rtneat2)
                print "successful load"
    	else: print "WARNING PATH DOES NOT EXIST"
        

    def set_speedup(self, speedup):
        self.speedup = speedup
        if self.environment:
            self.environment.speedup = speedup
    
    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        self.weights[key] = (value-100.0)/100.0
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

    def fitChange(self,team,value):
        value = value/100.0
        self.fitness[team] = value
        print 'Team:', team, "Fitness Ratio =",value

    def capChange(self,team,value):
        value = value/100.0
        self.capture[team] = value
        print 'Team:', team, "Capture Ratio =",value


    def getNumToAdd(self):
        return self.num_to_add

    #This is the function ran when an agent already in the field causes the generation of a new agent
    def addAgent(self,pos):
        self.num_to_add -= 1
        self.currTeam += 1
        if self.currTeam == 3: self.currTeam = 1
        if self.currTeam == 1: addObject("data/shapes/character/steve_red_armed.xml",Vector3f(pos[0],pos[1],pos[2]),type = AGENT)
        if self.currTeam == 2: addObject("data/shapes/character/steve_blue_armed.xml",Vector3f(pos[0],pos[1],pos[2]),type = AGENT)

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = BlocksworldModule()
    return gMod

def parseInput(strn):
    if strn == "deploy": return
    if len(strn) < 2: return
    mod = getMod()
    loc,val = strn.split(' ')
    vali = 1
    if strn.isupper(): vali = int(val)
    if loc == "F1": mod.fitChange(1,vali)
    if loc == "F2": mod.fitChange(2,vali)
    if loc == "C1": mod.capChange(1,vali)
    if loc == "C2": mod.capChange(2,vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_rtneat(val,1)
    if loc == "load1": mod.load_rtneat(val,1)
    if loc == "save2": mod.save_rtneat(val,2)
    if loc == "load2": mod.load_rtneat(val,2)

def ServerMain():
    print "Starting mod Blocksworld"
