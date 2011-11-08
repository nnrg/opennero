import constants

from OpenNero import *

from common import *

class FirstPersonAgent(AgentBrain):
    """
    A human-controlled agent
    """
    key_pressed = None
    action_map = {'FWD':0, 'CW':3, 'CCW':2, 'BCK':1}
    def __init__(self):
        AgentBrain.__init__(self) # do not remove!
        self.time = 0
    def initialize(self, init_info):
        self.constraints = init_info.actions
        return True
    def key_action(self):
        self.time += 1
        action = self.constraints.get_instance()
        if FirstPersonAgent.key_pressed:
            action[0] = FirstPersonAgent.action_map[FirstPersonAgent.key_pressed]
            FirstPersonAgent.key_pressed = None
        else:
            action[0] = MAZE_NULL_MOVE
        return action
    def start(self, time, observations):
        return self.key_action()
    def act(self, time, observations, reward):
        return self.key_action()
    def end(self, time, reward):
        return True