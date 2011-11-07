import OpenNero

class Turret(OpenNero.AgentBrain):
    """
    Simple Rotating Turret
    """
    def __init__(self):
        OpenNero.AgentBrain.__init__(self)
        self.team = 1

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
