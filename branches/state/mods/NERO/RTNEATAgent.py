from OpenNero import *
import random
#from module import getMod
from time import time

def gettime():
	return time() 

EXPLOIT_PROB = 0.0

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
        from NERO.module import getMod;
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        self.team = getMod().currTeam #Team ID
        return True

    def start(self, time, sensors):
        """
        start of an episode
        """
        from NERO.module import getMod
        global rtneat
        print "PRODUCING A BRAND SPANKING NEW ORGANISM"
        EXPLOIT_PROB = getMod().ee
        self.org = rtneat.next_organism(EXPLOIT_PROB)
        #self.file_out = []
        #self.file_out.append(str(gettime()))
        #self.file_out.append(",")
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
        #self.file_out.append(str(gettime()))
        #self.file_out.append(",")
        self.reward += reward # store reward
        self.org.fitness = self.reward # assign organism fitness for evolution
        self.org.time_alive += 1
        #self.file_out.append(str(self.reward))
        #self.file_out.append('\n')
        #strn = "".join(self.file_out)
        #f = open('output.out','a')
        #f.write(strn)
        #assert(self.org.fitness >= 0) # we have to have a non-negative fitness for rtNEAT to work
        print  "Final reward: %f, cumulative: %f" % (reward, self.reward)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True
        
    def getTeam(self):
        return self.team
    
    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        assert(len(sensors)==15) # make sure we have the right number of sensors
        inputs = [sensor for sensor in sensors] # create the sensor array
        self.net.load_sensors(inputs)
        self.net.activate()
        outputs = self.net.get_outputs()

        actions = self.actions.get_instance() # make a vector for the actions
        #actions = outputs

        maxOutput = 0 # select the action based on the biggest output of the network

        for i in range(0,len(self.actions.get_instance())):
            actions[i] = outputs[i]
            #if outputs[maxOutput] < outputs[i]:
            #    maxOutput = i

        #actions[maxOutput] = 1

        return actions
