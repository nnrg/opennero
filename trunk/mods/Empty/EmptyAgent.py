from OpenNero import *

class EmptyAgent(AgentBrain):
    """
    An empty agent
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
        print 'initialize:',init_info

    def start(self, time, sensors):
        """
        start of an episode
        """
        print 'start:',time,sensors

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        print 'act:',time,sensors,reward

    def end(self, time, reward):
        """
        end of an episode
        """
        print 'end:',time,reward
