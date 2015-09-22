import sys
import random
import tempfile

import constants
import module
import OpenNero

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.pose = (0, 0, 0)  # current x, y, heading
        self.prev_pose = (0, 0, 0)
        self.initial_position = OpenNero.Vector3f(0, 0, 0)
        self.initial_rotation = OpenNero.Vector3f(0, 0, 0)
        self.total_damage = 0
        self.curr_damage = 0

    def __str__(self):
        x, y, h = self.pose
        px, py, ph = self.prev_pose
        return 'agent { id: %d, pose: (%.02f, %.02f, %.02f), prev_pose: (%.02f, %.02f, %.02f) }' % \
            (self.id, x, y, h, px, py, ph)

    def randomize(self, x, y):
        dx = random.randrange(constants.SPAWN_RANGE * 2) - constants.SPAWN_RANGE
        dy = random.randrange(constants.SPAWN_RANGE * 2) - constants.SPAWN_RANGE
        self.initial_position.x = x + dx
        self.initial_position.y = y + dy
        self.prev_pose = self.pose = (self.initial_position.x,
                                      self.initial_position.y,
                                      self.initial_rotation.z)

    def reset_pose(self, position, rotation):
        self.prev_pose = self.pose = (position.x, position.y, rotation.z)

    def update_damage(self):
        """
        Update the damage for an agent, returning the current damage.
        """
        self.total_damage += self.curr_damage
        damage = self.curr_damage
        self.curr_damage = 0
        return damage

    def update_pose(self, move_by, turn_by):
        dist = constants.MAX_MOVEMENT_SPEED * move_by
        heading = common.wrap_degrees(self.agent.state.rotation.z, turn_by)
        x = self.agent.state.position.x + dist * math.cos(math.radians(heading))
        y = self.agent.state.position.y + dist * math.sin(math.radians(heading))

        self.prev_pose = self.pose
        self.pose = (x, y, heading)

        # try to update position
        pos = copy.copy(self.agent.state.position)
        pos.x = x
        pos.y = y
        self.agent.state.position = pos

        # try to update rotation
        rot = copy.copy(self.agent.state.rotation)
        rot.z = heading
        self.agent.state.rotation = rot

class NeroAgent(object):
    """
    base class for nero agents
    """
    @staticmethod
    def factory_class(ai):
        ai_map = {
            'neat': NEATAgent,
            'qlearning': QLearningAgent
        }
        return ai_map.get(ai, NEATAgent)

    @staticmethod
    def factory(ai, *args):
        return NeroAgent.factory_class(ai)(*args)

    def __init__(self, team_type=None, group='Agent'):
        self.team_type = team_type
        self.group = group
        self.info = OpenNero.AgentInitInfo(*self.agent_info_tuple())
        self.mod_state = AgentState()

    def agent_info_tuple(self):
        abound = OpenNero.FeatureVectorInfo() # actions
        sbound = OpenNero.FeatureVectorInfo() # sensors

        # actions
        abound.add_continuous(-1, 1) # forward/backward speed
        abound.add_continuous(-constants.MAX_TURNING_RATE, constants.MAX_TURNING_RATE) # left/right turn (in radians)
        abound.add_continuous(0, 1) # fire 
        abound.add_continuous(0, 1) # omit friend sensors 

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1)

        rbound = OpenNero.FeatureVectorInfo()
        for f in constants.FITNESS_DIMENSIONS:
            rbound.add_continuous(0, 1)

        return sbound, abound, rbound

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        self.rewards = init_info.sensors
        return True

    def destroy(self):
        mod = module.getMod()
        if mod is not None:
            mod.remove_agent(self)
        return True

    def get_team(self):
        mod = module.getMod()
        if mod is not None:
            return mod.teams[self.team_type]

    def randomize(self):
        env = OpenNero.get_environment()
        x = env.spawn_x[self.team_type]
        y = env.spawn_y[self.team_type]
        self.mod_state.randomize(x, y)

class NEATAgent(NeroAgent, OpenNero.AgentBrain):

    def __init__(self, *args, **kwargs):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an
        # AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)

        NeroAgent.__init__(self, *args, **kwargs)
        self.omit_friend_sensors = False
        #TODO org init

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
        return self.network_action(sensors)

    def stats(self):
        stats = '<message><content class="edu.utexas.cs.nn.opennero.Genome"\n'
        stats += 'id="%d" bodyId="%d" fitness="%f" timeAlive="%d"' % (org.id, self.state.id, org.fitness, org.time_alive)
        stats += ' champ="%s">\n' % ('true' if org.champion else 'false')
        stats += '<rawFitness>\n'
        for d in constants.FITNESS_DIMENSIONS:
            dname = constants.FITNESS_NAMES[d]
            f = self.org.stats[constants.FITNESS_INDEX[d]]
            stats += '  <entry dimension="%s">%f</entry>\n' % (dname, f)
        stats += '</rawFitness></content></message>'
        return stats

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
                self.state.label = str(self.org.id)
            elif display_hint == 'species id':
                self.state.label = str(self.org.species_id)
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
        outputs = self.org.net.get_outputs()

        actions = self.actions.get_instance()
        for i in range(len(self.actions.get_instance())):
             actions[i] = outputs[i]
        #disabling firing for testing...
        #actions[constants.ACTION_INDEX_FIRE] = 0
        denormalized_actions = self.actions.denormalize(actions)

        if denormalized_actions[constants.ACTION_INDEX_ZERO_FRIEND_SENSORS] > 0.5:
            self.omit_friend_sensors = True
        else:
            self.omit_friend_sensors = False

        return denormalized_actions

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
            elif display_hint == 'species id':
                self.state.label = 'q'
            elif display_hint == 'debug':
                self.state.label = str(OpenNero.get_environment().get_state(self))
            else:
                self.state.label = '?'
        else:
            # the first time we switch away from displaying stuff,
            # change the window caption
            if self.state.label:
                self.state.label = ""

    def agent_info_tuple(self):
        sbound, abound, _ = NeroAgent.agent_info_tuple(self)
        rbound = OpenNero.FeatureVectorInfo() # single-dimension rewards
        rbound.add_continuous(0, 1)
        return sbound, abound, rbound

class Turret(NeroAgent, OpenNero.AgentBrain):
    """
    Simple Rotating Turret
    """
    def __init__(self):
        OpenNero.AgentBrain.__init__(self)
        NeroAgent.__init__(self, team_type=constants.OBJECT_TYPE_TEAM_1, group='Turret')

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
