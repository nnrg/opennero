import time
from math import *
from OpenNero import *
from NERO.module import *
from constants import *
from copy import copy
from random import *
import sys

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
        # previous command
        self.prev_command_pose = None
        # starting position
        self.initial_position = Vector3f(0, 0, 0)
        # starting orientation
        self.initial_rotation = Vector3f(0, 0, 0)
        self.time = time.time()
        self.start_time = self.time
        self.total_damage = 0
        self.curr_damage = 0

class NeroEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        from NERO.module import getMod
        """
        Create the environment
        """
        Environment.__init__(self) 
        
        self.curr_id = 0
        self.step_delay = 0.1 # delay between AI decisions at speedup of 0
        self.speedup = 0 # between 0 and 1.0
        self.max_steps = 20
        self.time = time.time()
        self.MAX_DIST = hypot(XDIM, YDIM)
        self.states = {}
        self.teams = {}
        
        abound = FeatureVectorInfo() # actions
        sbound = FeatureVectorInfo() # sensors
        rbound = FeatureVectorInfo() # rewards
        
        # actions
        abound.add_continuous(-1,1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)

        # sensor dimensions
        for a in range(N_SENSORS):
            sbound.add_continuous(0,1);

        # Rewards
        # the enviroment returns the raw multiple dimensions of the fitness as
        # they get each step. This then gets combined into, e.g. Z-score, by
        # the ScoreHelper in order to calculate the final rtNEAT-fitness
        for f in FITNESS_DIMENSIONS:
            # we don't care about the bounds of the individual dimensions
            rbound.add_continuous(-sys.float_info.max, sys.float_info.max) # range for reward
        
        # initialize the rtNEAT algorithm parameters
        # input layer has enough nodes for all the observations plus a bias
        # output layer has enough values for all the actions
        # population size matches ours
        # 1.0 is the weight initialization noise
        rtneat = RTNEAT("data/ai/neat-params.dat", N_SENSORS, N_ACTIONS, pop_size, 1.0, rbound)
        
        # set the initial lifetime
        lifetime = getMod().lt
        rtneat.set_lifetime(lifetime)
        print 'rtNEAT lifetime:', lifetime
        
        set_ai("rtneat", rtneat)
        
        self.agent_info = AgentInitInfo(sbound, abound, rbound)
    
    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        from NERO.module import getMod
        state = self.get_state(agent)
        if agent.group == "Agent":
            dx = randrange(XDIM/20) - XDIM/40
            dy = randrange(XDIM/20) - XDIM/40
            state.initial_position.x = getMod().spawn_x + dx
            state.initial_position.y = getMod().spawn_y + dy
            agent.state.position = copy(state.initial_position)
            agent.state.rotation = copy(state.initial_rotation)
            state.pose = (state.initial_position.x, state.initial_position.y, state.initial_rotation.z)
            state.prev_pose = state.pose
        state.total_damage = 0
        state.curr_damage = 0
        ff = self.getFriendFoe(agent)
        return True
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """ 
        for a in WALL_RAY_SENSORS:
            agent.add_sensor(RaySensor(cos(radians(a)), sin(radians(a)), 0, 50, OBJECT_TYPE_OBSTACLE, False))
        for (a0, a1) in FLAG_RADAR_SENSORS:
            agent.add_sensor(RadarSensor(a0, a1, -90, 90, MAX_VISION_RADIUS, OBJECT_TYPE_FLAG, False))
        for (a0, a1) in ENEMY_RADAR_SENSORS:
            if agent.get_team() == 0: agent.add_sensor(RadarSensor(a0, a1, -90, 90, MAX_VISION_RADIUS, OBJECT_TYPE_TEAM_1, False))
            if agent.get_team() == 1: agent.add_sensor(RadarSensor(a0, a1, -90, 90, MAX_VISION_RADIUS, OBJECT_TYPE_TEAM_0, False))
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
            if agent.get_team() not in  self.teams:
                self.teams[agent.get_team()] = {}
            self.teams[agent.get_team()][agent] = self.states[agent]
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
        if agent.get_team() in self.teams:
            friend = self.teams[agent.get_team()]
        if 1-agent.get_team() in self.teams:
            foe = self.teams[1-agent.get_team()]
        else:
            foe = []
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
            fd = self.distance(state.pose,(alt[curr].pose[0],alt[curr].pose[1]))
            if fd != 0:
                fh  = ((degrees(atan2(alt[curr].pose[1]-state.pose[1],alt[curr].pose[0] - state.pose[0])) - state.pose[2]) % 360)
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
        from NERO.module import getMod, parseInput
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        state = self.get_state(agent)
        
        # get the reward (which has multiple components)
        reward = self.agent_info.reward.get_instance()

        #Initilize Agent state
        if agent.step == 0 and agent.group != "Turret":
            p = copy(agent.state.position)
            r = copy(agent.state.rotation)
            if agent.group == "Agent":
                r.z = randrange(360)
                agent.state.rotation = r #Note the internal components of agent.state.rotation are immutable you need to make a copy, modify the copy, and set agent.state.rotation to be the copy.
            state.initial_position = p
            state.initial_rotation = r
            #print 'initial_rotation:',state.initial_rotation, "group:", agent.group
            state.pose = (p.x, p.y, r.z)
            state.prev_pose = (p.x, p.y, r.z)
            return reward
        
        # Spawn more agents if there are more to spawn
        if get_ai("rtneat").ready():
            if getMod().getNumToAdd() > 0:
                dx = randrange(XDIM/20) - XDIM/40
                dy = randrange(XDIM/20) - XDIM/40
                getMod().addAgent((getMod().spawn_x + dx, getMod().spawn_y + dy, 2))

        # Update Damage totals
        state.total_damage += state.curr_damage
        damage = state.curr_damage
        state.curr_damage = 0
        
        #Fitness Function Parameters
        distance_st = getMod().dta
        distance_ae = getMod().dtb
        distance_af = getMod().dtc
        friendly_fire = getMod().ff

        # the position and the rotation of the agent on-screen
        position = copy(agent.state.position)
        rotation = copy(agent.state.rotation)
        
        # get the current pose of the agent
        x, y, heading = position.x, position.y, rotation.z
        state.pose = (x,y,heading)
        
        # get the actions of the agent
        move_by = action[0]
        turn_by = degrees(action[1])
        
        # set animation speed
        if self.speedup < 1.0:
            animation_speed = move_by * 250.0 / (1.0 - self.speedup)
            if agent.state.animation_speed != animation_speed:
                agent.state.animation_speed = animation_speed
        else:
            agent.state.animation_speed = 250.0
        
        # figure out the new heading
        new_heading = wrap_degrees(heading, turn_by)
        
        # figure out the new x,y location
        new_x = x + MAX_MOVEMENT_SPEED * cos(radians(new_heading)) * move_by
        new_y = y + MAX_MOVEMENT_SPEED * sin(radians(new_heading)) * move_by

        # figure out the firing location
        fire_x = x + self.MAX_DIST * cos(radians(new_heading))
        fire_y = y + self.MAX_DIST * sin(radians(new_heading))

        # draw the line of fire
        fire_pos = copy(position)
        fire_pos.x, fire_pos.y = fire_x, fire_y
        
        # calculate if we hit anyone
        hit = 0
        data = self.target(agent)
        if data != None:#len(data) > 0:
            objects = getSimContext().findInRay(position,data.state.position, OBJECT_TYPE_OBSTACLE | OBJECT_TYPE_TEAM_0 | OBJECT_TYPE_TEAM_1, True)
            if len(objects) > 0: sim = objects[0]
            else: sim = data
            if len(objects) == 0 or objects[0] == sim:
                target = self.get_state(data)
                if target != -1:
                    target.curr_damage += 1
                    hit = 1
                    #print "Target hit successfully Firing Group:", agent.group
        
        # calculate friend/foe
        ffr = self.getFriendFoe(agent)
        if ffr[0] == []:
            return reward #Corner Case
        
        ff = []
        ff.append(self.nearest(state.pose, state.id, ffr[0]))
        ff.append(self.nearest(state.pose, state.id, ffr[1]))
        
        #calculate fitness accrued during this step
        R = dict([(f, 0) for f in FITNESS_DIMENSIONS])
        R[FITNESS_STAND_GROUND] = -action[0]
        if ff[0]:
            d = self.distance(self.get_state(ff[0]).pose,state.pose)
            R[FITNESS_STAND_GROUND] = -d*d
        if ff[1]:
            d = self.distance(self.get_state(ff[1]).pose,state.pose)
            R[FITNESS_APPROACH_ENEMY] = -d*d
        d = self.flag_distance(agent)
        R[FITNESS_APPROACH_FLAG] = -d*d
        R[FITNESS_HIT_TARGET] = hit
        R[FITNESS_AVOID_FIRE] = -damage
        
        # put the fitness dimensions into the reward vector in order
        for (i,f) in enumerate(FITNESS_DIMENSIONS):
            reward[i] = R[f]
        
        # tell the system to make the calculated motion
        # this is actually done in is_active() and depends on speedup
        state.prev_pose = state.pose
        state.pose = (new_x, new_y, new_heading)
        state.time = time.time()

        state.prev_command_pose = None
        print 'NeroEnvironment::step:', agent.state.id, state.prev_pose, state.pose
        
        return reward

    def sense(self, agent, observations):
        """ 
        figure out what the agent should sense
        """
        # we only use the built-in sensors defined in get_agent_info
        
        f = len(observations)
        state = self.get_state(agent)
        
        ffr = self.getFriendFoe(agent)
        if (ffr[0] == []): return v
        xloc = agent.state.position.x
        yloc = agent.state.position.y
        for x in ffr[0]:
            xloc += ffr[0][x].pose[0]
            yloc += ffr[0][x].pose[1]
        xloc /= len(ffr[0])
        yloc /= len(ffr[0])
        fd = self.distance(state.pose,(xloc,yloc))
        fh = 0
        if fd != 0:
            fh = ((degrees(atan2(yloc-state.pose[1],xloc - state.pose[0])) - state.pose[2]) % 360) - 180

        if fd <= 15:
            observations[f-3] = fd/15.0
            observations[f-2] = fh/360.0

        if observations[f-2] < 0: observations[f-2] += 1

        
        data = self.target(agent)
        observations[f-1] = 0
        if data != None: observations[f-1] = 1

        return observations
   
    def flag_loc(self):
        """
        Returns the current location of the flag
        """
        from NERO.module import getMod
        return getMod().flag_loc

    def flag_distance(self, agent):
        """
        Returns the distance of the current agent from the flag
        """
        pos = agent.state.position
        flag_loc = self.flag_loc()
        return hypot(pos.x - flag_loc.x, pos.y - flag_loc.y)

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
        nearest = None
        value = self.MAX_DIST * 5
        for other in array:
            if id == array[other].id:
                continue
            if self.distance(cloc, array[other].pose) < value:
                nearest = other
                value - self.distance(cloc, array[other].pose)
        return nearest

    def set_animation(self, agent, state, animation):
        """
        Sets current animation
        """
        if agent.state.animation != animation:
            agent.state.animation = animation
    
    def is_active(self, agent):
        """ 
        Return true when the agent should consider what to do next
        During the frames in-between, interpolate from previous
        to current pose.
        """
        # get the agent state
        state = self.get_state(agent)

        # calculate desired delay and time since last update
        delay = self.step_delay * (1.0 - self.speedup)
        time_since_last = time.time() - state.time

        # if the previous motion was a bump, we just repeat the previous request
        # this is where "unstuck" code would go
        if agent.state.bumped and state.prev_command_pose:
            print 'NeroEnvironment::is_active:bumped:', agent.state.id, state.prev_command_pose
            agent.state.position = state.prev_command_pose
            return time_since_last > delay
        
        # try to interpolate between prev_pose and pose
        (x1, y1, h1) = state.prev_pose
        (x2, y2, h2) = state.pose
        if delay <= time_since_last:
            f = 1.0
        else:
            f = float(time_since_last) / delay
            self.set_animation(agent, state, 'run')
        # f between p1 (0) and p2 (1)
        x = x1 * (1 - f) + x2 * f
        y = y1 * (1 - f) + y2 * f
        # combine degrees carefully to avoid 'flips'
        # see common/module.py for def
        h = mix_angles(h1,h2,f)

        # try to update position
        pos = copy(agent.state.position)
        pos.x = x
        pos.y = y
        print 'NeroEnvironment::is_active:', agent.state.id, agent.state.position, pos
        state.prev_command_pose = pos
        agent.state.position = pos

        # try to update rotation
        rot = copy(agent.state.rotation)
        rot.z = h1 * (1-f) + h2 * f
        agent.state.rotation = rot

        # return if ready for the next decision
        return f >= 1.0
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        from NERO.module import getMod
        if agent.group == "Turret": return False
        self.max_steps = getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        if not get_ai("rtneat").has_organism(agent):
            print 'agent selected by evolution!'
            return True
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return True
        else:
            return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        killScript('NERO/menu.py')
        return True
