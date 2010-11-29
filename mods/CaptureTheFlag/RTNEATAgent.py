from OpenNero import *
import random
from time import time
from CaptureTheFlag.constants import *

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
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        AgentBrain.__init__(self)

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        from CaptureTheFlag.module import getMod
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        self.team = getMod().currTeam #Team ID
        return True

    def get_rtneat(self):
        """
        Returns the rtNEAT object for this agent
        """
        return get_ai("neat%d" % self.team)

    def start(self, time, sensors):
        """
        start of an episode
        """
        print "in RTNEATAGENT start"
        from CaptureTheFlag.module import getMod
        EXPLOIT_PROB = getMod().ee
        rtneat = get_ai("neat%d" % self.team)
        self.org = rtneat.next_organism(EXPLOIT_PROB)
        self.state.label = "%.02f" % self.org.fitness
        if FITNESS_OUT:    
            self.file_out = []
            self.file_out.append(str(gettime()))
            self.file_out.append(",")
        self.net = self.org.net
        self.reward = 0
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        self.reward += reward # store reward        
        # return action
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        print "in RTNEATAGENT end"
        if FITNESS_OUT:
            self.file_out.append(str(gettime()))
            self.file_out.append(",")
        self.reward += reward # store reward
        self.org.fitness = self.reward # assign organism fitness for evolution
        self.org.time_alive += 1
        if FITNESS_OUT:
            self.file_out.append(str(self.reward))
            self.file_out.append('\n')
            strn = "".join(self.file_out)
            f = open('output.out' + str(self.team),'a')
            f.write(strn)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True
        
    def getTeam(self):
        """
        Returns this teams information (depricated)
        """
        return self.team
   
    def get_team(self):
        """
        Returns this teams information
        """
        return self.team

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        from CaptureTheFlag.module import getMod
        assert(len(sensors)==NEAT_SENSORS) # make sure we have the right number of sensors
        sensors = self.sensors.normalize(sensors)
        inputs = [sensor for sensor in sensors] # create the sensor array
        self.net.load_sensors(inputs)
        self.net.activate()
        outputs = self.net.get_outputs()
        actions = self.actions.get_instance() # make a vector for the actions
        for i in range(0,len(self.actions.get_instance())):
            actions[i] = outputs[i]
        actions = self.actions.denormalize(actions)
        return actions
