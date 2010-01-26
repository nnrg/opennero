import time
import world_handler
from common.server import GetRegisteredServer
from math import *
from OpenNero import *
##
from RTNEATAgent import RTNEATAgent
##
import pylab as pl

MAX_SPEED = 1
STEP_DT = 0.1
AGENT_X = 10
AGENT_Y = 10
N_FIXED_SENSORS = 3     # 0: wall bump, 1: self position X, 2: self position Y
N_S_IN_BLOCK = 4

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self, pos = Vector3f(0, 0, 0), rot = Vector3f(0, 0, 0)):
        self.initial_position = pos
        self.initial_rotation = rot
        self.step_count = 0
        self.episode_count = 0
        self.time = time.time()
        self.start_time = self.time
        self.bumped = False
        self.reward = 0

        self.initialize()

    def initialize(self):
        """ add more parameters here (optional) """
        self.initial_velocity = Vector3f(0, 0, 0)
        self.is_out = False

    def reset(self):
        self.goal_reached = False
        self.step_count = 0
        self.is_out = False
        self.reward = 0
        
class SandboxEnvironment(Environment):
    """
    Sample Environment for the Sandbox
    """
    def __init__(self, XDIM, YDIM, server):
        """
        Create the environment
        """
        Environment.__init__(self) 
        
        self.XDIM = XDIM
        self.YDIM = YDIM
        self.max_steps = 100        
        self.crumb_count = 0
        self.server = server
        self.states = {} # dictionary of agent states
        self.crumbs = world_handler.read_pellets("Roomba/world_config.txt")

        abound = FeatureVectorInfo() # actions
        sbound = FeatureVectorInfo() # sensors
        rbound = FeatureVectorInfo() # rewards

        # actions
        abound.add_continuous(-pi, pi) # amount to turn by
        
        # sensors
        sbound.add_discrete(0,1)    # wall bump
        sbound.add_continuous(0,93)   # position X
        sbound.add_continuous(0,93)   # position Y
        
        # rewards
        rbound.add_continuous(-100,100) # range for reward

        self.add_crumbs()        
        for crumb in self.crumbs:
            self.add_crumb_sensors(sbound)
        
        self.agent_info = AgentInitInfo(sbound, abound, rbound)        

    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            print "new state created"
            pos = agent.state.position
            rot = agent.state.rotation
            self.states[agent] = AgentState(pos, rot)
            return self.states[agent]
        
    def randomize(self):
        self.crumbs = world_handler.read_pellets("Stage1/server/world_config.txt")

    def add_crumb_sensors(self, sbound):
        """Add the crumb sensors, in order: x position of crumb (0 to XDIM,
        continuous), y position of crumb (0 to XDIM, continuous), whether
        crumb is present at said position or has been vacced (0 or 1), and
        reward for vaccing said crumb."""
        sbound.add_continuous(0, 93)    # crumb position X
        sbound.add_continuous(0, 93)    # crumb position Y
        sbound.add_discrete(0, 1)       # crumb present/ not present
        sbound.add_discrete(1, 5)       # reward for crumb

    def add_crumbs(self):
        for crumb in self.crumbs:
            if (crumb[2] != 1):
                self.server.mark_blue(float(crumb[0]), float(crumb[1]))
                self.crumb_count += 1
                print "crumb added"
                crumb[2] = 1     # adding crumbs to the matrix as we go

    def reset(self, agent):
        """ reset the environment to its initial state """
        state = self.get_state(agent)
        state.reset()
        agent.state.position = state.initial_position
        agent.state.rotation = state.initial_rotation
        agent.state.velocity = state.initial_velocity
        state.episode_count += 1
        self.add_crumbs()
        print "Episode %d complete" % state.episode_count
        return True
    
    def get_agent_info(self, agent):
        """ return a blueprint for a new agent """
        return self.agent_info

    def num_sensors(self):
        return (len(self.crumbs)*4 + N_FIXED_SENSORS)
    
    def step(self, agent, action):
        """
        A step for an agent
        """
        state = self.get_state(agent) # the agent's status
        if (state.is_out == True):
            server.unmark(agent.state.position.x, agent.state.position.y)
        else:
            assert(self.agent_info.actions.validate(action)) # check if the action is valid
            angle = action[0] # in range of -pi to pi
            position = Vector3f(agent.state.position.x, agent.state.position.y, agent.state.position.z)
            rotation = agent.state.rotation
            displacement = Vector3f(cos(angle) * MAX_SPEED, sin(angle) * MAX_SPEED, 0)
            if self.out_of_bounds(position + displacement):
                state.bumped = True
                if not self.out_of_bounds(position - displacement):
                    self.update_position(agent, position - displacement)
            else:
                rotation.z = rotation.z + degrees(angle)
                agent.state.rotation = rotation
                return self.update_position(agent, position+displacement)
            # check if agent has expended its step allowance
            if (self.max_steps != 0) and (state.step_count >= self.max_steps):
                state.is_out = True    # if yes, mark it to be removed
        return 0
        
    def update_position(self, agent, pos):
        agent.state.position = pos
        state = self.get_state(agent)
        state.step_count += 1
        print self.server.findInSphere(pos, MAX_SPEED)
        for crumb in self.crumbs:
            dist = sqrt((crumb[0] - pos.x)**2 + (crumb[1] - pos.y)**2)
            if (dist < MAX_SPEED):
                self.server.unmark(crumb[0], crumb[1])
                crumb[2] = 0     # remove crumb from memory matrix
                self.crumb_count -= 1  # decrement crumb count
                if (self.crumb_count <= 0 and self.max_steps != 0):
                    reward = crumb[3] + self.max_steps - state.step_count
                elif (self.max_steps == 0):
                    reward = crumb[3] + (1/agent.state_count) * 10000
                else:
                    reward = crumb[3]     # successfully vacuumed up a pellet
            else:
                reward = 0   # default reward
            #print "Crumb picked up @ (",crumb[0],", ",crumb[1],") for", reward, " points."
        return reward
    
    def out_of_bounds(self, pos):
        return \
            (pos.x) < 0 or \
            (pos.y) < 0 or \
            (pos.x) > self.XDIM or \
            (pos.y) > self.YDIM
    
    def sense(self, agent):
        """ figure out what the agent should sense """
        v = self.agent_info.sensors.get_instance()
        state = self.get_state(agent)
        if state.bumped:
            v[0] = 1
            state.bumped = False
        else:
            v[0] = 0

        # get agent's position
        pos = agent.state.position
        v[1] = pos.x
        v[2] = pos.y

        self.sense_crumbs(v, N_S_IN_BLOCK, N_FIXED_SENSORS)

        return v

    def sense_crumbs(self, sensors, num_sensors, start_sensor):
        i = start_sensor
        for crumb in self.crumbs:
            sensors[i] = crumb[0]
            sensors[i+1] = crumb[1]
            sensors[i+2] = int(crumb[2])
            sensors[i+3] = int(crumb[3])
            i = i + num_sensors
        return True
                     
    def is_active(self, agent):
        """ return true when the agent should act """
        state = self.get_state(agent)
        if time.time() - state.time > STEP_DT:
            state.time = time.time()
            return True
        else:
            return False     
    
    def is_episode_over(self, agent):
        """ is the current episode over for the agent? """
        state = self.get_state(agent)
        if self.max_steps != 0 and state.step_count >= self.max_steps:
            return True
        elif (self.crumb_count <= 0):
            return True
        else:
            return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        return True
