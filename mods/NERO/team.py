import common
import constants
import environment
import OpenNero
import agent

class NeroTeam(object):
    """
    Basic NERO Team
    """
    def __init__(self, team_type):
        self.team_type = team_type
        self.color = constant.TEAM_LABELS[team_type]
    
    def deploy(self, ai):
        """
        Deploy a team of agents with the appropriate AI
        """
        self.environment.remove_all_agents(team)
        for _ in range(constants.pop_size):
            agent = agent.NeroAgent.factory(ai)
            self.spawnAgent(agent)
        self.start_ai()

    def spawn_agent(self, agent):
        """
        Spawn a single agent with the appropriate AI
        """
        simId = common.addObject(
            "data/shapes/character/steve_%s.xml" % (self.color),
            OpenNero.Vector3f(agent.mod, self.spawn_y + dy, 2),
            type=self.team)
        common.initObjectBrain(simId, agent)
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

    # def load(location):
    #     """
    #     Loads a team from a file at location
    #     """
    #     pass

class RTNEATTeam(NeroTeam):
    pass
