import constants
import module
import OpenNero
import constants
import Queue

class RTNEATAgent(OpenNero.AgentBrain):
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
        self.team = module.getMod().curr_team

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

    def destroy(self):
        """
        the agent brain is discarded
        """
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

class Turret(OpenNero.AgentBrain):
    """
    Simple Rotating Turret
    """
    def __init__(self):
        OpenNero.AgentBrain.__init__(self)
        self.team = constants.OBJECT_TYPE_TEAM_1

    def initialize(self, init_info):
        self.actions = init_info.actions
        self.sensors = init_info.sensors
        self.group = "Turret"
        self.previous_fire =  0
        return True

    def start(self, time, sensors):
        self.org = None
        self.net = None
        self.state.label = "Turret"
        self.group = "Turret"
        a = self.actions.get_instance()
        for x in range(len(self.actions)):
            a[x] = 0
            if x == 1:
              a[x] = 0
        return a

    def act(self,time,sensors,reward):
        a = self.actions.get_instance()
        for x in range(len(self.actions)):
            a[x] = 0
            if x == 1:
              a[x] = .15

        return a

    def get_team(self):
        return self.team

    def end(self,time,reward):
        return True

    def destroy(self):
        return True

class FirstPersonAgent(OpenNero.AgentBrain):
    """
    A human-controlled agent with a Firset Person View
    """
    key_buffer = Queue.Queue(5) # key buffer
    @classmethod
    def control_fps(cls, action):
        try:
            cls.key_buffer.put_nowait(action)
        except Queue.Full:
            try:
                cls.key_buffer.get_nowait()
                cls.key_buffer.put_nowait(action)
            except Queue.Empty, Queue.Full:
                pass # should rarely happen
    def __init__(self):
        OpenNero.AgentBrain.__init__(self) # do not remove!
        self.group = "FirstPersonAgent"
    def initialize(self, init_info):
        self.action_info = init_info.actions
        return True
    def get_team(self):
        # we are not on either team 1 or 2! we are just watching.
        return 0
    def key_action(self):
        action = self.action_info.get_instance() # create a zero action
        try:
            key = FirstPersonAgent.key_buffer.get_nowait()
            if key is not None and key in constants.FIRST_PERSON_ACTIONS:
                (movement, turn) = constants.FIRST_PERSON_ACTIONS[key]
                action[0] = movement
                action[1] = turn
        except Queue.Empty:
            pass # no keys were pressed
        return action
    def start(self, time, observations):
        return self.key_action()
    def act(self, time, observations, reward):
        return self.key_action()
    def end(self, time, reward):
        return True
