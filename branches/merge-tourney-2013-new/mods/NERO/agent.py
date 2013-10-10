import sys
import tempfile

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
        abound.add_continuous(-constants.MAX_TURNING_RATE, constants.MAX_TURNING_RATE) # left/right turn (in radians)

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1)

        return sbound, abound, module.rtneat_rewards()

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        return True

    def end(self, time, reward):
        org = self.get_org()
        print "index: %d, fitness: %f" % (self.index, org.fitness)
        return True

    def destroy(self):
        env = OpenNero.get_environment()
        if env is not None:
            env.remove_agent(self)
        return True

        
class KeyboardAgent(NeroAgent, OpenNero.AgentBrain):
    keys = set([]) # keys pressed
    action_map = {'fwd':0, 'back':1, 'left':2, 'right':3}
    ACCELERATION = 0.1 # max acceleration of agent in one step
    DRAG_CONST = 0.005 # constant for calculating drag
    
    ai = None

    """
    Keyboard agent with simple ASWD(QE) controls
    """
    def __init__(self):
        """
        this is the constructor - it gets called when the brain object is first created
        """
        # call the parent constructor
        OpenNero.AgentBrain.__init__(self) # do not remove!
        NeroAgent.__init__(self, group='Turret')

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
        self.velocity = 0
        return self.key_action()

    def act(self, time, sensors, reward):
        """
        take in new sensors and reward from previous action and return the next action
        """
        return self.key_action()

    def end(self, time, reward):
        """
        take in last reward
        """
        # store last reward and add average reward to fitness (i.e. how good the agent was since
        # collecting the last cube)
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
                    theta = -0.1
                    accel = 1
                elif key == 'left':
                    theta = 0.1
                    accel = 1
                elif key == 'fwd':
                    accel = 1
                elif key == 'back':
                    accel = -1

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
        if vel > 1: vel = 1
        elif vel < -1: vel = -1

        # The x and y components of movement vector specify the action.
        actions[0] = 0.98*vel
        actions[1] = 0.98*theta
        self.velocity = actions[0]

        return actions

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
        if (org.time_alive > 1):
            module.getServer().write_data(self.stats())
        # at the beginning of a physical life cycle, print the organism deets
        # FIXME: this causes lots of giant files
        #tf = tempfile.NamedTemporaryFile(
        #    prefix='opennero-org-%05d-' % org.id,
        #    suffix='.xml',
        #    delete=False)
        #print >>tf, org        
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # return action
        return self.network_action(sensors)

    def end(self, time, reward):
        """
        end of an episode
        """
        OpenNero.get_ai("rtneat-%s" % self.get_team()).release_organism(self)
        return True

    def stats(self):
        org = self.get_org()
        stats = '<message><content class="edu.utexas.cs.nn.opennero.Genome"\n'
        stats += 'id="%d" bodyId="%d" fitness="%f" timeAlive="%d"' % (org.id, self.state.id, org.fitness, org.time_alive)
        stats += ' champ="%s">\n' % ('true' if org.champion else 'false')
        stats += '<rawFitness>\n'
        for d in constants.FITNESS_DIMENSIONS:
            dname = constants.FITNESS_NAMES[d]
            f = org.stats[constants.FITNESS_INDEX[d]]
            stats += '  <entry dimension="%s">%f</entry>\n' % (dname, f)
        stats += '</rawFitness></content></message>'
        return stats

    def set_display_hint(self):
        """
        set the display hint above the agent's head (toggled with F2)
        """
        display_hint = constants.getDisplayHint()
        if display_hint:
            org = self.get_org()
            if display_hint == 'fitness':
                self.state.label = '%.2f' % org.fitness
            elif display_hint == 'time alive':
                self.state.label = str(org.time_alive)
            elif display_hint == 'hit points':
                self.state.label = ''.join('.' for i in range(int(5*OpenNero.get_environment().get_hitpoints(self))))
            elif display_hint == 'id':
                self.state.label = str(org.id)
            elif display_hint == 'species id':
                self.state.label = str(org.species_id)
            elif display_hint == 'champion':
                if org.champion:
                    self.state.label = 'champ!'
                else:
                    self.state.label = ''
            elif display_hint == 'rank':
                self.state.label = str(org.rank)
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
        rbound.add_continuous(-sys.float_info.max, sys.float_info.max)
        return sbound, abound, rbound
