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
        sensor_info = FeatureVectorInfo()
        self.expected = []
        for i in xrange(N_OBJECTS):
            a = radians(i * 360.0 / N_OBJECTS)
            cosa, sina = cos(a), sin(a)
            da0 = (i-0.5) * 360.0 / N_OBJECTS
            a0 = radians(da0)
            da1 = (i+0.5) * 360.0 / N_OBJECTS
            a1 = radians(da1)

            # N ray sensors that should be hitting the boxes
            agent.add_sensor(RaySensor(cosa, sina, 0, 2 * RADIUS, OBJECT_TYPE_SENSED))
            sensor_info.add_continuous(0,1)
            self.expected.append(0.5)

            # N ray sensors that should be too short
            agent.add_sensor(RaySensor(cosa, sina, 0, 0.5 * RADIUS, OBJECT_TYPE_SENSED))
            sensor_info.add_continuous(0,1)
            self.expected.append(1.0)

            # N ray sensors that should miss
            agent.add_sensor(RaySensor(cos(a0), sin(a0), 0, 2 * RADIUS, OBJECT_TYPE_SENSED))
            sensor_info.add_continuous(0,1)
            self.expected.append(1.0)

            # N radar sensors that should have 1 object each
            agent.add_sensor(RadarSensor(da0, da1, -90, 90, 1.1 * RADIUS, OBJECT_TYPE_SENSED))
            sensor_info.add_continuous(0,1)
            self.expected.append(0.5)

            # N radar sensors that should be too short
            agent.add_sensor(RadarSensor(da0, da1, -90, 90, 0.5 * RADIUS, OBJECT_TYPE_SENSED))
            sensor_info.add_continuous(0,1)
            self.expected.append(0)

        # a radar sensors that should have all the objects
        agent.add_sensor(RadarSensor(-180.0, 180.0, -90, 90, 2 * RADIUS, OBJECT_TYPE_SENSED))
        sensor_info.add_continuous(0,1)
        self.expected.append(1)

        # this is a special ray sensor that should see the forward object
        agent.add_sensor(RaySensor(1, 0, 0, 0.67 * RADIUS, OBJECT_TYPE_FORWARD))
        sensor_info.add_continuous(0,1)
        self.expected.append(0.5)

        # this is a special radar sensor that should see the forward object
        agent.add_sensor(RadarSensor(-5, 5, -90, 90, 0.67 * RADIUS, OBJECT_TYPE_FORWARD))
        sensor_info.add_continuous(0,1)
        self.expected.append(0.5)

        reward_info = FeatureVectorInfo()
        reward_info.add_continuous(0,1)
        return AgentInitInfo(sensor_info, sensor_info, reward_info)

    def sense(self, agent, observations):
        print 'SensorTestEnvironment.sense()'
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
            print ' ', ' '.join([str(c) for c in correct_list])
            print ' ', ' '.join([str(o) for o in observations])
            print ' ', ' '.join([str(e) for e in self.expected])
        return observations
        
    def step(self, agent, actions):
        print 'SensorTestEnvironment.step()'
        return 1.0
        
    def is_active(self, agent):
        return True
        
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
