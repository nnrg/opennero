import time
from math import *
from OpenNero import *
from NERO.module import *

MAX_SPEED = 10
MAX_SD = 100
MAX_ACC = 1000000
OBSTACLE = 1#b0001
AGENT = 2#b0010

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.id = -1
        self.position = (0, 0)
        self.prev_position = (0, 0, 0)
        self.initial_position = Vector3f(0, 0, 0)
        self.initial_rotation = Vector3f(0, 0, 0)
        self.time = time.time()
        self.start_time = self.time
        self.total_damage = 0
        self.curr_damage = 0
        self.curr_fitness = 0
        self.prev_fitness = 0
        self.team = 0
        self.sg = 0
        self.st = 0
        self.ae = 0
        self.ag = 0
        self.ht = 0
        self.vg = 0





class NeroEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self, XDIM, YDIM):
        from NERO.module import getMod
        """
        Create the environment
        """

        print "CREATING NERO ENVIRONMENT: " + str(dir(module))
        Environment.__init__(self) 
        
        self.curr_id = 0
        self.step_size = 0.1 # time between steps in seconds
        self.max_steps = 20
        self.time = time.time()
        self.XDIM = XDIM
        self.YDIM = YDIM
        self.MAX_DIST = pow((pow(XDIM,2) + pow(YDIM,2)),.5)
        self.states = {}
        self.teams = {}
        self.flag_loc = getMod().flag_loc
        self.pop_state = {}

        abound = FeatureVectorInfo() # actions
        sbound = FeatureVectorInfo() # sensors
        rbound = FeatureVectorInfo() # rewards
        
        # actions
        abound.add_continuous(-pi/2, pi/2) # direction of motion
        abound.add_continuous(0,1) # how fast to move
        abound.add_continuous(-pi/2,pi/2) # Firing direction
        abound.add_continuous(0,1)
        abound.add_continuous(0,1) 

         #sensors
        sbound.add_continuous(0,1) # Straight Ahead
        
        sbound.add_continuous(0,1) # +.25 Rads
        sbound.add_continuous(0,1) # +.50 Rads
        sbound.add_continuous(0,1) # +.75 Rads
        sbound.add_continuous(0,1) # +1.0 Rads

        sbound.add_continuous(0,1) # -.25 Rads
        sbound.add_continuous(0,1) # -.50 Rads
        sbound.add_continuous(0,1) # -.75 Rads
        sbound.add_continuous(0,1) # -1.0 Rads
        
        sbound.add_continuous(0,self.MAX_DIST) # Flag Sensors - Dist
        sbound.add_continuous(-1 * pi * 2, pi * 2) # Flag Sensors - Heading
        
        sbound.add_continuous(0,self.MAX_DIST) # Ally Sensors - Dist
        sbound.add_continuous(-1 * pi * 2, pi * 2) # Ally Sensors - Heading
        
        sbound.add_continuous(0,self.MAX_DIST) # Enemy Sensors - Dist
        sbound.add_continuous(-1 * pi * 2, pi * 2) # Enemy Sensors - Heading
        
        #rewards
#        rbound.add_continuous(-100,100) # range for reward
        
        self.agent_info = AgentInitInfo(sbound, abound, rbound)
    
    def out_of_bounds(self, pos):
        return pos.x < 0 or pos.y < 0 or pos.x > self.XDIM or pos.y > self.YDIM
    
    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        state = self.get_state(agent)
        agent.state.position = state.initial_position
        agent.state.rotation = state.initial_rotation
        state.position = (state.initial_position.x, state.initial_position.y)
        state.prev_position = state.position
        state.time = time.time()
        state.start_time = state.time
        state.total_damage = 0
        state.curr_damage = 0
        state.prev_fitness = state.curr_fitness
        state.curr_fitness = 0
        state.team = agent.getTeam()
        
        #update client fitness
        from client import set_stat
        ff = self.getFriendFoe(agent)
        avg = 0
        for x in ff[0]:
            avg += x.prev_fitness
        avg /= len(ff[0])

        set_stat(avg,state.team)
        
        print "Episode %d complete" % agent.episode 
        return True
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        return self.agent_info
   
    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            print str(dir(agent.org))
            self.states[agent] = AgentState()
            self.states[agent].id = agent.state.id
            myTeam = agent.getTeam()
            self.states[agent].team = myTeam
            if myTeam in self.teams:
                self.teams[myTeam].append(self.states[agent])
            else:
                self.teams[myTeam] = []
            return self.states[agent]

    def getStateId(self, id):
        for state in self.states:
            if id == self.states[state].id:
                return self.states[state]
        else:
            return -1
            
    def getFriendFoe(self, agent):
        friend = []
        foe = []
        astate = self.get_state(agent)
        myTeam = astate.team
        friend = self.teams[myTeam]
        for r in self.teams:
            if r != myTeam:
                foe = self.teams[r] #TODO MAKE THIS VIABLE OVER 3+ TEAMS
        return (friend,foe)

    def step(self, agent, action):
        """
        A step for an agent
        """
        from NERO.module import getMod
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))

        print action

        state = self.get_state(agent)
        if agent.step == 0:
            state.initial_position = agent.state.position
            state.initial_rotation = agent.state.rotation
            self.pop_state[agent.org.id] = state 

        # Update Damage totals
        state.total_damage += state.curr_damage
        damage = state.curr_damage
        state.curr_damage = 0
        
        #Fitness Function Parameters
        stand_ground = getMod().sg
        stick_together = getMod().st
        approach_enemy = getMod().ae
        approach_flag = getMod().af
        hit_target = getMod().ht
        avoid_fire = getMod().vf
        distance_st = getMod().dta
        distance_ae = getMod().dtb
        distance_af = getMod().dtc
        friendly_fire = getMod().ff
        
        position = agent.state.position
        rotation = agent.state.rotation

        rotation.z += action[1]
        agent.state.rotation.z = action[1] + agent.state.rotation.z
        heading = rotation.z

        position.x += MAX_SPEED * cos(heading) * action[0]
        position.y += MAX_SPEED * sin(heading) * action[0]


        firing = agent.state.position
        firing.x += self.MAX_DIST * cos(action[2] + heading)
        firing.y += self.MAX_DIST * sin(action[2] + heading)
        
        safe = True
        mo = self.raySense(agent,heading,action[0] * MAX_SPEED,OBSTACLE + AGENT)
        if mo < 0.5:
            safe = False

        if safe:
         self.update_observer((position.x, position.y, radians(rotation.z)))
        else:
            position = agent.state.position
        p0 = agent.state.position
        p1 = firing
        data = getSimContext().findInRay(p0,p1,AGENT + OBSTACLE,True)
        
        string = agent.state.label + str(len(data)) +": "
        
        hit = 0
        
        if len(data) > 0:
            sim = data[0]
            string += str(sim.label) + "," + str(sim.id) + ";"
            target = self.getStateId(sim.id)
            if target != -1:
                if target.team == state.team:
                    target.curr_damage += 1 * friendly_fire
                else:
                    target.curr_damage += 1
                    hit = 1

        agent.state.position = position
        agent.state.rotation = rotation

        state.position = (position.x,position.y)

        ffr = self.getFriendFoe(agent)
        if ffr[0] == []:
            return 0 #Corner Case
        ff= []
        ff.append(self.nearest(state.position,state.id,ffr[0]))
        ff.append(self.nearest(state.position,state.id,ffr[1]))
        
        #calculate f
        #sg = -action[0]
        #st = self.distance(ff[0].position,state.position)
        #ae = self.distance(ff[1].position,state.position)
        #af = (distance_af/self.flag_distance(agent))
        #ht = h
        #vf = -damage
        
        
        #update current self data with f's
        #self.sg += sg
        #self.st += st
        #self.ae += ae
        #self.af += af
        #self.ht += ht
        #self.vg += vf

        
        #Update Fitness Function with 
        stand_ground   *= -action[0]
        if not safe:
            stand_ground = 0
        if ff[0] == 1:
            stick_together = 0
        else:
         stemp = self.distance(ff[0].position,state.position)
         if stemp > 0:
          stick_together *= min(distance_st/stemp,1)
         else:
          stick_together = 0
        if ff[1] == 1:
            approach_enemy = 0
        else:
         atemp = self.distance(ff[1].position,state.position)
         if atemp > 0:
          approach_enemy *= min(distance_ae/atemp,1)
         else:
          approach_enemy = 0
        approach_flag  *= (distance_af/self.flag_distance(agent))
        hit_target     *= hit
        avoid_fire     *= -damage
        
     
        #print "sg: " + str(stand_ground) + " st: " + str(stick_together) + "
        #ae: " + str(approach_enemy) + " af: " + str(approach_flag) + " ht: " +
        #str(hit_target) +  " vf: " + str(avoid_fire)
        state.curr_fitness += stand_ground + stick_together + approach_enemy + approach_flag + hit_target + avoid_fire

        #state.curr_fitness = 0
        #return 0
        return stand_ground + stick_together + approach_enemy + approach_flag + hit_target + avoid_fire
    
  
    def raySense (self, agent, heading_mod, dist, types = 0, boo = False):
        state = self.get_state(agent)
        firing = agent.state.position
        rotation = agent.state.rotation
        heading = rotation.z + heading_mod
        firing.x += dist * cos(heading)
        firing.y += dist * sin(heading)
        p0 = agent.state.position
        p1 = firing
        result = getSimContext().findInRay(p0,p1,types)
        if len(result) > 0:
            (sim, hit) = result
            ray = p1 - p0
            len_ray = ray.getLength()
            data = hit - p0
            len_data = data.getLength()
            if len_ray != 0:
                return len_data / len_ray
        return 1

    def sense(self, agent):
        """ figure out what the agent should sense """
        v = self.agent_info.sensors.get_instance()
        vx = []
        vx.append(self.raySense(agent,0,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,.25,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,.5,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,.75,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,1,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,-.25,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,-.5,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,-.75,MAX_SD,OBSTACLE))
        vx.append(self.raySense(agent,-1,MAX_SD,OBSTACLE))

        for iter in range(len(vx)):
            v[iter] = vx[iter]

        state = self.get_state(agent)
        
        v[9] = self.flag_distance(agent)
        v[10] = sin((state.position[1] - self.flag_loc.y) / v[9])

        # TODO: make faster?
        ffr = self.getFriendFoe(agent)
        if (ffr[0] == []):
            return v

        ff = []
        # TODO: make faster?
        ff.append(self.nearest(state.position,state.id,ffr[0]))
        ff.append(self.nearest(state.position,state.id,ffr[1]))
        if ff[0] == 1:
            return v
        v[11] = self.distance(ff[0].position,state.position)
        v[12] = self.angle(state.position,ff[0].position)
        v[13] = self.distance(state.position,ff[1].position)
        v[14] = self.angle(state.position,ff[1].position)
        return v
    
    def flag_distance(self, agent):
        pos = self.get_state(agent).position
        return pow(pow(float(pos[0]) - self.flag_loc.x, 2) + pow(float(pos[1]) - self.flag_loc.y,2),.5)

    def distance(self,agloc,tgloc):
        return pow(pow(float(agloc[0] - tgloc[0]), 2) + pow(float(agloc[1] - tgloc[1]), 2),.5)

    def angle(self,agloc,tgloc):
        if(agloc[1] == tgloc[1]):
            return 0
        return tan((agloc[0]-tgloc[0])/ (agloc[1]-tgloc[1]))

    def nearest(self,cloc,id,array):
        nearest = 1
        value = self.MAX_DIST * 5
        for other in array:
            if id == other.id:
                continue
            if self.distance(cloc,other.position) < value:
                nearest = other
                value - self.distance(cloc,other.position)
        return nearest


    def is_active(self, agent):
        """ return true when the agent should act """
        state = self.get_state(agent)
        if time.time() - state.time > self.step_size:
            state.time = time.time()
            return True
        else:
            return False
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        from NERO.module import getMod
        self.max_steps = getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return True
        else:
            return False
    
    def update_observer(self, pose):
        return 1 
   
    def cleanup(self):
        """
        cleanup the world
        """
        return True
