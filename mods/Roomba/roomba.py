from OpenNero import *
import time
import random
import math
import action_script
from Queue import Queue
from constants import *

class RoombaBrain(AgentBrain):
    """
    Scripted behavior for Roomba agent
    A "lifetime" of an agent brain looks like this:
    1. __init__() is called (new brain)
    2. initialize() is called (passing specs for sensors, actions and rewards)
    3. start() is called
    4. act() is called 0 or more times
    5. end() is called
    6. if new episode, go to 3, otherwise go to 7
    7. destroy() is called
    """
    def __init__(self):
        """
        this is the constructor - it gets called when the brain object is first created
        """
        # call the parent constructor
        AgentBrain.__init__(self) # do not remove!
        self.action_sq = Queue()
          
    def initialize(self, init_info):
        """
        init_info contains a description of the observations the agent will see
        and the actions the agent will be able to perform
        """
        self.init_info = init_info
        return True

    def start(self, time, sensors):
        """
        Take in the initial sensors and return the first action
        """
        return self.act(time, sensors, 0)
        
    def act(self, time, sensors, reward):
        """
        Take in new sensors and reward from previous action and return the next action
        Specifically, just move toward the closest crumb!
        """
        action = self.init_info.actions.get_instance()
        # just tell the agent which way to go!
        action[0] = action_script.go_xy(sensors, sensors[3], sensors[4])
        return action
        
    def end(self, time, reward):
        """
        take in final reward
        """
        return True

    def destroy(self):
        """
        called when the agent is destroyed
        """
        self.time = 0
        return True
