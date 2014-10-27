import sys
import random
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
        abound.add_continuous(0, 1) # fire 
        abound.add_continuous(0, 1) # omit friend sensors 

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
        # if a trace has been loaded, then process it here
        if self.environment.use_trace:
            trfitness = self.evaluate_trace()  # Calculate trace fitness and passed steps
            if self.environment.run_backprop:  # backprop invalidates org.fitness because it changes network
                if self.index != 0:  # run backprop only if the agent is not the elite organism
                    self.backprop_trace()
                    trfitness = self.evaluate_trace()  # Calculate trace fitness again since backprop changes network
                org.fitness = trfitness
                print "index: %d, trace fitness: %f" % (self.index, trfitness)
            else:
                org.fitness = floor(org.fitness if org.fitness > 0.0 else 0.0) + trfitness
                print "index: %d, trace fitness: %f, combined fitness: %f" % (self.index, trfitness, org.fitness)
        else:
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
        self.omit_friend_sensors = False

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

        if self.omit_friend_sensors:
            for idx in constants.SENSOR_INDEX_FRIEND_RADAR:
                sensors[idx] = 0
        
        org.net.load_sensors(
            list(self.sensors.normalize(sensors)) + [constants.NEAT_BIAS])
        org.net.activate()
        outputs = org.net.get_outputs()

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

    def evaluate_trace(self):
        """
        evaluate agent and compute fitness based on trace information
        """

        # flush network from previous activations
        org = self.get_org()
        org.net.flush()
        
        environment = OpenNero.get_environment()
        trace = environment.trace

        # place the agent at the beginning of the trace
        self.state.position = OpenNero.Vector3f(
            trace.position[0].x, trace.position[0].y, trace.position[0].z)
        self.state.rotation = OpenNero.Vector3f(
            trace.rotation[0].x, trace.rotation[0].y, trace.rotation[0].z)
        current_step = trace.initial_step
        j = 0  # trace index at which error is calculated
        while j < len(trace.position)-1 and current_step < environment.STEPS_PER_EPISODE:
            self.state.position = position
            self.state.rotation = rotation
            sensors = environment.sense(self)
            actions = self.network_action(sensors)

            # error based on position - find index in trace where error based on
            # current position starts to increase, i.e. starting with the current
            # trace index, we find position in trace that is closest to the current
            # state position.
            error1 = trace.position[j].getDistanceFrom(position)
            error2 = trace.position[j+1].getDistanceFrom(position)
            while error1 >= error2 and j < len(trace.position)-2:
                j += 1
                error1 = error2
                error2 = trace.position[j+1].getDistanceFrom(position)

            if error1 > self.ERROR_THRESHOLD:
                break

            # calculate new position, orientation, and velocity
            self.environment.act(self, actions)
            current_step += 1

        self.passed_steps = j
        return float(j)/len(trace.position)


    def backprop_trace(self):
        """
        adjust network weights using backprop and position information in the trace
        """
        self.net.flush()
        state = self.environment.get_new_state()
        trace = self.environment.trace
        state.position = Vector3f(trace.position[self.passed_steps].x, trace.position[self.passed_steps].y, trace.position[self.passed_steps].z)
        state.rotation = Vector3f(trace.rotation[self.passed_steps].x, trace.rotation[self.passed_steps].y, trace.rotation[self.passed_steps].z)
        state.current_step = trace.initial_step + self.passed_steps
        i = self.passed_steps
        j = 0
        while i < len(trace.position) and j < self.BACKPROP_WINDOW_SIZE and state.current_step < self.environment.STEPS_PER_EPISODE:
            # activate network only if at least one action in the trace is non-zero
            if reduce(lambda x, y: x or y != 0, trace.actions[i], False):
                sensors = self.environment.compute_sensors(state)
                actions = self.network_action(sensors)

                # TODO: will the scaling of actions in network_action() affect the implementation below?
                # e.g. does it change the derivative at the output nodes?

                # Don't run backprop on the first iteration, i.e. before the agent moves
                if i > self.passed_steps:
                    # calculate desired actions based on current state of agent and position in trace
                    # x-y actions
                    #poserr = [trace.position[i].x - state.position.x, trace.position[i].y - state.position.y]
                    #desired_actions = [poserr[0]/self.environment.VELOCITY, poserr[1]/self.environment.VELOCITY]

                    # r-theta actions
                    poserrx = trace.position[i].x - state.position.x
                    poserry = trace.position[i].y - state.position.y
                    errpos = sqrt(poserrx*poserrx + poserry*poserry)
                    errpos = errpos/self.environment.VELOCITY
                    errtheta = degrees(atan2(poserry, poserrx)) - state.rotation.z
                    errtheta = errtheta - floor(errtheta/360)*360  # range [0, 360)
                    if errtheta > 180: errtheta = errtheta - 360   # range [-180, 180]
                    errtheta = errtheta/(2*self.environment.ANGULAR_VELOCITY)
                    desired_actions = [errpos, errtheta]

                    #for j in range(len(desired_actions)):
                    #    if desired_actions[j] < -0.5: desired_actions[j] = -0.5
                    #    elif desired_actions[j] > 0.5: desired_actions[j] = 0.5

                    # calculate error based on desired action and network output
                    error = [desired_actions[0] - actions[0], desired_actions[1] - actions[1]]

                    self.net.load_errors(error)
                    self.net.backprop()

                # calculate new position, orientation, and velocity
                self.environment.perform_actions(state, actions)
                state.current_step += 1
                j += 1

            i += 1

        self.org.update_genotype()


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
        a[0] = a[1] = a[2] = a[3] = 0
        return a

    def act(self, time, sensors, reward):
        a = self.actions.get_instance()
        a[0] = 0
        a[1] = 0.1
        a[2] = 1 
        a[3] = 0
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
