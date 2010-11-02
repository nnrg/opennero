from OpenNero import *

class NSAgent(AgentBrain):
    """
    NeroSociety agent
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

    def start(self, time, sensors):
        """
        start of an episode
        """

    def act(self, time, sensors, reward):
        """
        a state transition
        """

    def end(self, time, reward):
        """
        end of an episode
        """
