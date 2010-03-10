from OpenNero import *
import time
import random
import math
import action_script
from Queue import Queue

XDIM = 93
YDIM = 93
MOE = 0      # Margin Of Error
WAIT_TIME = 0.1
N_FIXED_SENSORS = 3     # 0: wall bump, 1: self position X

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
        
          
    def initialize(self, init_info):
        """
        init_info contains a description of the observations the agent will see
        and the actions the agent will be able to perform
        """
        self.constraints = init_info.actions
        self.action_sq = Queue()

        self.step_count = 0
        self.reward = 0

        return True

    def start(self, time, sensors):
        """
        Take in the initial sensors and return the first action
        """
        return self.act(time, sensors, 0)
        
    def act(self, time, sensors, reward):
        """
        Take in new sensors and reward from previous action and return the next action
        """
        # if I'm ready and it's my turn, act
        self.plan(time, sensors, reward)
        action = self.constraints.get_instance()
        action[0] = self.action_sq.get_nowait()
        return action
        
    def plan(self, time, sensors, reward):
        """ Plan for the next actions and enqueue them in action_sq (short for
        action sequence)
        """
        
        # Read position of self along x, y axes
        pos_x = (sensors[1])    
        pos_y = (sensors[2])

        # Re-initialize dictionary of remaining crumbs
        distances = {}

        # Read information about all crumbs
        crumbs = self.map_crumbs(sensors, 4, N_FIXED_SENSORS)

        # Calculate the distances to all remaining crumbs and put them in dictionary
        for crumb_data in crumbs:
            if (crumb_data[2] == 1):         # meaning: the crumb is there
                # calculate distance to every crumb
                dist = math.hypot((crumb_data[0] - pos_x), (crumb_data[1] - pos_y))

                # tack distance to map as value
                distances[(crumb_data[0], crumb_data[1])] = dist

        # find closest crumb
        list_crumbs = distances.items()   # extract list of crumbs with distance
        
        if len(list_crumbs) > 0: # make sure there are crumbs
            target = list_crumbs[0] # closest crumb: target
            for crumb in list_crumbs:
                if (crumb[1] < target[1]):
                    target = crumb

            # book-keeping
            self.reward = self.reward + reward
            self.step_count += 1

            # go to target
            x_dist = target[1]
            if (x_dist > MOE):
                action = action_script.go_xy(sensors, target[0][0], target[0][1])
                self.action_sq.put_nowait(action)
        else:
            self.action_sq.put_nowait(random.random() * math.pi)

        return True

    def map_crumbs(self, sensors, num_sensors, start_sensor):
        """Map the sensor information into 4 x n raw matrix. Each row should
        contains, in order: x position of crumb, y position of crumb, whether
        crumb is present at that location (i.e. whether it has already been
        vacced), and the reward for vaccing that crumb.
        Params:
            sensors: the range of sensors we're reading from
            num_sensors: the number of relevant sensors in a block. In this case,
            four.
            start_sensor: the # of the first relevant sensor
        """
        crumbs = [[]]   # 4 x n raw matrix
        row = 0
        for i in range(start_sensor, len(sensors)):
            crumbs[row].append(sensors[i])
            count = (i - start_sensor) + 1
            if (count > 0 and count % num_sensors == 0):
                crumbs.append([])
                row += 1
        return crumbs[0:-1]

    def end(self, time, reward):
        """
        take in final reward
        """
        self.reward += reward
        return True

    def destroy(self):
        """
        called when the agent is destroyed
        """
        self.time = 0
        return True
