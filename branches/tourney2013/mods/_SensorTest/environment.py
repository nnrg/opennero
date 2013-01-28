from OpenNero import *
from common import *
from constants import *
from math import *

class SensorTestEnvironment(Environment):

    def __init__(self):
        Environment.__init__(self)
        self.counter = 0
        self.sensor_info = FeatureVectorInfo()
        self.expected = []
        self.sensor_names = []
        print 'SensorTestEnvironment()'

    def add_ray(self, agent, x, y, z, radius, type_mask, expected):
        sensor = RaySensor(x,y,z,radius,type_mask, True)
        agent.add_sensor(sensor)
        self.sensor_info.add_continuous(0,1)
        self.expected.append(expected)
        self.sensor_names.append(str(sensor))

    def add_radar(self, agent, left_bound, right_bound, bottom_bound, top_bound, radius, type_mask, expected):
        sensor = RadarSensor(left_bound, right_bound, bottom_bound, top_bound, radius, type_mask, True)
        agent.add_sensor(sensor)
        self.sensor_info.add_continuous(0,1)
        self.expected.append(expected)
        self.sensor_names.append(str(sensor))

    def get_agent_info(self, agent):
        print 'SensorTestEnvironment.get_agent_info()'
        self.expected = []
        for i in xrange(N_OBJECTS):
            a = radians(i * 360.0 / N_OBJECTS)
            cosa, sina = cos(a), sin(a)
            da0 = (i-0.5) * 360.0 / N_OBJECTS
            a0 = radians(da0)
            da1 = (i+0.5) * 360.0 / N_OBJECTS
            a1 = radians(da1)

            # N ray sensors that should be hitting the boxes
            self.add_ray(agent, cosa, sina, 0, 2*RADIUS, OBJECT_TYPE_SENSED, 0.5)

            # N ray sensors that should be too short
            self.add_ray(agent, cosa, sina, 0, 0.5 * RADIUS, OBJECT_TYPE_SENSED, 1.0)

            # N ray sensors that should miss
            self.add_ray(agent, cos(a0), sin(a0), 0, 2 * RADIUS, OBJECT_TYPE_SENSED, 1.0)

            # N radar sensors that should have 1 object each
            self.add_radar(agent, da0, da1, -90, 90, 1.1 * RADIUS, OBJECT_TYPE_SENSED, 0.5)

            # N radar sensors that should be too short
            self.add_radar(agent, da0, da1, -90, 90, 0.5 * RADIUS, OBJECT_TYPE_SENSED, 0.0)

        # a radar sensors that should have all the objects
        self.add_radar(agent, -180.0, 180.0, -90, 90, 2 * RADIUS, OBJECT_TYPE_SENSED, 1.0)

        # this is a special ray sensor that should see the forward object
        self.add_ray(agent, 1, 0, 0, 0.67 * RADIUS, OBJECT_TYPE_FORWARD, 0.5)

        # this is a special radar sensor that should see the forward object
        self.add_radar(agent, -5, 5, -90, 90, 0.67 * RADIUS, OBJECT_TYPE_FORWARD, 0.5)

        reward_info = FeatureVectorInfo()
        reward_info.add_continuous(0,1)
        return AgentInitInfo(self.sensor_info, self.sensor_info, reward_info)

    def sense(self, agent, observations):
        correct_list = []
        for i in xrange(len(self.expected)):
            if self.expected[i] == 0.5:
                # we should see something, but not saturate
                correct_list.append( observations[i] > 0 and observations[i] < 1 )
            else:
                correct_list.append( observations[i] == self.expected[i] )
        correct = reduce(lambda x,y: x and y, correct_list)
        if not correct:
            print 'SENSOR ERROR:'
        for i in xrange(len(correct_list)):
            if not correct_list[i]:
                print self.sensor_names[i], ', expected:', self.expected[i], ', observed:', observations[i]
        return observations
        
    def step(self, agent, actions):
        return 1.0
        
    def is_episode_over(self, agent):
        if self.counter > N_STEPS:
            print 'SensorTestEnvironment: episode is over'
            self.counter = 0
            return True
        else:
            self.counter += 1
        return False

    def reset(self, agent):
        print 'SensorTestEnvironment.reset()'

    def cleanup(self):
        pass
