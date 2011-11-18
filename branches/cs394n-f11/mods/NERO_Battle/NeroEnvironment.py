import NERO.NeroEnvironment
import NERO.constants as constants
import OpenNero


class NeroEnvironment(NERO.NeroEnvironment.NeroEnvironment):
    """
    Environment for Nero battle.
    """
    def __init__(self):
        NERO.NeroEnvironment.NeroEnvironment.__init__(self)
        self.script = 'NERO_Battle/menu.py'

    def step(self, agent, action):
        """
        2A step for an agent
        """
        reward = NERO.NeroEnvironment.NeroEnvironment.step(self, agent, action)

        if not reward[constants.FITNESS_INDEX[constants.FITNESS_HIT_TARGET]]:
            return reward

        damages = {}
        for team in constants.TEAMS:
            damages[team] = 0
            agents = self.teams.get(team, set())
            if agents:
                damage = sum(self.get_state(a).total_damage for a in agents)
                damage += (constants.pop_size - len(agents)) * self.hitpoints
                damages[team] = damage
            else:
                foe = constants.OBJECT_TYPE_TEAM_0
                if team == foe:
                    foe = constants.OBJECT_TYPE_TEAM_1
                print constants.TEAM_LABELS[foe], 'team wins!!!'

        if len(damages) == 2:
            print 'damages sustained by:',
            for t, d in sorted(damages.iteritems()):
                print '%s:%d' % (constants.TEAM_LABELS[t], d),
            print

        return reward

    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        state = self.get_state(agent)
        assert self.hitpoints > 0
        if state.total_damage >= self.hitpoints:
            # agent has died, send it to heaven.
            agent.state.position = OpenNero.Vector3f(100000, 100000, 100000)
            state.pose = (100000, 100000, 0)
            self.teams[agent.get_team()].discard(agent)
        return False
