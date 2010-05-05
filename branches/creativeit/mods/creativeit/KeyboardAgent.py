from OpenNero import *
import random
from math import *
import getopt

class KeyboardAgent(AgentBrain):
    keys = set([]) # keys pressed
    action_map = {'fwd':0, 'back':1, 'left':2, 'right':3}
    ACCELERATION = 0.025 # max acceleration of agent in one step
    DRAG_CONST = 0.005 # constant for calculating drag
    
    """
    Keyboard agent with simple ASWD(QE) controls
    """
    def __init__(self):
        """
        this is the constructor - it gets called when the brain object is first created
        """
        # call the parent constructor
        AgentBrain.__init__(self) # do not remove!
        
    def initialize(self, init_info):
        """
        called when the agent is first created
        """
        self.actions = init_info.actions
        return True
        
    def start(self, time, sensors):
        """
        take in the initial sensors and return the first action
        """
        self.reward = 0
        self.count = 0
        self.fitness = 0
        self.velocity = 0
        return self.key_action()
        
    def act(self, time, sensors, reward):
        """
        take in new sensors and reward from previous action and return the next action
        """
        # Store fitness if a cube was collected, otherwise accumulate reward.
        if reward >= 1:
            self.fitness += reward
            self.reward = 0
            self.count = 0
        else:
            self.reward += reward
            self.count += 1
        return self.key_action()
        
    def end(self, time, reward):
        """
        take in last reward
        """
        # store last reward and add average reward to fitness (i.e. how good the agent was since
        # collecting the last cube)
        self.reward += reward
        self.count += 1
        self.fitness += self.reward/self.count
        print  "Final reward: %f, fitness: %f" % (reward, self.fitness)
        return True

    def destroy(self):
        """
        called when the agent is destroyed
        """
        return True

    def key_action(self):
        actions = self.actions.get_instance()
        assert(len(actions) == 2)
        
        accel = 0
        theta = 0
        if len(KeyboardAgent.keys) > 0:
            # Keys specify movement relative to screen coordinates.
            for key in KeyboardAgent.keys:
                if key == 'right':
                    theta = -0.5
                    accel = 1
                elif key == 'left':
                    theta = 0.5
                    accel = 1
                elif key == 'fwd':
                    accel = 1
                elif key == 'back':
                    accel = 1
     
            print "keys: ", ' '.join(KeyboardAgent.keys)
            KeyboardAgent.keys.clear()

        dvel = accel*self.ACCELERATION
        vel = self.velocity

        # Calculate drag assuming viscous resistance (drag = -velocity*const). 
        drag = -vel*self.DRAG_CONST 
        # If no other force is acting, drag cannot change sign of velocity. 
        if vel*(vel+drag) < 0: drag = -vel 
        # Calculate new velocity.
        vel += dvel + drag 
        if vel > 0.5: vel = 0.5
        elif vel < -0.5: vel = -0.5

        # The x and y components of movement vector specify the action.
        actions[0] = 0.98*vel
        actions[1] = 0.98*theta
        self.velocity = actions[0]

        return actions
    
