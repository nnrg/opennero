from OpenNero import *
from math import *

class Turret(AgentBrain):
    """
    Simple Rotating Turret
    """
    def __init__(self):
        AgentBrain.__init__(self)

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        self.previous_fire =  0
        return True

    def start(self, time, sensors):
        a = self.actions.get_instance()
        for x in range(len(self.actions)):
            a[x] = 0
            if x == 2:
              a[x] = 0
        return a

    def act(self,time,sensors,reward):
        a = self.actions.get_instance()
        for x in range(len(self.actions)):
            a[x] = 0
            if x == 1:
              a[x] = .1

        return a

    def getTeam(self):
        return 2

    def end(self,time,reward):
        return True

    def destroy(self):
        return True
