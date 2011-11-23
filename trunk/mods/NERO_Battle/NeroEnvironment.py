import sys

import module
import NERO.NeroEnvironment
import NERO.constants as constants
import OpenNero

TEAMS = [constants.OBJECT_TYPE_TEAM_0, constants.OBJECT_TYPE_TEAM_1]


class NeroEnvironment(NERO.NeroEnvironment.NeroEnvironment):
    """
    Environment for Nero battle.
    """
    def __init__(self):
        NERO.NeroEnvironment.NeroEnvironment.__init__(self)

        self.done_spawning = {}
        for team in TEAMS:
            self.done_spawning[team] = False
            rtneat = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                     constants.N_SENSORS,
                                     constants.N_ACTIONS,
                                     constants.pop_size,
                                     1.0,
                                     self.agent_info.reward)
            rtneat.set_lifetime(sys.maxint)
            rtneat.disable_evolution()
            OpenNero.set_ai("rtneat-%s" % team, rtneat)

        self.script = 'NERO_Battle/menu.py'

    def step(self, agent, action):
        """
        2A step for an agent
        """
        reward = NERO.NeroEnvironment.NeroEnvironment.step(self, agent, action)

        if not reward[constants.FITNESS_INDEX[constants.FITNESS_HIT_TARGET]]:
            return reward

        labels = {constants.OBJECT_TYPE_TEAM_0: 'blue',
                  constants.OBJECT_TYPE_TEAM_1: 'red'}

        damages = {}
        for team in TEAMS:
            damages[team] = 0
            agents = self.teams.get(team, set())
            if agents:
                damage = sum(self.get_state(a).total_damage for a in agents)
                damage += (constants.pop_size - len(agents)) * module.getMod().hp
                damages[team] = damage
            else:
                foe = constants.OBJECT_TYPE_TEAM_0
                if team == foe:
                    foe = constants.OBJECT_TYPE_TEAM_1
                print labels[foe], 'team wins!!!'
                OpenNero.disable_ai()

        if len(damages) == 2:
            print 'damages: %s' % ', '.join(
                '%s %s' % (labels[t], d) for t, d in damages.iteritems())

        return reward

    def maybe_spawn(self, agent):
        '''Spawn more agents if there are more to spawn.'''
        team = agent.get_team()
        friends, foes = self.getFriendFoe(agent)
        if friends and len(friends) == constants.pop_size:
            self.done_spawning[team] == True
        if not self.done_spawning[team]:
            NERO.NeroEnvironment.NeroEnvironment.maybe_spawn(self, agent)

    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        state = self.get_state(agent)
        hp = module.getMod().hp
        if agent.group != "Turret" and hp != 0 and state.total_damage >= hp:
            # agent has died, send it to heaven.
            agent.state.position = OpenNero.Vector3f(10000, 10000, 10000)
            state.pose = (10000, 10000, 0)
            self.teams[agent.get_team()].discard(agent)
        return False
