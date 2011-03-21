import time
from math import *
from OpenNero import *
from NERO_Battle.module import *
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
        # starting position
        self.initial_position = Vector3f(0, 0, 0)
        # starting orientation
        self.initial_rotation = Vector3f(0, 0, 0)
        self.time = time.time()
        self.start_time = self.time
        self.total_damage = 0
        self.curr_damage = 0
        self.team = 0
        self.animation = 'stand'

class NeroEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        from NERO_Battle.module import getMod
        """
        Create the environment
        """
        Environment.__init__(self) 
        
        self.curr_id = 0
        self.step_delay = 0.25 # time between steps in seconds
        self.max_steps = 20
        self.time = time.time()
        self.MAX_DIST = hypot(XDIM, YDIM)
        self.states = {}
        self.teams = {}
        self.speedup = 0
        
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
        # In battle mode, we tell each agent how much health it has left
        rbound.add_continuous(0,1)
        
        # initialize the rtNEAT algorithm parameters
        # input layer has enough nodes for all the observations plus a bias
        # output layer has enough values for all the actions
        # population size matches ours
        # 1.0 is the weight initialization noise
        rtneat = [RTNEAT("data/ai/neat-params.dat", N_SENSORS, N_ACTIONS, pop_size, 1.0, rbound), RTNEAT("data/ai/neat-params.dat", N_SENSORS, N_ACTIONS, pop_size, 1.0, rbound)]
        
        # set the initial lifetime
        # in Battle, the lifetime is basically infinite, unless they get killed
        lifetime = sys.maxint
        for algo in rtneat: 
            algo.set_lifetime(lifetime)
            algo.disable_evolution()
        print 'rtNEAT lifetime:', lifetime
        
        set_ai("rtneat0", rtneat[0])
        set_ai("rtneat1", rtneat[1])
        
        self.agent_info = AgentInitInfo(sbound, abound, rbound)
    
    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        from NERO_Battle.module import getMod
        state = self.get_state(agent)
        if agent.group == "Agent":
            dx = randrange(XDIM/20) - XDIM/40
            dy = randrange(XDIM/20) - XDIM/40
            if agent.get_team() == 0:
                state.initial_position.x = getMod().spawn_x_1 + dx
                state.initial_position.y = getMod().spawn_y_1 + dy
            if agent.get_team() == 1:
                state.initial_position.x = getMod().spawn_x_2 + dx
                state.initial_position.y = getMod().spawn_y_2 + dy
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
            agent.add_sensor(RaySensor(cos(radians(a)), sin(radians(a)), 0, 50, OBJECT_TYPE_OBSTACLE, True))
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
        other = abs(1-agent.get_team())
        if agent.get_team() in self.teams:
            friend = self.teams[agent.get_team()]
        if other in self.teams:
            foe = self.teams[other]
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

    blue_score = None
    red_score = None

    def step(self, agent, action):
        """
        2A step for an agent
        """
        from NERO_Battle.module import getMod, parseInput
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        state = self.get_state(agent)
        if len(self.teams) == 2:
            b = len(self.teams[0])
            r = len(self.teams[1])
            if b != NeroEnvironment.blue_score or r != NeroEnvironment.red_score:
                NeroEnvironment.blue_score = b
                NeroEnvironment.red_score = r
                print 'Blue Team Score:', b, 'Red Team Score:', r
        if 0 in self.teams and len(self.teams[0]) == 0:
            print "Red Team Wins!!"
            disable_ai()
        if 1 in self.teams and len(self.teams[1]) == 0:
            print "Blue Team Wins!!"
            disable_ai()
        
        # get the reward
        reward = self.agent_info.reward.get_instance()
        # the reward is just a fraction of the hit points left
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return reward
        
        # the reward is the fraction of hp's the agent has left
        hit_points = getMod().hp
        damage = state.total_damage
        if hit_points > 0:
            reward[0] = (hit_points - damage)/float(hit_points)

        # Spawn more agents if there are more to spawn
        # TODO: don't spawn more than the initial team size
        
        if get_ai("rtneat0").ready():
            dx = randrange(XDIM/20) - XDIM/40
            dy = randrange(XDIM/20) - XDIM/40
            getMod().addAgent((getMod().spawn_x_1 + dx, getMod().spawn_y_1 + dy, 2),OBJECT_TYPE_TEAM_0)
        
        if get_ai("rtneat1").ready():
            dx = randrange(XDIM/20) - XDIM/40
            dy = randrange(XDIM/20) - XDIM/40
            getMod().addAgent((getMod().spawn_x_2 + dx, getMod().spawn_y_2 + dy, 2),OBJECT_TYPE_TEAM_1)

        # Update Damage totals
        state.total_damage += state.curr_damage
        damage = state.curr_damage
        state.curr_damage = 0
        
        # the position and the rotation of the agent on-screen
        position = agent.state.position
        rotation = agent.state.rotation
        
        # get the current pose of the agent
        x, y, heading = position.x, position.y, rotation.z
        state.pose = (x,y,heading)
        
        # get the actions of the agent
        move_by = action[0]
        turn_by = degrees(action[1])
        
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
        if data != None:
            if agent.state.type == OBJECT_TYPE_TEAM_0:
                other_type = OBJECT_TYPE_TEAM_1
            elif agent.state.type == OBJECT_TYPE_TEAM_1:
                other_type = OBJECT_TYPE_TEAM_0
            fire_from = copy(position)
            fire_at = copy(data.state.position)
            fire_from.z += 4
            fire_at.z += 4
            objects = getSimContext().findInRay(fire_from, fire_at, other_type | OBJECT_TYPE_OBSTACLE, True)
            if len(objects) > 0 and objects[0].type & other_type:
                print 'agent id:', agent.state.id, 'type:', agent.state.type, 'fired at:',data.state.id,'of type:', data.state.type, 'hit:', objects[0].id, 'of type:', objects[0].type
                target = self.get_state(data)
                if target != -1:
                    target.curr_damage += 1
                    hit = 1

        # calculate friend/foe
        ffr = self.getFriendFoe(agent)
        if ffr[0] == ffr[1]: assert(false)
        if ffr[0] == []:
            return reward #Corner Case
        
        # calculate the motion
        new_position = copy(position)
        new_position.x, new_position.y = new_x, new_y
        
        # make the calculated motion
        agent.state.position = new_position
        rotation.z = new_heading
        agent.state.rotation = rotation
        state.prev_pose = state.pose
        state.pose = (new_position.x, new_position.y, rotation.z)
        state.time = time.time()
        
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
        if len(ffr[0]) == 0:
            xloc = 0
            yloc = 0
        else:
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
        from NERO_Battle.module import getMod
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
        if state.animation != animation:
            agent.state.setAnimation(animation)
            state.animation = animation
    
    def is_active(self, agent):
        """ return true when the agent should act """
        return True
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        from NERO_Battle.module import getMod
        if agent.group == "Turret": return False
        self.max_steps = getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return False#True
        if not get_ai("rtneat" + str(agent.get_team())).has_organism(agent):
            print 'agent selected by evolution!'
            return False#True
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            agent.state.position = Vector3f(100,100,100)
            state.pose = (agent.state.position.x, agent.state.position.y, agent.state.rotation.z)
            if agent in self.teams[agent.get_team()]: self.teams[agent.get_team()].pop(agent)
            return False#True
        else:
            return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        killScript('NERO_Battle/menu.py')
        return True

    def get_delay(self):
        """
        Set simulation delay
        """
        return self.step_delay * (1.0 - self.speedup)
