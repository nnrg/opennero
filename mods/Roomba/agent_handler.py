### Testing code ###
# First Modified: Oct 7, 2009
# Last Modified: Oct 7, 2009
" Code for handling different agent types "
" Code for handling agent states "
import time
from OpenNero import *

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

class AgentInit:
    """
    Initialization parameters for different types of agent
    """
    def __init__(self, agent_type = "common"):
        self.types = {}
        self.add_type(agent_type)
        
    def add_type(self, agent_type):
        """ add init info for an agent type that is not yet present """
        if agent_type in self.types:
            print "Init info found for agent type ", agent_type, ". Did nothing."
            return False
        else:
            self.types[agent_type] = []
            abound = FeatureVectorInfo() # actions
            sbound = FeatureVectorInfo() # sensors
            rbound = FeatureVectorInfo() # rewards
            self.types[agent_type].append(abound)
            self.types[agent_type].append(sbound)
            self.types[agent_type].append(rbound)
            return True

    def get_sensor(self, agent_type = "common"):
        """ return sensor vector for agent type.
        'common' assumed to be agent type if argument ommitted. """
        if not agent_type in self.types:
            print "Sensors NOT found for agent type ", agent_type,". Did nothing."
            return False
        else:
            return self.types[agent_type][1]

    def get_action(self, agent_type = "common"):
        """ return action vector for agent type.
        'common' assumed to be agent type if argument ommitted. """
        if not agent_type in self.types:
            print "Actions NOT found for agent type ", agent_type,". Did nothing."
            return False
        else:
            return self.types[agent_type][0]

    def get_reward(self, agent_type = "common"):
        """ return reward vector for agent type.
        'common' assumed to be agent type if argument ommitted. """
        if not agent_type in self.types:
            print "Rewards NOT found for agent type ", agent_type,". Did nothing."
            return False
        else:
            return self.types[agent_type][2]
            
    def get_info(self, agent_type = "common"):
        """ return init info for agent type.
        'common' assumed to be agent type if argument ommitted. """
        if not (agent_type in self.types):
            print "Init info NOT found for agent type ", agent_type,". Did nothing."
            return False
        else:
            sbound = self.get_sensor(agent_type)
            abound = self.get_action(agent_type)
            rbound = self.get_reward(agent_type)
            return AgentInitInfo(sbound, abound, rbound) 
     
