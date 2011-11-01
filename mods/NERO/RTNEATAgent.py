from OpenNero import *
import random
from time import time
from constants import *

def gettime():
    return time()

class RTNEATAgent(AgentBrain):
    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an
        # AgentBrainPtr by C++
        AgentBrain.__init__(self)
        self.team = 0

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        self.group = "Agent"
        return True

    def get_org(self):
        """
        Returns the rtNEAT object for this agent
        """
        rtneat = get_ai("rtneat")
        return rtneat.get_organism(self)

    def start(self, time, sensors):
        """
        start of an episode
        """
        org = self.get_org()
        org.time_alive += 1
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        self.set_display_hint()
        # return action
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        get_ai("rtneat").release_organism(self)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True

    def set_display_hint(self):
        """
        set the display hint above the agent's head (toggled with F2)
        """
        display_hint = getDisplayHint()
        if display_hint:
            org = self.get_org()
            if display_hint == 'fitness':
                self.state.label = ' '.join(['%.02g' % x for x in self.fitness])
                setWindowCaption("Displaying Fitness")
            elif display_hint == 'time alive':
                self.state.label = str(org.time_alive)
                setWindowCaption("Displaying Time Alive")
            elif display_hint == 'hit points':
                self.state.label == 'hit points'
                setWindowCaption("Displaying Hit Points")
            elif display_hint == 'genome id':
                self.state.label = str(org.id)
                setWindowCaption("Displaying Organism ID")
            elif display_hint == 'species id':
                self.state.label = str(org.species_id)
                setWindowCaption("Displaying Species ID")
            elif display_hint == 'champion':
                if org.champion:
                    self.state.label = 'champ!'
                else:
                    self.state.label = ''
                setWindowCaption("Displaying Champion")
            elif display_hint == 'rank':
                self.state.label = str(org.rank)
                setWindowCaption("Displaying Rank")
            elif display_hint == 'debug':
                self.state.label = str(get_environment().get_state(self))
                setWindowCaption("Displaying Debug")
            else:
                self.state.label = '?'
        else:
            self.state.label = ""
            setWindowCaption("")

    def get_team(self):
        return self.team

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        # make sure we have the right number of sensors
        assert(len(sensors)==N_SENSORS)
        # convert the sensors into the [0.0, 1.0] range
        sensors = self.sensors.normalize(sensors)
        # create the list of sensors
        inputs = [sensor for sensor in sensors]
        # add the bias value
        inputs.append(NEAT_BIAS)
        # get the current organism we are using
        org = self.get_org()
        # increment the lifetime counter
        org.time_alive += 1
        # get the current network we are using
        net = org.net
        # load the list of sensors into the network input layer
        net.load_sensors(inputs)
        # activate the network
        net.activate()
        # get the list of network outputs
        outputs = net.get_outputs()

        # create a C++ vector for action values
        actions = self.actions.get_instance()
        # assign network outputs to action vector
        for i in range(0,len(self.actions.get_instance())):
            actions[i] = outputs[i]
        # convert the action vector back from [0.0, 1.0] range
        actions = self.actions.denormalize(actions)
        #print "in:", inputs, "out:", outputs, "a:", actions
        return actions
