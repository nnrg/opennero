from OpenNero import *
import random
from time import time
from constants import *

FITNESS_OUT = False

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

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        return True

    def get_rtneat(self):
        """
        Returns the rtNEAT object for this agent
        """
        return get_ai("rtneat")

    def start(self, time, sensors):
        """
        start of an episode
        """
        from NERO.module import getMod
        EXPLOIT_PROB = getMod().ee
        rtneat = self.get_rtneat()
        self.org = rtneat.next_organism(EXPLOIT_PROB)
        self.state.label = "%.02f" % self.org.fitness
        self.group = "Agent"
        if FITNESS_OUT:    
            self.file_out = []
            self.file_out.append(str(gettime()))
            self.file_out.append(",")
        self.net = self.org.net
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # return action
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        if FITNESS_OUT:
            self.file_out.append(str(gettime()))
            self.file_out.append(",")
        self.org.fitness = self.fitness[0] # assign organism fitness for evolution
        self.org.time_alive += 1
        if FITNESS_OUT:
            self.file_out.append(str(self.fitness[0]))
            self.file_out.append('\n')
            strn = "".join(self.file_out)
            f = open('output.out','a')
            f.write(strn)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True
        
    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        # make sure we have the right number of sensors
        assert(len(sensors)==NEAT_SENSORS)
        # convert the sensors into the [0.0, 1.0] range
        sensors = self.sensors.normalize(sensors)
        # create the list of sensors
        inputs = [sensor for sensor in sensors]        
        # add the bias value
        inputs.append(NEAT_BIAS)
        # load the list of sensors into the network input layer
        self.net.load_sensors(inputs)
        # activate the network
        self.net.activate()
        # get the list of network outputs
        outputs = self.net.get_outputs()
        # create a C++ vector for action values
        actions = self.actions.get_instance()
        # assign network outputs to action vector
        for i in range(0,len(self.actions.get_instance())):
            actions[i] = outputs[i]
        # convert the action vector back from [0.0, 1.0] range
        actions = self.actions.denormalize(actions)
        #print "in:", inputs, "out:", outputs, "a:", actions
        return actions
