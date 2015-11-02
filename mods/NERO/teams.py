import json

import constants
import OpenNero
import agent as agents

def factory(ai, *args):
    cls = ai_map.get(ai, NeroTeam)
    return cls(*args)

class TeamEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, NeroTeam):
            return {
                'team_ai': inv_ai_map.get(obj.__class__, 'none'),
                'agents': [
                    {
                        'agent_ai': agent.ai_label(),
                        'args': agent.args()
                    } 
                    for agent in obj.agents
                ]
            }
        return json.JSONEncoder.default(self, obj)

def as_team(team_type, dct):
    if 'team_ai' in dct:
        team = factory(dct['team_ai'], team_type)
        for a in dct['agents']:
            team.create_agent(a['agent_ai'], *a['args'])
        return team
    return dct

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
            self.create_agent(ai)

    def create_agent(self, ai, *args):
        a = agents.factory(ai, self.team_type, *args)
        self.add_agent(a)
        return a

    def add_agent(self, a):
        self.agents.add(a)

    def is_episode_over(self, agent):
        return False

    def reset(self, agent):
        pass

    def destroy(self):
        pass

class RTNEATTeam(NeroTeam):
    def __init__(self, team_type):
        NeroTeam.__init__(self, team_type)
        self.pop = OpenNero.Population()
        self.ai = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                  self.pop,
                                  constants.DEFAULT_LIFETIME_MIN,
                                  constants.DEFAULT_EVOLVE_RATE)

    def add_agent(self, a):
        NeroTeam.add_agent(self, a)
        self.pop.add_organism(a.org)

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
