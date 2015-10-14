import common
import constants
import environment
import OpenNero
import agent

def factory(ai, *args):
    ai_map = {
        'rtneat': RTNEATTeam
    }
    cls = ai_map.get(ai, NeroTeam)
    return cls(*args)

class NeroTeam(object):
    """
    Basic NERO Team
    """

    def __init__(self, team_type):
        self.team_type = team_type
        self.color = constants.TEAM_LABELS[team_type]
        self.agents = set()
    
    def create_agents(self, ai):
        for _ in range(constants.pop_size):
            self.self.create_agent(ai)

    def create_agent(self, ai):
        a = agent.factory(ai, self.team_type)
        self.agents.add(a)
        return a

    def save(self, location):
        """
        Saves a team to a file at location
        """
        pass

    def is_episode_over(self, agent):
        return False

    def reset(self, agent):
        pass

    def destroy(self):
        pass

    # def load(location):
    #     """
    #     Loads a team from a file at location
    #     """
    #     pass

class RTNEATTeam(NeroTeam):
    def __init__(self, team_type):
        NeroTeam.__init__(self, team_type)
        self.ai = None

    def create_agents(self, ai):
        cls = agent.factory_class(ai)
        self.ai = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                  cls.num_inputs,
                                  cls.num_outputs,
                                  constants.pop_size,
                                  1.0,
                                  constants.DEFAULT_EVOLVE_RATE)
        self.ai.set_lifetime(constants.DEFAULT_LIFETIME_MIN)
        for org in self.ai.organisms:
            self.agents.add(cls(self.team_type, org))
        self.start_ai()

    def start_ai(self):
        OpenNero.set_ai('rtneat-%s' % self.team_type, self.ai)

    def stop_ai(self):
        OpenNero.set_ai('rtneat-%s' % self.team_type, None)

    def is_episode_over(self, agent):
        return agent.org.eliminate

    def reset(self, agent):
        agent.org = self.ai.reproduce_one()

    def destroy(self):
        self.stop_ai()


