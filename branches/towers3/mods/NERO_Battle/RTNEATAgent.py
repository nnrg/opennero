from OpenNero import *
import random
from time import time
from constants import *
from math import *

def gettime():
    return time() 

class RTNEATAgent(AgentBrain):
    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an 
        # AgentBrainPtr by C++
        AgentBrain.__init__(self)
        from NERO_Battle.module import getMod
        team_type = getMod().curr_team
        if team_type == OBJECT_TYPE_TEAM_0:
            self.team = TEAM0
        elif team_type == OBJECT_TYPE_TEAM_1:
            self.team = TEAM1

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        self.group = "Agent"
        return True

    def get_org(self):
        """
        Returns the rtNEAT object for this agent
        """
        rtneat = get_ai("rtneat" + str(self.team))
        return rtneat.get_organism(self)

    def display_health(self, h):
        # the reward in battle tells us how much life we have left
        # it is a value between 0 (no health) and 1 (full health)
        # so we display it as a label consisting of round(r*10) characters
        self.state.label = ''.join(['.' for x in range(round(h*10))])

    def start(self, time, sensors):
        """
        start of an episode
        """
        self.display_health(1)
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        self.display_health(reward[0])
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        self.display_health(reward[0])
        get_ai("rtneat" + self.team).release_organism(self)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True

    def get_team(self):
        return self.team

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        # make sure we have the right number of sensors
        assert(len(sensors)==N_SENSORS)
        # convert the sensors into the [0.0, 1.0] range
        sensors = self.sensors.normalize(sensors)
        # create the list of sensors
        inputs = [sensor for sensor in sensors]        
        # add the bias value
        inputs.append(NEAT_BIAS)
        # get the current organism we are using
        org = self.get_org()
        # increment the lifetime counter
        org.time_alive += 1
        # get the current network we are using
        net = org.net
        # load the list of sensors into the network input layer
        net.load_sensors(inputs)
        # activate the network
        net.activate()
        # get the list of network outputs
        outputs = net.get_outputs()
        # create a C++ vector for action values
        actions = self.actions.get_instance()
        # assign network outputs to action vector
        for i in range(0,len(self.actions.get_instance())):
            actions[i] = outputs[i]
        # convert the action vector back from [0.0, 1.0] range
        actions = self.actions.denormalize(actions)
        #print "in:", inputs, "out:", outputs, "a:", actions
        return actions
