import json

import constants
import OpenNero
import agent as agents

def factory(ai, *args):
    cls = ai_map.get(ai, NeroTeam)
    return cls(*args)

class TeamEncoder(json.JSONEncoder):
    def default(self, obj):
        agentEncoder = agents.AgentEncoder()
        if isinstance(obj, NeroTeam):
            return {
                'team_ai': inv_ai_map.get(type(obj), 'none'),
                'agents': [agentEncoder.default(agent) for agent in obj.agents]
            }
        return json.JSONEncoder.default(self, obj)

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
        a = agents.factory(ai, self.team_type)
        self.agents.add(a)
        return a

    def is_episode_over(self, agent):
        return False

    def reset(self, agent):
        pass

    def destroy(self):
        pass

class RTNEATTeam(NeroTeam):
    def __init__(self, team_type):
        NeroTeam.__init__(self, team_type)
        self.ai = None

    def create_agents(self, ai):
        cls = agents.factory_class(ai)
        self.pop = OpenNero.Population(cls.genome,
                                       constants.pop_size,
                                       1.0)

        self.ai = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                  self.pop,
                                  constants.DEFAULT_LIFETIME_MIN,
                                  constants.DEFAULT_EVOLVE_RATE)
        
        for org in self.pop.organisms:
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


ai_map = {
    'rtneat': RTNEATTeam,
    'none': NeroTeam
}

inv_ai_map = {v: k for k, v in ai_map.items()}
