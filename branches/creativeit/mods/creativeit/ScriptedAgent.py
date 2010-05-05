from OpenNero import *
from common.server import GetRegisteredServer
import random

class ScriptedAgent(AgentBrain):
    """
    Scripted agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        AgentBrain.__init__(self)

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
        self.reward = 0
        self.count = 0
        self.fitness = 0
        GetRegisteredServer().scripted.flush()
        return self.take_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # Store fitness if a cube was collected, otherwise accumulate reward.
        if reward >= 1:
            self.fitness += reward
            self.reward = 0
            self.count = 0
        else:
            self.reward += reward
            self.count += 1
        return self.take_action(sensors) # return action

    def end(self, time, reward):
        """
        end of an episode
        """
        # store last reward and add average reward to fitness (i.e. how good the agent was since
        # collecting the last cube)
        self.reward += reward
        self.count += 1
        self.fitness += self.reward/self.count
        print  "Final reward: %f, fitness: %f" % (reward, self.fitness)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True
        
    def take_action(self, sensors):
        """
        Produce actions from sensor readings
        """
        assert(len(sensors) == GetRegisteredServer().num_sensors) # make sure we have the right number of sensors

        # create the sensor array and convert sensor values from network to advice space for
        # script computations.
        inputs = [sensor for sensor in sensors]
        for i in range(len(inputs)):
            nmin = GetRegisteredServer().sbounds_network.min(i)
            nmax = GetRegisteredServer().sbounds_network.max(i)
            amin = GetRegisteredServer().sbounds_advice.min(i)
            amax = GetRegisteredServer().sbounds_advice.max(i)
            inputs[i] = (inputs[i]-nmin)*(amax-amin)/(nmax-nmin) + amin;

        outputs = GetRegisteredServer().scripted.evaluate(inputs) # evaluate the script to find outputs
        actions = self.actions.get_instance() # make a vector for the actions

        assert(len(actions) == len(outputs))
        for i in range(len(outputs)):
            assert((outputs[i] >= -1 and outputs[i] <= 1), "agent action not in range [-1, 1]")
            actions[i] = (outputs[i]+1.0)/2.0  # convert value from advice to network space, i.e. [0, 1]
            actions[i] = actions[i] - 0.5

        return actions
