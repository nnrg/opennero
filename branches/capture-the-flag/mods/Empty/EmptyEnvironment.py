from OpenNero import *
from Empty.module import *

class EmptyEnvironment(Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        
        Environment.__init__(self)
        
        self.agent_info = None # temporary
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        return self.agent_info

    def step(self, agent, action):
        
        return 0

    def sense(self, agent):
        """ 
        figure out what the agent should sense 
        """
    
    def is_active(self, agent):
        """
        return true when the agent should act
        """
        return True
    
    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        return True
