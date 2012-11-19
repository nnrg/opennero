import random
import common
import constants
import module
import OpenNero

class ScriptedAgent(OpenNero.AgentBrain):
    """
    Scripted agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        return True

    def start(self, time, sensors):
        """
        start of an episode
        """
        module.getMod().scripted.flush()
        return self.take_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # Store fitness if a cube was collected, otherwise accumulate reward.
        if reward >= 1:
            self.reward = 0
            self.count = 0
        else:
            self.reward += reward[0]
            self.count += 1
        return self.take_action(sensors) # return action

    def end(self, time, reward):
        """
        end of an episode
        """
        print  "Final reward: %f, fitness: %f" % (reward, self.fitness)
        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True

    def take_action(self, sensors):
        """
        Produce actions from sensor readings
        """
        assert(len(sensors) == module.getMod().num_sensors) # make sure we have the right number of sensors

        # create the sensor array and convert sensor values from network to advice space for
        # script computations.
        inputs = [sensor for sensor in sensors]
        for i in range(len(inputs)):
            nmin = module.getMod().sbounds_network.min(i)
            nmax = module.getMod().sbounds_network.max(i)
            amin = module.getMod().sbounds_advice.min(i)
            amax = module.getMod().sbounds_advice.max(i)
            inputs[i] = (inputs[i]-nmin)*(amax-amin)/(nmax-nmin) + amin;

        outputs = module.getMod().scripted.evaluate(inputs) # evaluate the script to find outputs
        actions = self.actions.get_instance() # make a vector for the actions

        assert(len(actions) == len(outputs))
        for i in range(len(outputs)):
            assert outputs[i] >= -1 and outputs[i] <= 1
            actions[i] = (outputs[i]+1.0)/2.0  # convert value from advice to network space, i.e. [0, 1]
            actions[i] = actions[i] - 0.5

        return actions

class RTNEATAgent(OpenNero.AgentBrain):
    INDEX_COUNT = 0
    BACKPROP_WINDOW_SIZE = 10
    ERROR_THRESHOLD = 2

    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        OpenNero.AgentBrain.__init__(self)
        self.rtneat = OpenNero.get_ai("neat")
        self.org = None
        self.environment = OpenNero.get_environment()
        self.passed_steps = 0

    def initialize(self, init_info):
        """
        Initialize an agent brain with action and sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.sensors = init_info.sensors # constraints for sensors
        return True

    def start(self, time, sensors):
        """
        start of an episode
        """
        # Index is assigned here instead of in initialize() because it is here that
        # we get an rtneat organism for the agent.  The index is reassigned at the
        # start of every episode because the organism changes in every episode.
        self.index = RTNEATAgent.INDEX_COUNT
        RTNEATAgent.INDEX_COUNT = (RTNEATAgent.INDEX_COUNT+1)% module.getMod().pop_size

        self.org = self.rtneat.get_organism(self)
        self.net = self.org.net
        self.net.flush()
        self.reward = 0
        self.reward_steps = 0
        self.reward_steps_total = 0
        self.org.fitness = 0
        #if self.index == 0:  # Color the elite organism
        #    self.sim.color = SColor(0, 200, 28, 56)
        #print "got network to evaluate: " + str(self.net)
        #if self.index == 0:
        #    self.net.print_links()
        return self.network_action(sensors)

    def act(self, time, sensors, reward):
        """
        a state transition
        """
        # Accumulate reward and increment fitness if a cube was visited.
        self.reward += reward[0]
        self.reward_steps += 1
        if reward >= 1:
            self.org.fitness += reward[0]
            self.reward = 0
            self.reward_steps_total += self.reward_steps
            self.reward_steps = 0
        return self.network_action(sensors) # return action

    def end(self, time, reward):
        """
        end of an episode
        """
        # if simulation display has been turned off, complete the episode here
        if self.environment.simdisplay == False:
            state = self.environment.get_state(self)
            while state.current_step < self.environment.STEPS_PER_EPISODE and self.environment.is_active(self):
                sensors = self.environment.compute_sensors(state)
                actions = self.act(time, sensors, reward)
                reward = self.environment.calculate_reward(state) if self.environment.perform_actions(state, actions) else 0.0
                state.current_step += 1

        # accumulate reward and increment fitness if a cube was visited in the last step
        self.reward += reward[0]
        self.reward_steps += 1
        if reward >= 1:
            self.org.fitness += reward[0]
            self.reward_steps_total += self.reward_steps
        
        # increase fitness by adding a fraction for how quickly the cubes were visited
        self.org.fitness += self.org.fitness/self.reward_steps_total if self.reward_steps_total > 0 else 0.0

        # add another fraction to fitness (i.e. average reward since visiting the last cube)
        if reward < 1: self.org.fitness += self.reward/self.reward_steps/100  # note that average reward can be negative

        self.org.time_alive += 1

        # if a trace has been loaded, then process it here
        if self.environment.use_trace:
            trfitness = self.evaluate_trace()  # Calculate trace fitness and passed steps
            if self.environment.run_backprop:  # backprop invalidates self.org.fitness because it changes network
                if self.index != 0:  # run backprop only if the agent is not the elite organism
                    self.backprop_trace()
                    trfitness = self.evaluate_trace()  # Calculate trace fitness again since backprop changes network
                self.org.fitness = trfitness
                print "index: %d, trace fitness: %f" % (self.index, trfitness)
            else:
                self.org.fitness = floor(self.org.fitness if self.org.fitness > 0.0 else 0.0) + trfitness
                print "index: %d, trace fitness: %f, combined fitness: %f" % (self.index, trfitness, self.org.fitness)
        else:
            print "index: %d, fitness: %f" % (self.index, self.org.fitness)

        # if we are "Agent 0", we should save the population into a file with the current episode number
        if self.index == 0:
            self.rtneat.save_population('saved_population_%d.txt' % self.environment.get_state(self).current_episode)

        self.rtneat.release_organism(self)

        return True

    def destroy(self):
        """
        the agent brain is discarded
        """
        return True

    def network_action(self, sensors):
        """
        Take the current network
        Feed the sensors into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid actions
        """
        assert(len(sensors) == module.getMod().num_sensors) # make sure we have the right number of sensors
        inputs = [sensor for sensor in sensors] # create the sensor array
        self.net.load_sensors(inputs)
        self.net.activate()
        outputs = self.net.get_outputs()

        #if self.index == 0:
        #    print "agent 0 inputs: ", inputs
        #    print "agent 0 outputs: ", outputs
        #    #self.net.show_activation()

        actions = self.actions.get_instance() # make a vector for the actions

        assert(len(actions) == len(outputs))
        for i in range(len(outputs)):
            actions[i] = outputs[i] - 0.5

            # Scale values in the range [-0.25, 0.25] to [-0.5, 0.5] so that advice can
            # produce maximum output when output is set to 1 (a variable set to 1 in the
            # advice language produces only ~ 0.768 activation in the network since we
            # restrict values to the region of the sigmoid that is approximately linear).
            actions[i] = -0.5 if actions[i] < -0.25 else 0.5 if actions[i] > 0.25 else (actions[i]+0.25)/0.5 - 0.5;

        return actions

    def evaluate_trace(self):
        """
        evaluate agent and compute fitness based on trace information
        """
        self.net.flush()
        state = self.environment.get_new_state()
        trace = self.environment.trace
        state.position = Vector3f(trace.position[0].x, trace.position[0].y, trace.position[0].z)
        state.rotation = Vector3f(trace.rotation[0].x, trace.rotation[0].y, trace.rotation[0].z)
        state.current_step = trace.initial_step
        j = 0  # trace index at which error is calculated
        while j < len(trace.position)-1 and state.current_step < self.environment.STEPS_PER_EPISODE:
            sensors = self.environment.compute_sensors(state)
            actions = self.network_action(sensors)

            # error based on position - find index in trace where error based on
            # current position starts to increase, i.e. starting with the current
            # trace index, we find position in trace that is closest to the current
            # state position.
            error1 = trace.position[j].getDistanceFrom(state.position)
            error2 = trace.position[j+1].getDistanceFrom(state.position)
            while error1 >= error2 and j < len(trace.position)-2:
                j += 1
                error1 = error2
                error2 = trace.position[j+1].getDistanceFrom(state.position)

            if error1 > self.ERROR_THRESHOLD:
                break

            # calculate new position, orientation, and velocity
            self.environment.perform_actions(state, actions)
            state.current_step += 1

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

class KeyboardAgent(OpenNero.AgentBrain):
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
        OpenNero.AgentBrain.__init__(self) # do not remove!

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
        self.reward += reward[0]
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

