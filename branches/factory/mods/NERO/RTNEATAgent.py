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
        self.team = 0

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
        rtneat = get_ai("rtneat")
        return rtneat.get_organism(self)

    def start(self, time, sensors):
        """
        start of an episode
        """
        from NERO.module import getMod
        EXPLOIT_PROB = getMod().ee
        org = self.get_org()
        org.time_alive += 1
        self.state.label = "%.02f" % org.fitness
        if FITNESS_OUT:    
            self.file_out = []
            self.file_out.append(str(gettime()))
            self.file_out.append(",")
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
        if FITNESS_OUT:
            self.file_out.append(str(self.fitness[0]))
            self.file_out.append('\n')
            strn = "".join(self.file_out)
            f = open('output.out','a')
            f.write(strn)
        get_ai("rtneat").release_organism(self)
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
        assert(len(sensors)==NEAT_SENSORS)
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
