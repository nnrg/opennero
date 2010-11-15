from OpenNero import *
from common import *
from constants import *
from math import *

class SensorTestEnvironment(Environment):

    def __init__(self):
        Environment.__init__(self)
        self.counter = 0
        print 'SensorTestEnvironment()'

    def get_agent_info(self, agent):
        print 'SensorTestEnvironment.get_agent_info()'
        for i in N_OBJECTS:
            a = radians(i * 360.0 / N_OBJECTS)
            cosa, sina = cos(a), sin(a)
            a0 = radians((i-0.5) * 360.0 / N_OBJECTS)
            a1 = radians((i+0.5) * 360.0 / N_OBJECTS)
            # N ray sensors that should be hitting the boxes
            agent.add_sensor(RaySensor(cosa, sina, 0, 2 * RADIUS, OBJECT_TYPE_SENSED))
            # N ray sensors that should be too short
            agent.add_sensor(RaySensor(cosa, sina, 0, 0.5 * RADIUS, OBJECT_TYPE_SENSED))
            # N ray sensors that should miss
            agent.add_sensor(RaySensor(cos(a0), sin(a0), 2 * RADIUS, OBJECT_TYPE_SENSED))
            # N radar sensors that should have 1 object each
            agent.add_sensor(RadarSensor(a0 - 180.0, a1 - 180.0, -90, 90, 2 * RADIUS, OBJECT_TYPE_SENSED))
            # N radar sensors that should be too short
            agent.add_sensor(RadarSensor(a0 - 180.0, a1 - 180.0, -90, 90, 0.5 * RADIUS, OBJECT_TYPE_SENSED))
        # a radar sensors that should have all the objects
        agent.add_sensor(RadarSensor(-180.0, 180.0, -90, 90, 0.5 * RADIUS, OBJECT_TYPE_SENSED))

    def sense(self, agent, observations):
        print 'SensorTestEnvironment.sense()', observations
        
    def step(self, agent, actions):
        print 'SensorTestEnvironment.step()', actions
        
    def is_active(self, agent):
        return True
        
    def is_episode_over(self, agent):
        if self.counter > N_STEPS:
            print 'SensorTestEnvironment: episode is over'
            self.counter = 0
            return True
        else:
            self.counter += 1

    def reset(self, agent):
        print 'SensorTestEnvironment.reset()'

    def cleanup(self):
        pass
