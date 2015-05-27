from OpenNero import *
import time
import random
import math
import action_script
from Queue import Queue
from constants import *


class TabularRLAgent(AgentBrain):
    """
    Tabular RL Agent
    """
    def __init__(self): #, gamma, alpha, epsilon
        """
        Constructor that is called from the robot XML file.
        Parameters:
        @param gamma reward discount factor (between 0 and 1)
        @param alpha learning rate (between 0 and 1)
        @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        """
        AgentBrain.__init__(self) # initialize the superclass
        self.gamma = 0.5 
        self.alpha = 0.1
        self.epsilon = 0.1
        self.action_sq = Queue()
        """
        Our Q-function table. Maps from a tuple of sensors (state) to 
        another map of actions to Q-values. To look up a Q-value, call the predict method.
        """
        self.Q = {} # our Q-function table
        self.log = open("Tabular.log", "wb+")
        self.accreward = 0
        
    
    '''
    def __str__(self):
        return self.__class__.__name__ + \
            ' with gamma: %g, alpha: %g, epsilon: %g' \
            % (self.gamma, self.alpha, self.epsilon)
    '''
    
    def initialize(self, init_info):
        """
        Create a new agent using the init_info sent by the environment
        """
        self.action_info = init_info.actions
        self.sensor_info = init_info.sensors
        return True
    
    def predict(self, sensors, action):
        """
        Look up the Q-value for the given state (sensors), action pair.
        """
        o = tuple([x for x in sensors])
        
        # -----------------------------------------------------------------
        # TODO: tilting code here
        

        # -----------------------------------------------------------------

        #print "predict: " + str(o)
        if o not in self.Q:
            return 0
        else:
            #print "predict returning :" + str(self.Q[o][action])
            return self.Q[o][action]
    
    def update(self, sensors, action, new_value):
        """
        Update the Q-function table with the new value for the (state, action) pair
        and update the blocks drawing.
        """
        o = tuple([x for x in sensors])  

        actions = self.get_possible_actions(sensors)
        if o not in self.Q:
            self.Q[o] = [0 for a in actions]
        self.Q[o][action] = new_value
        self.draw_q(o)
    
    def draw_q(self, o):
        e = get_environment()
        if hasattr(e, 'draw_q'):
            e.draw_q(o, self.Q)

    def get_possible_actions(self, sensors):
        """
        Get the possible actions that can be taken given the state (sensors)
        """
        aMin = self.action_info.min(0)
        aMax = self.action_info.max(0)
        actions = range(int(aMin), int(aMax+1))
        #actions = [-math.pi/2, 0, math.pi/2, math.pi]
        actions = range(4)
        return actions
        
    def get_max_action(self, sensors):
        """
        get the action that is currently estimated to produce the highest Q
        """
        actions = self.get_possible_actions(sensors)
        max_action = actions[0]
        max_value = self.predict(sensors, max_action)
        for a in actions[1:]:
            value = self.predict(sensors, a)
            if value > max_value:
                max_value = value
                max_action = a
        return (max_action, max_value)

    def get_epsilon_greedy(self, sensors, max_action = None, max_value = None):
        """
        get the epsilon-greedy action
        """
        actions = self.get_possible_actions(sensors)
        if random.random() < self.epsilon: # epsilon of the time, act randomly
            return random.choice(actions)
        elif max_action is not None and max_value is not None:
            # we already know the max action
            return max_action
        else:
            # we need to get the max action
            (max_action, max_value) = self.get_max_action(sensors)
            return max_action
    
    def start(self, time, sensors):
        """
        Called to figure out the first action given the first sensors
        @param time current time
        @param sensors a DoubleVector of sensors for the agent (use len() and [])
        """
        self.previous_sensors = sensors
        self.previous_action = self.get_epsilon_greedy(sensors)
        return self.previous_action

    def act(self, time, sensors, reward):
        """
        return an action given the reward for the previous action and the new sensors
        @param time current time
        @param sensors a DoubleVector of sensors for the agent (use len() and [])
        @param the reward for the agent
        """
        # get the reward from the previous action
        r = reward[0]
        if r > 0: 
            self.accreward += r
        
        # get the updated epsilon, in case the slider was changed by the user
        # self.epsilon = get_environment().epsilon
        
        # get the old Q value
        Q_old = self.predict(self.previous_sensors, self.previous_action)
        
        # get the max expected value for our possible actions
        (max_action, max_value) = self.get_max_action(sensors)
        
        # update the Q value
        self.update( \
            self.previous_sensors, \
            self.previous_action, \
            Q_old + self.alpha * (r + self.gamma * max_value - Q_old) )
        
        # select the action to take
        action = self.get_epsilon_greedy(sensors, max_action, max_value)
        self.previous_sensors = sensors
        self.previous_action = action
        #print "myaction: ", action, "myreward: ", reward
        return (action-1)*(math.pi/2)

    def end(self, time, reward):
        """
        receive the reward for the last observation
        """
        # get the reward from the last action
        r = reward[0]
        o = self.previous_sensors
        a = self.previous_action

        # get the updated epsilon, in case the slider was changed by the user
        #self.epsilon = get_environment().epsilon

        # Update the Q value
        Q_old = self.predict(o, a)
        q = self.update(o, a, Q_old + self.alpha * (r - Q_old) )
        return True

    def destroy(self):
        self.log.close()
