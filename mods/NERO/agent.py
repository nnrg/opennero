import sys
import json

import constants
import OpenNero

def factory_class(ai):
    return ai_map.get(ai, NEATAgent)

def factory(ai, *args):
    return factory_class(ai)(*args)

class AgentEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, NeroAgent):
            return {
                'agent_ai': inv_ai_map.get(type(obj), 'none'),
                '__str__': str(obj).split('\n') 
            }

class NeroAgent(object):
    """
    base class for nero agents
    """
    def __init__(self, team_type=None, group='Agent'):
        self.team_type = team_type
        self.group = group

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        self.rewards = init_info.sensors
        return True

    def destroy(self):
        return True

class NEATAgent(NeroAgent, OpenNero.AgentBrain):
    num_inputs = constants.N_SENSORS + 1
    num_outputs = constants.N_ACTIONS
    genome = OpenNero.Genome(num_inputs, num_outputs, 0, 0)

    def __init__(self, team_type=None, org=None):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an
        # AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)

        NeroAgent.__init__(self, team_type)
        self.omit_friend_sensors = False
        self.org = org

    def __str__(self):
        return str(self.org)

    def start(self, time, sensors):
        """
        start of an episode
        """
        self.org.time_alive += 1
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # return action
        self.org.fitness = self.fitness[0] / self.step
        return self.network_action(sensors)

    def set_display_hint(self):
        """
        set the display hint above the agent's head (toggled with F2)
        """
        display_hint = constants.getDisplayHint()
        if display_hint:
            if display_hint == 'fitness':
                self.state.label = '%.2f' % self.org.fitness
            elif display_hint == 'time alive':
                self.state.label = str(self.org.time_alive)
            elif display_hint == 'hit points':
                self.state.label = ''.join('.' for i in range(int(5*OpenNero.get_environment().get_hitpoints(self))))
            elif display_hint == 'id':
                self.state.label = str(self.org.genome.id)
            elif display_hint == 'champion':
                if self.org.champion:
                    self.state.label = 'champ!'
                else:
                    self.state.label = ''
            elif display_hint == 'rank':
                self.state.label = str(self.org.rank)
            elif display_hint == 'debug':
                self.state.label = str(OpenNero.get_environment().get_state(self))
            else:
                self.state.label = '?'
        else:
            # the first time we switch away from displaying stuff,
            # change the window caption
            if self.state.label:
                self.state.label = ""

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid actions
        """
        assert(len(sensors) == constants.N_SENSORS)

        self.org.time_alive += 1

        if self.omit_friend_sensors:
            for idx in constants.SENSOR_INDEX_FRIEND_RADAR:
                sensors[idx] = 0
        
        self.org.net.load_sensors(
            list(self.sensors.normalize(sensors)) + [constants.NEAT_BIAS])
        self.org.net.activate()

        actions = self.actions.get_instance()
        for i in range(len(self.actions.get_instance())):
             actions[i] = self.org.net.outputs[i].active_out
        #disabling firing for testing...
        #actions[constants.ACTION_INDEX_FIRE] = 0
        denormalized_actions = self.actions.denormalize(actions)

        if denormalized_actions[constants.ACTION_INDEX_ZERO_FRIEND_SENSORS] > 0.5:
            self.omit_friend_sensors = True
        else:
            self.omit_friend_sensors = False

        return denormalized_actions

    def is_episode_over(self):
        return self.org.eliminate

class QLearningAgent(NeroAgent, OpenNero.QLearningBrain):
    """
    QLearning agent.
    """
    def __init__(self, team_type=None, gamma=0.8, alpha=0.8, epsilon=0.1,
                 action_bins=3, state_bins=5,
                 num_tiles=0, num_weights=0):
        OpenNero.QLearningBrain.__init__(
            self, gamma, alpha, epsilon,
            action_bins, state_bins,
            num_tiles, num_weights)
        NeroAgent.__init__(self, team_type)
    
    def set_display_hint(self):
        """
        set the display hint above the agent's head (toggled with F2)
        """
        display_hint = constants.getDisplayHint()
        if display_hint:
            if display_hint == 'fitness':
                self.state.label = '%.2g' % self.fitness[0]
            elif display_hint == 'time alive':
                self.state.label = str(self.step)
            elif display_hint == 'hit points':
                self.state.label = ''.join('.' for i in range(int(5*OpenNero.get_environment().get_hitpoints(self))))
            elif display_hint == 'id':
                self.state.label = str(self.state.id)
            elif display_hint == 'debug':
                self.state.label = str(OpenNero.get_environment().get_state(self))
            else:
                self.state.label = '?'
        else:
            # the first time we switch away from displaying stuff,
            # change the window caption
            if self.state.label:
                self.state.label = ""

class Turret(NeroAgent, OpenNero.AgentBrain):
    """
    Simple Rotating Turret
    """
    def __init__(self, team_type=None):
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self, team_type, group='Turret')

    def start(self, time, sensors):
        self.state.label = 'Turret'
        a = self.actions.get_instance()
        a[0] = a[1] = a[2] = a[3] = 0
        return a

    def act(self, time, sensors, reward):
        a = self.actions.get_instance()
        a[0] = 0
        a[1] = 0.1
        a[2] = 1 
        a[3] = 0
        return a

ai_map = {
    'neat': NEATAgent,
    'qlearning': QLearningAgent,
    'turret': Turret
}

inv_ai_map = {v: k for k, v in ai_map.items()}
