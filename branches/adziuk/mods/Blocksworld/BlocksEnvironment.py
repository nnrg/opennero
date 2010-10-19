import time
from math import *
from OpenNero import *
from Blocksworld.module import *
from constants import *
from common.fitness import Fitness, FitnessStats
from copy import copy
from random import *

MAX_SPEED = 12
MAX_SD = 100
OBSTACLE = (1 << 0)
AGENT = (1 << 0)

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.id = -1
        # current x, y, heading pose
        self.pose = (0, 0, 0)
        # previous x, y, heading pose
        self.prev_pose = (0, 0, 0)
        # starting position
        self.initial_position = Vector3f(0, 0, 0)
        # starting orientation
        self.initial_rotation = Vector3f(0, 0, 0)        
        self.time = time.time()
        self.start_time = self.time
        self.total_damage = 0
        self.curr_damage = 0
        self.team = 0
        self.fitness = Fitness()
        self.prev_fitness = Fitness()
        self.final_fitness = 0
        self.animation = 'stand'
        self.prev_collected = []

class BlocksEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        from Blocksworld.module import getMod
        """
        Create the environment
        """
        print "CREATING NERO ENVIRONMENT: " + str(dir(module))
        Environment.__init__(self) 

        self.curr_id = 0
        self.step_delay = 0.25 # time between steps in seconds
        self.max_steps = 20
        self.time = time.time()
        self.MAX_DIST = pow((pow(XDIM, 2) + pow(YDIM, 2)), .5)
        self.states = {}
        self.teams = {}
        self.speedup = 0

        self.pop_state_1 = {}
        self.pop_state_2 = {}
        
        abound = FeatureVectorInfo() # actions
        sbound = FeatureVectorInfo() # sensors
        rbound = FeatureVectorInfo() # rewards
        
        # actions
        abound.add_continuous(0, pi / 2) # direction of motion
        abound.add_continuous(0, 1) # how fast to move
        abound.add_continuous(0, pi / 2) # direction of motion
        
        #Flag Sensors
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        """
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        sbound.add_continuous(0, 1) # Distance
        """
        self.agent_info = AgentInitInfo(sbound, abound, rbound)
    
    def out_of_bounds(self, pos):
        """
        Checks if a given position is in bounds
        """
        return pos.x < 0 or pos.y < 0 or pos.x > XDIM or pos.y > YDIM
    
    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        dx = randrange(XDIM/20) - XDIM/40
        dy = randrange(XDIM/20) - XDIM/40
                
        state = self.get_state(agent)
        state.team = agent.getTeam()
        if state.team == 1: agent.state.position = Vector3f(TEAM_1_SL_X + dx, TEAM_1_SL_Y + dy,2)
        if state.team == 2: agent.state.position = Vector3f(TEAM_2_SL_X + dx, TEAM_2_SL_Y + dy,2)
        agent.state.rotation = copy(state.initial_rotation)
        state.pose = (state.initial_position.x, state.initial_position.y, state.initial_rotation.z)
        state.prev_pose = state.pose
        state.total_damage = 0
        state.curr_damage = 0
        state.prev_fitness = state.fitness
        state.fitness = Fitness()
        #update client fitness
        from client import set_stat
        ff = self.getFriendFoe(agent)
        return True
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        return self.agent_info
   
    def get_state(self, agent):
        """
        Returns the state of an agent
        """
        if agent in self.states:
            return self.states[agent]
        else:
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
        """
        Searches for the state with the given ID
        """
        for state in self.states:
            if id == self.states[state].id:
                return self.states[state]
        else:
            return - 1
            
    def getFriendFoe(self, agent):
        """
        Returns lists of all friend agents and all foe agents.
        """
        friend = []
        foe = []
        astate = self.get_state(agent)
        myTeam = astate.team
        friend = self.teams[myTeam]
        for r in self.teams:
            if r != myTeam:
                foe = self.teams[r] #TODO MAKE THIS VIABLE OVER 3+ TEAMS
        return (friend, foe)

    def target(self, agent):
        #Get list of all targets
        ffr = self.getFriendFoe(agent)
        alt = ffr[1]#ffr[0] + ffr[1]
        if (ffr[0] == []):
            return None

        state = self.get_state(agent)
        

        #sort in order of variance from 0~2 degrees (maybe more)
        valids = []
        for curr in alt:
            fd = self.distance(state.pose,(curr.pose[0],curr.pose[1]))
            if fd != 0:
                fh  = ((degrees(atan2(curr.pose[1]-state.pose[1],curr.pose[0] - state.pose[0])) - state.pose[2]) % 360)
            else:
                fh = 0
            fh = abs(fh)
            if fh <= 2:
                valids.append((curr,fd,fh))
             
        #Valids contains (state,distance,heading to distance) pairs
        #get one that is nearest based on distance / cos(radians(degrees() * 20))
        top = None 
        top_v = 'A'

        for (curr,fd,fh) in valids:
            if top_v == 'A' or top_v > (fd / cos(radians(fh * 20))):
                top = curr
                top_v = (fd/cos(radians(fh * 20)))

        return top


    def step(self, agent, action):
        """
        2A step for an agent
        """
        from Blocksworld.module import getMod, parseInput
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        startScript('Blocksworld/menu.py')
        data = script_server.read_data()
        while data:
            parseInput(data.strip())
            data = script_server.read_data()

        state = self.get_state(agent)

        #Initilize Agent state
        if agent.step == 0:
            p = agent.state.position
            agent.state.rotation.z = randrange(360)
            r = agent.state.rotation
            
            state.initial_position = p
            state.initial_rotation = r
            
            state.pose = (p.x, p.y, r.z)
            state.prev_pose = (p.x, p.y, r.z)
            if agent.get_team() == 1: 
             self.pop_state_1[agent.org.id] = state 
            else:
             self.pop_state_2[agent.org.id] = state 

        #Spawn more agents if there are more to spawn (Staggered spawning times tend to yeild better behavior)
        if agent.step == 3:
            if getMod().getNumToAdd() > 0:
                dx = randrange(XDIM/20) - XDIM/40
                dy = randrange(XDIM/20) - XDIM/40
                if getMod().currTeam == 2:
                    XSL = TEAM_1_SL_X
                    YSL = TEAM_1_SL_Y
                else:
                    XSL = TEAM_2_SL_X
                    YSL = TEAM_2_SL_Y
                getMod().addAgent((XSL + dx, YSL + dy, 2))

        # Update Damage totals
        state.total_damage += state.curr_damage
        damage = state.curr_damage
        state.curr_damage = 0
        
        #Add current unit to pop_state
        if agent.get_team() == 1: 
             self.pop_state_1[agent.org.id] = state 
        else:
             self.pop_state_2[agent.org.id] = state 
        
        #Fitness Function Parameters
        fitness = getMod().weights
        distance_st = getMod().dta
        distance_ae = getMod().dtb
        distance_af = getMod().dtc
        friendly_fire = getMod().ff

        # the position and the rotation of the agent on-screen
        position = agent.state.position
        rotation = agent.state.rotation
        
        # get the current pose of the agent
        (x, y, heading) = state.pose
        
        # get the actions of the agent
        turn_by = degrees(action[0] * 2.0) - degrees(action[2] * 2.0)
        move_by = action[1]
        
        # figure out the new heading
        new_heading = wrap_degrees(heading, turn_by)        
        
        # figure out the new x,y location
        new_x = x + MAX_SPEED * cos(radians(new_heading)) * move_by
        new_y = y + MAX_SPEED * sin(radians(new_heading)) * move_by
        
        # figure out the firing location
        fire_x = x + self.MAX_DIST * cos(radians(new_heading))
        fire_y = y + self.MAX_DIST * sin(radians(new_heading))
        
        # see if we can move forward
        new_position = copy(position)
        new_position.x, new_position.y = new_x, new_y
        distance_ahead = self.raySense(agent, turn_by, MAX_SPEED * move_by, OBSTACLE)
        safe = (distance_ahead >= 1)
        if not safe:
            # keep the position the same if we cannot move
            new_position = agent.state.position
        
        # make the calculated motion
        position.x, position.y = state.pose[0], state.pose[1]
        agent.state.position = position
        rotation.z = new_heading
        agent.state.rotation = rotation
        state.prev_pose = state.pose
        state.pose = (new_position.x, new_position.y, rotation.z)
        state.time = time.time()

        x = 0

        #Check distance to all flags, collect as necessary
        for id in self.flag_locs():
            if id not in self.flag_teams()[agent.get_team()] and self.flag_distance(agent, id) <= CRANGE:
                loc = self.flag_locs()[id]
                for team in self.flag_teams():
                    if id in team: team.remove(id)
                self.flag_teams()[agent.get_team()].append(id)
                x += 1    
                print "FLAG GRAB: TEAM:", agent.get_team(), "ID:", id, "DISTANCE: ", self.flag_distance(agent,id)
                if agent.get_team() == 1: getMod().change_flag((TEAM_1_SL_X + 80,loc.y,0),id)
                if agent.get_team() == 2: getMod().change_flag((TEAM_2_SL_X - 80,loc.y,0),id)

        if len(self.flag_teams()[agent.get_team()]) - len(state.prev_collected) > 0:
            x -= len(self.flag_teams()[agent.get_team()]) - len(state.prev_collected)

        state.prev_collected = self.flag_teams()[agent.get_team()]

        #If it's the final state, handle clean up behaviors
        #You may get better behavior if you move this to epsiode_over
        if agent.step >= self.max_steps - 1:
            print "FITNESS:", len(self.flag_teams()[agent.get_team()])
            return len(self.flag_teams()[agent.get_team()])

        return 0
    
    def raySense(self, agent, heading_mod, dist, types=0, draw=True, foundColor = Color(255, 0, 128, 128), noneColor = Color(255, 0, 255, 255) ):
        """
        Sends out a ray to find objects via line of sight, using irrlicht.
        """
        state = self.get_state(agent)
        firing = agent.state.position
        rotation = agent.state.rotation
        heading = radians(rotation.z + heading_mod)
        firing.x += dist * cos(heading)
        firing.y += dist * sin(heading)
        p0 = agent.state.position
        p1 = firing
        result = getSimContext().findInRay(p0, p1, types, draw, foundColor, noneColor)
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
        """ 
        figure out what the agent should sense 
        """
        
        state = self.get_state(agent)
        v = self.agent_info.sensors.get_instance()
        vx = []
        
        if len(self.flag_teams()[agent.get_team()]) == len(self.flag_locs()): return v

        r = {}
        for val in range(0,180,30): r[val] = [0,0]


        for flag in self.flag_locs():
         if flag in self.flag_teams()[agent.get_team()]: continue
         fd = self.flag_distance(agent, 0)
         if fd != 0:
             fh  = ((degrees(atan2(self.flag_locs()[flag].y-state.pose[1],self.flag_locs()[flag].x - state.pose[0])) - state.pose[2]) % 360) - 180
         else:
             fh = 0
 
         if fh < 0:
             fh += 360
 
         if fh > 360:
             fh -= 360
      
         for val in r:
             if (fh - val) > 0 and (fh - val) <= 30 and (r[val][0] == 0 or r[val][0] > fd):
                 r[val][0] = fd
             if cos(radians(fh-val)) > r[val][1]:
                 r[val][1] = max(0,cos(radians(fh-val)))
       
        for val in r:
            vx.append(min(1,max(0,(self.MAX_DIST-r[val][0])/self.MAX_DIST)))
            vx.append(r[val][1])

        for iter in range(len(vx)):
            v[iter] = vx[iter]
        
        return v
   
    def flag_locs(self):
        """
        Returns the current location of the flag
        """
        from Blocksworld.module import getMod
        return getMod().flag_locs

    def flag_teams(self):
        from Blocksworld.module import getMod
        return getMod().flags

    def flag_distance(self, agent, id):
        """
        Returns the distance of the current agent from the flag
        """
        pos = self.get_state(agent).pose
        return pow(pow(float(pos[0]) - self.flag_locs()[id].x, 2) + pow(float(pos[1]) - self.flag_locs()[id].y, 2), .5)

    def distance(self, agloc, tgloc):
        """
        Returns the distance between agloc and tgloc
        """
        return pow(pow(float(agloc[0] - tgloc[0]), 2) + pow(float(agloc[1] - tgloc[1]), 2), .5)

    def angle(self, agloc, tgloc):
        """
        returns the angle between agloc and tgloc (test before using to make sure it's returning what you think it is)
        """
        if(agloc[1] == tgloc[1]):
            return 0
        (x, y, heading) = agloc
        (xt, yt, ignore) = tgloc
        # angle to target
        theading = atan2(yt - y, xt - x)
        rel_angle_to_target = theading - radians(heading)
        return rel_angle_to_target

    def nearest(self, cloc, id, array):
        """
        Returns the nearest agent in array to agent with id id at current location.
        """
        # TODO: this needs to only be computed once per tick, not per agent
        nearest = 1
        value = self.MAX_DIST * 5
        for other in array:
            if id == other.id:
                continue
            if self.distance(cloc, other.pose) < value:
                nearest = other
                value - self.distance(cloc, other.pose)
        return nearest

    def set_animation(self, agent, state, animation):
        """
        Sets current animation
        """
        if state.animation != animation:
            agent.state.setAnimation(animation)
            state.animation = animation
    
    def is_active(self, agent):
        """ return true when the agent should act """
        state = self.get_state(agent)
        # interpolate between prev_pose and pose
        (x1, y1, h1) = state.prev_pose
        (x2, y2, h2) = state.pose
        if x1 != x2 or y1 != y2:
            fraction = 1.0
            if self.get_delay() != 0:
                fraction = min(1.0, float(time.time() - state.time) / self.get_delay())
            pos = agent.state.position
            pos.x = x1 * (1 - fraction) + x2 * fraction
            pos.y = y1 * (1 - fraction) + y2 * fraction
            agent.state.position = pos
            self.set_animation(agent, state, 'run')
        else:
            self.set_animation(agent, state, 'stand')
        if time.time() - state.time > self.get_delay():
            state.time = time.time()
            return True
        else:
            return False
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        from Blocksworld.module import getMod
        self.max_steps = getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return True
        else:
            return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        killScript('Blocksworld/menu.py')
        return True

    def get_delay(self):
        """
        Set simulation delay
        """
        return self.step_delay * (1.0 - self.speedup)
