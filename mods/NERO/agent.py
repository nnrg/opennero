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
        self.info = OpenNero.AgentInitInfo(*self.agent_info_tuple())

    def get_team(self):
        return self.team

    def agent_info_tuple(self):
        abound = OpenNero.FeatureVectorInfo() # actions
        sbound = OpenNero.FeatureVectorInfo() # sensors

        # actions
        abound.add_continuous(-1, 1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1)

        return sbound, abound, module.rtneat_rewards()

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        return True

    def end(self, time, reward):
        return True

    def destroy(self):
        return True


class RTNEATAgent(NeroAgent, OpenNero.AgentBrain):
    """
    rtNEAT agent
    """

    ai = 'rtneat'

    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an
        # AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self)

    def get_org(self):
        """
        Returns the rtNEAT object for this agent
        """
        return OpenNero.get_ai("rtneat-%s" % self.get_team()).get_organism(self)

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
        OpenNero.get_ai("rtneat-%s" % self.get_team()).release_organism(self)
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
                self.state.label = ''.join('.' for i in range(int(5*OpenNero.get_environment().get_hitpoints(self))))
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
        Collect and interpret the outputs as valid actions
        """
        assert(len(sensors) == constants.N_SENSORS)

        org = self.get_org()
        org.time_alive += 1

        org.net.load_sensors(
            list(self.sensors.normalize(sensors)) + [constants.NEAT_BIAS])
        org.net.activate()
        outputs = org.net.get_outputs()

        actions = self.actions.get_instance()
        for i in range(len(self.actions.get_instance())):
             actions[i] = outputs[i]
        return self.actions.denormalize(actions)


class Turret(NeroAgent, OpenNero.AgentBrain):
    """
    Simple Rotating Turret
    """

    ai = None

    def __init__(self):
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self, team=constants.OBJECT_TYPE_TEAM_1, group='Turret')

    def start(self, time, sensors):
        self.state.label = 'Turret'
        a = self.actions.get_instance()
        a[0] = a[1] = 0
        return a

    def act(self, time, sensors, reward):
        a = self.actions.get_instance()
        a[0] = 0
        a[1] = 0.1
        return a


class QLearningAgent(NeroAgent, OpenNero.QLearningBrain):
    """
    QLearning agent.
    """

    ai = 'qlearning'

    def __init__(self, gamma=0.8, alpha=0.8, epsilon=0.1,
                 action_bins=3, state_bins=5,
                 num_tiles=0, num_weights=0):
        OpenNero.QLearningBrain.__init__(
            self, gamma, alpha, epsilon,
            action_bins, state_bins,
            num_tiles, num_weights)
        NeroAgent.__init__(self)

    def agent_info_tuple(self):
        sbound, abound, _ = NeroAgent.agent_info_tuple(self)
        rbound = OpenNero.FeatureVectorInfo() # single-dimension rewards
        rbound.add_continuous(-sys.float_info.max, sys.float_info.max)
        return sbound, abound, rbound
