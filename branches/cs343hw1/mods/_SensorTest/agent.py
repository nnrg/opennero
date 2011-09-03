from OpenNero import *

class NoopAgent(AgentBrain):
    """
    The actions are the same as the observations
    """
    def __init__(self):
        AgentBrain.__init__(self) # have to make this call

    def initialize(self, init_info):
        """
        create a new agent
        """
        return True

    def start(self, time, sensors):
        """
        return first action given the first observations
        """
        return sensors

    def reset(self):
        pass

    def act(self, time, sensors, reward):
        """
        return an action given the reward for the previous action and the new observations
        """
        return sensors

    def end(self, time, reward):
        """
        receive the reward for the last observation
        """
        print  "Final reward:", reward, "cumulative:", self.fitness
        return True

    def destroy(self):
        """
        called when the agent is done
        """
        return True