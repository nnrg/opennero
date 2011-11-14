import sys

import constants
import module
import OpenNero


class NeroAgent(object):
    """
    base class for nero agents
    """
    def __init__(self, team=None, group='Agent'):
        self.team = team or module.getMod().curr_team
        self.group = group
        self.info = self.create_agent_info()

    def get_team(self):
        return self.team

    def create_agent_info(self):
        abound = OpenNero.FeatureVectorInfo() # actions
        sbound = OpenNero.FeatureVectorInfo() # sensors
        rbound = OpenNero.FeatureVectorInfo() # rewards

        # actions
        abound.add_continuous(-1, 1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1);

        # Rewards
        for f in constants.FITNESS_DIMENSIONS:
            rbound.add_continuous(-sys.float_info.max, sys.float_info.max)

        return OpenNero.AgentInitInfo(sbound, abound, rbound)

    def end(self, time, reward):
        return True

    def destroy(self):
        return True


class RTNEATAgent(OpenNero.AgentBrain, NeroAgent):
    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an
        # AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self)

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
        return OpenNero.get_ai("rtneat-%s" % self.team).get_organism(self)

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
        OpenNero.get_ai("rtneat-%s" % self.team).release_organism(self)
        return True

    def set_display_hint(self):
        """
        set the display hint above the agent's head (toggled with F2)
        """
        display_hint = constants.getDisplayHint()
        if display_hint:
            org = self.get_org()
            if display_hint == 'fitness':
                self.state.label = ' '.join(['%.02g' % x for x in self.fitness])
                OpenNero.setWindowCaption("Displaying Fitness")
            elif display_hint == 'time alive':
                self.state.label = str(org.time_alive)
                OpenNero.setWindowCaption("Displaying Time Alive")
            elif display_hint == 'hit points':
                self.state.label = 'hp'
                OpenNero.setWindowCaption("Displaying Hit Points")
            elif display_hint == 'genome id':
                self.state.label = str(org.id)
                OpenNero.setWindowCaption("Displaying Organism ID")
            elif display_hint == 'species id':
                self.state.label = str(org.species_id)
                OpenNero.setWindowCaption("Displaying Species ID")
            elif display_hint == 'champion':
                if org.champion:
                    self.state.label = 'champ!'
                else:
                    self.state.label = ''
                OpenNero.setWindowCaption("Displaying Champion")
            elif display_hint == 'rank':
                self.state.label = str(org.rank)
                OpenNero.setWindowCaption("Displaying Rank")
            elif display_hint == 'debug':
                self.state.label = str(OpenNero.get_environment().get_state(self))
                OpenNero.setWindowCaption("Displaying Debug")
            else:
                self.state.label = '?'
        else:
            self.state.label = ""
            OpenNero.setWindowCaption("")

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        # make sure we have the right number of sensors
        assert(len(sensors) == constants.N_SENSORS)
        # convert the sensors into the [0.0, 1.0] range
        sensors = self.sensors.normalize(sensors)
        # create the list of sensors
        inputs = [sensor for sensor in sensors]
        # add the bias value
        inputs.append(constants.NEAT_BIAS)
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


class Turret(OpenNero.AgentBrain, NeroAgent):
    """
    Simple Rotating Turret
    """
    def __init__(self):
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self, team=constants.OBJECT_TYPE_TEAM_1, group='Turret')

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        self.previous_fire =  0
        return True

    def start(self, time, sensors):
        self.org = None
        self.net = None
        self.state.label = "Turret"
        a = self.actions.get_instance()
        for x in range(len(self.actions)):
            a[x] = 0
            if x == 1:
              a[x] = 0
        return a

    def act(self, time, sensors, reward):
        a = self.actions.get_instance()
        a[0] = 0
        a[1] = .15
        return a


class QLearningAgent(OpenNero.QLearningBrain, NeroAgent):
    def __init__(self, discount=0.8, learning_rate=0.8, epsilon=0.1, num_actions=3, num_states=5):
        OpenNero.QLearningBrain.__init__(
            self, discount, learning_rate, epsilon, num_actions, num_states)
        NeroAgent.__init__(self)

    def create_agent_info(self):
        abound = OpenNero.FeatureVectorInfo() # actions
        sbound = OpenNero.FeatureVectorInfo() # sensors
        rbound = OpenNero.FeatureVectorInfo() # rewards

        # actions
        abound.add_continuous(-1, 1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1);

        # Rewards -- just one dimension for reinforcement learning
        rbound.add_continuous(-sys.float_info.max, sys.float_info.max)

        return OpenNero.AgentInitInfo(sbound, abound, rbound)
