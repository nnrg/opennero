from OpenNero import *  # import the OpenNERO C++ API

class TestAgent(AgentBrain):
    """
    A simple agent with keyboard controls to move forward, back, left, and right.
    """
    keys = set([]) # keys pressed
    action_map = {'FWD':0, 'BCK':1, 'LFT':2, 'RHT':3}
    
    def __init__(self):
        """
        This is the constructor - it gets called when the brain object is first created
        """
        # call the parent constructor
        AgentBrain.__init__(self) # do not remove!
        
    def initialize(self, init_info):
        """
        Called when the agent is first created
        """
        self.actions = init_info.actions
        return True
        
    def start(self, time, sensors):
        """
        Take the initial sensors and return the first action
        """
        # Ignore sensors and just return the action.
        return self.key_action()
        
    def act(self, time, sensors, reward):
        """
        Take new sensors and reward from previous action and return the next action
        """
        # Ignore sensors and reward and just return the action.
        return self.key_action()
        
    def end(self, time, reward):
        """
        Take the last reward
        """
        # Ignore the reward here as well.
        return True

    def destroy(self):
        """
        Called when the agent is destroyed
        """
        return True

    def key_action(self):
        actions = self.actions.get_instance()
        assert(len(actions) == 2)
        
        # Keys specify movement action relative to screen coordinates.
        actions[0] = 0
        actions[1] = 0
        if len(TestAgent.keys) > 0:
            for key in TestAgent.keys:
                if key == 'RHT':
                    actions[1] = 1
                elif key == 'LFT':
                    actions[1] = -1
                elif key == 'FWD':
                    actions[0] = -1
                elif key == 'BCK':
                    actions[0] = 1
     
            print "keys: ", ' '.join(TestAgent.keys)
            TestAgent.keys.clear()

        return actions
    


def createInputMapping():
    """
    Create key bindings to control the agent.
    """
    # create an io map
    ioMap = PyIOMap()

    # tell the agent which key is pressed
    def control(key):
        def closure():
            TestAgent.keys.add(key)
        return closure

    # bind arrow keys to control agent.
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_RIGHT", "onPress", control('RHT') )
    ioMap.BindKey( "KEY_LEFT", "onPress", control('LFT') )
    ioMap.BindKey( "KEY_UP", "onPress", control('FWD') )
    ioMap.BindKey( "KEY_DOWN", "onPress", control('BCK') )
    
    return ioMap
