import common
import constants
import environment
import OpenNero
import agent

class NeroTeam(object):
    """
    Basic NERO Team
    """
    @staticmethod
    def factory(ai, *args):
        ai_map = {
            'rtneat': RTNEATTeam
        }
        cls = ai_map.get(ai, NeroTeam)
        return cls(*args)

    def __init__(self, team_type):
        self.team_type = team_type
        self.color = constants.TEAM_LABELS[team_type]
        self.agents = set()
    
    def deploy(self, ai):
        """
        Deploy a team of agents with the appropriate AI
        """
        self.remove_all_agents()
        for _ in range(constants.pop_size):
            self.spawn_agent(agent.NeroAgent.factory(ai, self.team_type))
        self.start_ai()

    def spawn_agent(self, agent):
        """
        Spawn a single agent with the appropriate AI
        """
        agent.randomize()
        simId = common.addObject(
            "data/shapes/character/steve_%s.xml" % (self.color),
            OpenNero.Vector3f(agent.mod_state.initial_position.x, agent.mod_state.initial_position.y, 2),
            type=self.team_type)
        common.initObjectBrain(simId, agent)
        self.agents.add(agent)
        return simId

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

    def remove_all_agents(self):
        for agent in list(self.agents):
            self.remove_agent(agent)

    def remove_agent(self, agent):
        common.removeObject(agent.state.id)
        try:
            self.agents.discard(agent)
        except:
            pass

    # def load(location):
    #     """
    #     Loads a team from a file at location
    #     """
    #     pass

class RTNEATTeam(NeroTeam):
    pass
