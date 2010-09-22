import time
from math import *
from OpenNero import *
from Battle.module import *
from copy import copy
from random import *

MAX_SPEED = 12
MAX_SD = 100
OBSTACLE = 1#b0001
AGENT = 2#b0010

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
    def __init__(self, XDIM, YDIM):
        from Battle.module import getMod
        """
        Create the environment
        """
        print "CREATING NERO ENVIRONMENT: " + str(dir(module))
        Environment.__init__(self) 

        self.curr_id = 0
        self.step_delay = 0.25 # time between steps in seconds
        self.max_steps = 20
        self.time = time.time()
        self.XDIM = XDIM
        self.YDIM = YDIM
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
        
        #sbound.add_continuous(0, self.MAX_DIST) # Ally Sensors - Dist
        #sbound.add_continuous(-1 * pi * 2, pi * 2) # Ally Sensors - Heading
        
        #sbound.add_continuous(0, self.MAX_DIST) # Enemy Sensors - Dist
        #sbound.add_continuous(-1 * pi * 2, pi * 2) # Enemy Sensors - Heading
        
        #rewards
#        rbound.add_continuous(-100,100) # range for reward
        
        self.agent_info = AgentInitInfo(sbound, abound, rbound)
    
    def out_of_bounds(self, pos):
        return pos.x < 0 or pos.y < 0 or pos.x > self.XDIM or pos.y > self.YDIM
    
    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        if 1 == 1: return True
        state = self.get_state(agent)
        agent.state.position = copy(state.initial_position)
        agent.state.rotation = copy(state.initial_rotation)
        state.pose = (state.initial_position.x, state.initial_position.y, state.initial_rotation.z)
        state.prev_pose = state.pose
        state.total_damage = 0
        state.curr_damage = 0
        state.team = agent.getTeam()
        from client import set_stat
        ff = self.getFriendFoe(agent)
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
            return - 1
            
    def getFriendFoe(self, agent):
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
        A step for an agent
        """
        from Battle.module import getMod, parseInput

        state = self.get_state(agent)

        while getScriptData("Battle/menu.py"):
            r = getScriptOutput("Battle/menu.py")
            r.flush()
            parseInput(r.readline().strip())

        if getMod().hp != 0 and state.total_damage >= getMod().hp:
           agent.state.position.x, agent.state.position.y = -100,-100 
           state.prev_pose = state.pose
           state.pose = (-100,-100,0)
           state.time = time.time()
           return 0
        
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        if agent.step == 0:
            temp = getMod().flag_loc
            getMod().change_flag((temp.x + random()/2, temp.y + random()/2, temp.z + random()/2))
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
        
        if agent.step == 3:
            #This is a test!
            if getMod().getNumToAdd() > 0:
                dx = randrange(getMod().XDIM/20) - getMod().XDIM/40
                dy = randrange(getMod().XDIM/20) - getMod().XDIM/40
                getMod().addAgent((getMod().XDIM/2 + dx, getMod().YDIM/3 + dy, 2))
        
        
        # Update Damage totals
        state.total_damage += state.curr_damage
        damage = state.curr_damage
        state.curr_damage = 0
        
        #Add current unit to pop_state
        if agent.get_team() == 1: 
             self.pop_state_1[agent.org.id] = state 
        else:
             self.pop_state_2[agent.org.id] = state 
        
        # the position and the rotation of the agent on-screen
        position = agent.state.position
        rotation = agent.state.rotation
        # get the current pose of the agent
        (x, y, heading) = state.pose
        # get the actions of the agent
        turn_by = degrees(action[0] * 2.0) - degrees(action[2] * 2.0)
        move_by = action[1]
        #fire_by = action[2]
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
        
        # make the calculated motion
        position.x, position.y = state.pose[0], state.pose[1]
        agent.state.position = position
        rotation.z = new_heading
        agent.state.rotation = rotation
        state.prev_pose = state.pose
        state.pose = (new_position.x, new_position.y, rotation.z)
        state.time = time.time()
        return 0
    
    def raySense(self, agent, heading_mod, dist, types=0, draw=True, foundColor = Color(255, 0, 128, 128), noneColor = Color(255, 0, 255, 255) ):
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
        """ figure out what the agent should sense """
        from module import getMod
        v = self.agent_info.sensors.get_instance()
        vx = []
        
        state = self.get_state(agent)
        
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return v

        
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
        from Battle.module import getMod
        return getMod().flag_loc

    def flag_distance(self, agent):
        pos = self.get_state(agent).pose
        return pow(pow(float(pos[0]) - self.flag_loc().x, 2) + pow(float(pos[1]) - self.flag_loc().y, 2), .5)

    def distance(self, agloc, tgloc):
        return pow(pow(float(agloc[0] - tgloc[0]), 2) + pow(float(agloc[1] - tgloc[1]), 2), .5)

    def angle(self, agloc, tgloc):
        if(agloc[1] == tgloc[1]):
            return 0
        (x, y, heading) = agloc
        (xt, yt, ignore) = tgloc
        # angle to target
        theading = atan2(yt - y, xt - x)
        rel_angle_to_target = theading - radians(heading)
        return rel_angle_to_target

    def nearest(self, cloc, id, array):
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
        from Battle.module import getMod
        self.max_steps = getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        if getMod().hp != 0 and state.total_damage >= getMod().hp:
            return False
        else:
            return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        closeScript('Battle/menu.py')
        return True

    def get_delay(self):
        return self.step_delay * (1.0 - self.speedup)

def wrap_degrees(a, da):
    a2 = a + da
    if a2 > 180:
        a2 = -180 + (a2 % 180)
    elif a2 < -180:
        a2 = 180 - (abs(a2) % 180)
    return a2
