import time
from math import *
from OpenNero import *
from NERO.module import *
from constants import *
from common.fitness import Fitness, FitnessStats
from copy import copy
from random import *

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.id = -1
        self.total_damage = 0
        self.curr_damage = 0
        self.fitness = Fitness()
        self.prev_fitness = Fitness()
        self.final_fitness = 0
        self.animation = 'stand'

class NeroEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        from NERO.module import getMod
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
        self.speedup = 0
        
        self.pop_state = {}
        
        abound = FeatureVectorInfo() # actions
        sbound = FeatureVectorInfo() # sensors
        rbound = FeatureVectorInfo() # rewards
        
        # actions
        abound.add_continuous(-1,1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)
        
        # Wall sensors
        for a in WALL_SENSORS:
            sbound.add_continuous(0,1)
                
        # Flag sensors
        for fs in FLAG_SENSORS:
            sbound.add_continuous(0,1)
        
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
        state.total_damage = 0
        state.curr_damage = 0
        state.prev_fitness = state.fitness
        state.fitness = Fitness()
        #update client fitness
        from client import set_stat
        return True
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """ 
        for a in WALL_SENSORS:
            agent.add_sensor(RaySensor(cos(radians(a)), sin(radians(a)), 0, MAX_VISION_RADIUS, OBJECT_TYPE_OBSTACLE))
        for (a0, a1) in FLAG_SENSORS:
            agent.add_sensor(RadarSensor(a0, a1, -90, 90, MAX_VISION_RADIUS * 5, OBJECT_TYPE_FLAG))
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
            
    def step(self, agent, action):
        """
        2A step for an agent
        """
        from NERO.module import getMod, parseInput
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))
        
        state = self.get_state(agent)

        #Initilize Agent state
        if agent.step == 0:
            p = agent.state.position
            agent.state.rotation.z = randrange(360)
            r = agent.state.rotation
            self.pop_state[agent.org.id] = state

        #Spawn more agents if there are more to spawn (Staggered spawning times tend to yeild better behavior)
        if agent.step == 3:
            if getMod().getNumToAdd() > 0:
                dx = randrange(XDIM/20) - XDIM/40
                dy = randrange(XDIM/20) - XDIM/40
                getMod().addAgent((XDIM/2 + dx, YDIM/3 + dy, 2))

        #Add current unit to pop_state
        self.pop_state[agent.org.id] = state 

        # the position and the rotation of the agent on-screen
        position = agent.state.position
        rotation = agent.state.rotation
        
        # get the current pose of the agent
        x, y = position.x, position.y
        heading = rotation.z
        
        # get the actions of the agent
        move_by = action[0]
        turn_by = degrees(action[1])
        
        # figure out the new heading
        new_heading = wrap_degrees(heading, turn_by)
        
        # figure out the new x,y location
        new_x = x + MAX_MOVEMENT_SPEED * cos(radians(new_heading)) * move_by
        new_y = y + MAX_MOVEMENT_SPEED * sin(radians(new_heading)) * move_by
        
        # calculate the motion
        new_position = copy(position)
        new_position.x, new_position.y = new_x, new_y
        
        # make the calculated motion
        agent.state.position = new_position
        rotation.z = new_heading
        agent.state.rotation = rotation
        state.time = time.time()
        
        return 0

    def sense(self, agent, observations):
        """ 
        figure out what the agent should sense
        """
        # we only use the built-in sensors defined in get_agent_info
        return observations
   
    def is_active(self, agent):
        """ return true when the agent should act """
        return True
    
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
    
    def cleanup(self):
        """
        cleanup the world
        """
        killScript('NERO/menu.py')
        return True
