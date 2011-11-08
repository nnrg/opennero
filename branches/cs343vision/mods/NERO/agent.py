import constants

import OpenNero

import common

import Queue

class FirstPersonAgent(OpenNero.AgentBrain):
    """
    A human-controlled agent
    """
    key_buffer = Queue.Queue(5) # key buffer
    @staticmethod
    def control_fps(action):
        FirstPersonAgent.key_buffer.put(action)
    def __init__(self):
        OpenNero.AgentBrain.__init__(self) # do not remove!
        self.group = self.__class__.__name__
    def initialize(self, init_info):
        self.action_info = init_info.actions
        return True
    def get_team(self):
        # we are not on either team 1 or 2! we are just watching.
        return 3
    def key_action(self):
        action = self.action_info.get_instance() # create a zero action
        try:
            key = FirstPersonAgent.key_buffer.get_nowait()
            if key is not None and key in constants.FIRST_PERSON_ACTIONS:
                (movement, turn) = constants.FIRST_PERSON_ACTIONS[key]
            action[0] = movement
            action[1] = turn
        except Queue.Empty:
            pass # no keys were pressed
        return action
    def start(self, time, observations):
        return self.key_action()
    def act(self, time, observations, reward):
        return self.key_action()
    def end(self, time, reward):
        return True
