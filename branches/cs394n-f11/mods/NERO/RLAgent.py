from OpenNero import QLearningBrain


class QLearningAgent(QLearningBrain):
    def __init__(self):
        QLearningBrain.__init__(self, 0.8, 0.8, 0.1)
        self.team = 0

    def get_team(self):
        return self.team
