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
            self.agents.add(agent.NeroAgent.factory(ai, self.team_type))

    def start_ai(self):
        """
        Start Team level AI
        """
        pass

    def save(self, location):
        """
        Saves a team to a file at location
        """
        pass

    def is_episode_over(self, agent):
        return False

    def reset(self, agent):
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
        cls = agent.NeroAgent.factory_class(ai)
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

    def is_episode_over(self, agent):
        return agent.org.eliminate

    def reset(self, agent):
        agent.org = self.ai.reproduce_one()
