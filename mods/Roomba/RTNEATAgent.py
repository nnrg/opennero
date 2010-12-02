from OpenNero import *
import math
import random

class RTNEATAgent(AgentBrain):
    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        AgentBrain.__init__(self)
        global rtneat
        rtneat = get_ai("neat")

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        return True

    def start(self, time, sensors):
        """
        start of an episode
        """
        global rtneat
        self.org = rtneat.next_organism(0.5)
        self.net = self.org.net
        #print "got network to evaluate: " + str(self.net)
        self.reward = 0
        sensors = self.sensors.normalize(sensors)
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
	if reward >= 1:
	    self.reward += reward # store reward
        sensors = self.sensors.normalize(sensors)
        # return action
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        self.reward += reward # store reward
        self.org.fitness = self.reward # assign organism fitness for evolution
        self.org.time_alive += 1
        #assert(self.org.fitness >= 0) # we have to have a non-negative fitness for rtNEAT to work
        #print  "Final reward: %f, cumulative: %f" % (reward, self.reward)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True
        
    def network_action(self, sensors):
        """
        Take the current network.
        Feed the sensors into it.
        Activate the network to produce the output.
        Collect and interpret the outputs as valid maze actions.
        """
        assert(len(sensors)==6) # make sure we have the right number of sensors
        inputs = [sensor for sensor in sensors] # create the sensor array
        self.net.load_sensors(inputs)
        #print "Loading sensor. Value is: ", inputs
        self.net.activate()
        outputs = self.net.get_outputs()
        actions = self.actions.get_instance() # make a vector for the actions

        assert(len(actions) == len(outputs))
        for i in range(len(outputs)):
            actions[i] = outputs[i] - 0.5

        #print actions
        return actions
