import time
from math import *
from OpenNero import *
from CaptureTheFlag.module import *
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
        self.time = time()
        self.start_time = self.time
        self.total_damage = 0
        self.curr_damage = 0
        self.team = 0
        self.fitness = Fitness()
        self.prev_fitness = Fitness()
        self.final_fitness = 0
        self.animation = 'stand'

class NeroEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        from CaptureTheFlag.module import getMod
        """
        Create the environment
        """
        print "CREATING NERO ENVIRONMENT: " + str(dir(module))
        Environment.__init__(self) 

        self.curr_id = 0
        self.step_delay = 0.25 # time between steps in seconds
        self.max_steps = 20
        self.time = time()
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
        #abound.add_continuous(-pi / 2, pi / 2) # Firing direction
        #abound.add_continuous(0, 1)
        #abound.add_continuous(0, 1) 
        
        #Wall Sensors
        sbound.add_continuous(0, 1) # -60 deg        
        #sbound.add_continuous(0, 1) # -45 deg
        sbound.add_continuous(0, 1) # -30 deg
        #sbound.add_continuous(0, 1) # -15 deg
        sbound.add_continuous(0, 1) # straight ahead
        #sbound.add_continuous(0, 1) # 15 deg
        sbound.add_continuous(0, 1) # 30 deg
        #sbound.add_continuous(0, 1) # 45 deg
        sbound.add_continuous(0, 1) # 60 deg
        
        #Foe Sensors
        sbound.add_continuous(0, 1) # -60 deg        
        #sbound.add_continuous(0, 1) # -45 deg
        sbound.add_continuous(0, 1) # -30 deg
        #sbound.add_continuous(0, 1) # -15 deg
        sbound.add_continuous(0, 1) # straight ahead
        #sbound.add_continuous(0, 1) # 15 deg
        sbound.add_continuous(0, 1) # 30 deg
        #sbound.add_continuous(0, 1) # 45 deg
        sbound.add_continuous(0, 1) # 60 deg
        
        #Friend Sensors
        sbound.add_continuous(0, 1) # -60 deg        
        #sbound.add_continuous(0, 1) # -45 deg
        sbound.add_continuous(0, 1) # -30 deg
        #sbound.add_continuous(0, 1) # -15 deg
        sbound.add_continuous(0, 1) # straight ahead
        #sbound.add_continuous(0, 1) # 15 deg
        sbound.add_continuous(0, 1) # 30 deg
        #sbound.add_continuous(0, 1) # 45 deg
        sbound.add_continuous(0, 1) # 60 deg

        #Flag Sensors
        sbound.add_continuous(0, 1) # 0 - 45
        #sbound.add_continuous(0, 1) # 45 - 90
        sbound.add_continuous(0, 1) # 90 - 135
        #sbound.add_continuous(0, 1) # 135 - 180
        sbound.add_continuous(0, 1) # 180 - 225
        #sbound.add_continuous(0, 1) # 225 - 270
        sbound.add_continuous(0, 1) # 270 - 315
        #sbound.add_continuous(0, 1) # 315 - 360
        sbound.add_continuous(0, 1) # Distance
        
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
        state = self.get_state(agent)
        agent.state.position = copy(state.initial_position)
        agent.state.rotation = copy(state.initial_rotation)
        state.pose = (state.initial_position.x, state.initial_position.y, state.initial_rotation.z)
        state.prev_pose = state.pose
        state.total_damage = 0
        state.curr_damage = 0
        state.team = agent.getTeam()
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
        from CaptureTheFlag.module import getMod, parseInput
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        startScript('CaptureTheFlag/menu.py')
        data = script_server.read_data()
        while data:
            parseInput(data.strip())
            data = script_server.read_data()

        state = self.get_state(agent)

        #Initilize Agent state
        if agent.step == 0:
            temp = getMod().flag_loc
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
                getMod().addAgent((XDIM/2 + dx, YDIM/3 + dy, 2))

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
        
        # draw the line of fire
        fire_pos = copy(position)
        fire_pos.x, fire_pos.y = fire_x, fire_y
        
        # calculate if we hit anyone
        data = self.target(agent)
        #string = agent.state.label + str(len(data)) + ": "
        hit = 0
        if data != None:#len(data) > 0:
            sim = data
            #string += str(sim.label) + "," + str(sim.id) + ";"
            target = self.getStateId(sim.id)
            if target != -1:
                if target.team == state.team:
                    target.curr_damage += 1 * friendly_fire
                else:
                    data = getSimContext().findInRay(position, Vector3f(sim.pose[0],sim.pose[1],2), AGENT + OBSTACLE, True)
                    if not(len(data) > 0 and data[0] != target):
                        target.curr_damage += 1
                        hit = 1
        
        # calculate friend/foe
        ffr = self.getFriendFoe(agent)
        if ffr[0] == []:
            return 0 #Corner Case
        ff = []
        ff.append(self.nearest(state.pose, state.id, ffr[0]))
        ff.append(self.nearest(state.pose, state.id, ffr[1]))
        
        st = 0
        ae = 0
        
        #calculate fitness
        sg = -action[0]
        if ff[0] != 1 and self.distance(ff[0].pose,state.pose) != 0:
            st = distance_st / self.distance(ff[0].pose,state.pose)
        if ff[1] != 1 and self.distance(ff[1].pose,state.pose) != 0:
            ae = distance_ae / self.distance(ff[1].pose,state.pose)
        af = (distance_af/self.flag_distance(agent))
        ht = hit
        vf = -damage        
        
        #update current state data with fitness values
        state.fitness[Fitness.STAND_GROUND] += sg
        state.fitness[Fitness.STICK_TOGETHER] += st
        state.fitness[Fitness.APPROACH_ENEMY] += ae
        state.fitness[Fitness.APPROACH_FLAG] += af
        state.fitness[Fitness.HIT_TARGET] += ht
        state.fitness[Fitness.AVOID_FIRE] += vf
        
        # make the calculated motion
        position.x, position.y = state.pose[0], state.pose[1]
        agent.state.position = position
        rotation.z = new_heading
        agent.state.rotation = rotation
        state.prev_pose = state.pose
        state.pose = (new_position.x, new_position.y, rotation.z)
        state.time = time()
        
        #If it's the final state, handle clean up behaviors
        #You may get better behavior if you move this to epsiode_over
        if agent.step >= self.max_steps - 1:
            rtneat = agent.get_rtneat()
            pop = rtneat.get_population_ids()
            if len(pop) == 0:
                return 0
            avg,sig = self.generate_averages(agent)
            sums = Fitness()
            sums = getMod().weights * (state.fitness - avg) / sig
            #Add current unit to pop_state
            if agent.get_team() == 1: 
                self.pop_state_1[agent.org.id] = state
            else:
                self.pop_state_2[agent.org.id] = state
            state.final_fitness = sums.sum()
            print 'FITNESS:',getMod().weights * state.fitness,'=> Z-SCORE:', state.final_fitness
            return state.final_fitness

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
        
        state = self.get_state(agent)
        
        vx.append(self.raySense(agent, -60, MAX_SD, OBSTACLE))
        #vx.append(self.raySense(agent, -45, MAX_SD, OBSTACLE))
        vx.append(self.raySense(agent, -30, MAX_SD, OBSTACLE))
        #vx.append(self.raySense(agent, -15, MAX_SD, OBSTACLE))
        vx.append(self.raySense(agent, 0, MAX_SD, OBSTACLE))
        #vx.append(self.raySense(agent, 15, MAX_SD, OBSTACLE))
        vx.append(self.raySense(agent, 30, MAX_SD, OBSTACLE))
        #vx.append(self.raySense(agent, 45, MAX_SD, OBSTACLE))
        vx.append(self.raySense(agent, 60, MAX_SD, OBSTACLE))
        
        ffr = self.getFriendFoe(agent)
        if (ffr[0] == []) or ffr[1] == []:
            return v
        ff = []
        ff.append(self.nearest(state.pose, state.id, ffr[0]))
        ff.append(self.nearest(state.pose, state.id, ffr[1]))
        if ff[0] == 1:
            return v
        fd = self.distance(state.pose,(ff[1].pose[0],ff[1].pose[1]))
        if fd != 0:
            fh  = ((degrees(atan2(ff[1].pose[1]-state.pose[1],ff[1].pose[0] - state.pose[0])) - state.pose[2]) % 360) - 180
        else:
            fh = 0

        if fh < 0:
            fh += 360

        if fh > 360:
            fh -= 360
        

        vx.append(max(0,cos(radians(fh-  0))))
        #vx.append(max(0,cos(radians(fh- 45))))
        vx.append(max(0,cos(radians(fh- 90))))
        #vx.append(max(0,cos(radians(fh-135))))
        
        vx.append(max(0,cos(radians(fh-180))))
        #vx.append(max(0,cos(radians(fh-225))))
        vx.append(max(0,cos(radians(fh-270))))
        #vx.append(max(0,cos(radians(fh-315))))
       
        vx.append(min(1,max(0,(self.MAX_DIST-fd)/self.MAX_DIST))) 
        
        fd = self.distance(state.pose,(ff[0].pose[0],ff[0].pose[1]))
        if fd != 0:
            fh  = ((degrees(atan2(ff[0].pose[1]-state.pose[1],ff[0].pose[0] - state.pose[0])) - state.pose[2]) % 360) - 180
        else:
            fh = 0
        if fh < 0:
            fh += 360
        if fh > 360:
            fh -= 360
        
        vx.append(max(0,cos(radians(fh-  0))))
        #vx.append(max(0,cos(radians(fh- 45))))
        vx.append(max(0,cos(radians(fh- 90))))
        #vx.append(max(0,cos(radians(fh-135))))
        
        vx.append(max(0,cos(radians(fh-180))))
        #vx.append(max(0,cos(radians(fh-225))))
        vx.append(max(0,cos(radians(fh-270))))
        #vx.append(max(0,cos(radians(fh-315))))
       
        vx.append(min(1,max(0,(self.MAX_DIST-fd)/self.MAX_DIST))) 
        
        
        fd = self.flag_distance(agent)
        if fd != 0:
            fh  = ((degrees(atan2(self.flag_loc().y-state.pose[1],self.flag_loc().x - state.pose[0])) - state.pose[2]) % 360) - 180
        else:
            fh = 0

        if fh < 0:
            fh += 360

        if fh > 360:
            fh -= 360

        vx.append(max(0,cos(radians(fh-  0))))
        #vx.append(max(0,cos(radians(fh- 45))))
        vx.append(max(0,cos(radians(fh- 90))))
        #vx.append(max(0,cos(radians(fh-135))))
        
        vx.append(max(0,cos(radians(fh-180))))
        #vx.append(max(0,cos(radians(fh-225))))
        vx.append(max(0,cos(radians(fh-270))))
        #vx.append(max(0,cos(radians(fh-315))))
       
        vx.append(min(1,max(0,(self.MAX_DIST-fd)/self.MAX_DIST)))

        for iter in range(len(vx)):
            v[iter] = vx[iter]
        
        return v
   
    def flag_loc(self):
        """
        Returns the current location of the flag
        """
        from CaptureTheFlag.module import getMod
        return getMod().flag_loc

    def flag_distance(self, agent):
        """
        Returns the distance of the current agent from the flag
        """
        pos = self.get_state(agent).pose
        return pow(pow(float(pos[0]) - self.flag_loc().x, 2) + pow(float(pos[1]) - self.flag_loc().y, 2), .5)

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
                fraction = min(1.0, float(time() - state.time) / self.get_delay())
            pos = agent.state.position
            pos.x = x1 * (1 - fraction) + x2 * fraction
            pos.y = y1 * (1 - fraction) + y2 * fraction
            agent.state.position = pos
            self.set_animation(agent, state, 'run')
        else:
            self.set_animation(agent, state, 'stand')
        if time() - state.time > self.get_delay():
            state.time = time()
            return True
        else:
            return False
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        from CaptureTheFlag.module import getMod
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
        killScript('CaptureTheFlag/menu.py')
        return True

    def generate_averages(self,agent):
        """
        Generates averages for using Z-Fitness
        """
        rtneat = agent.get_rtneat()
        pop = rtneat.get_population_ids()
        #Add current unit to pop_state
        if agent.get_team() == 1: 
             pop_state = self.pop_state_1
        else:
             pop_state = self.pop_state_2
        curr_dict = {}
        for x in pop:
            curr_dict[x] = pop_state[x]
        curr_dict[agent.org.id] = pop_state[agent.org.id]
        pop_state = curr_dict
        # calculate population average
        # calculate population standard deviation
        stats = FitnessStats()
        for x in pop_state:
            f = pop_state[x].prev_fitness
            stats.add(f)
        return stats.mean, stats.stddev()
    
    def clear_averages(self):
        for x in self.pop_state_1:
            self.pop_state_1[x].prev_fitness = Fitness()
        for x in self.pop_state_2:
            self.pop_state_2[x].prev_fitness = Fitness()

    def get_delay(self):
        """
        Set simulation delay
        """
        return self.step_delay * (1.0 - self.speedup)
