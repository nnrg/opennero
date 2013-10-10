import NERO.NeroEnvironment
import NERO.constants as constants
import OpenNero
import common


class NeroEnvironment(NERO.NeroEnvironment.NeroEnvironment):
    """
    Environment for Nero battle.
    """
    def __init__(self):
        NERO.NeroEnvironment.NeroEnvironment.__init__(self)
        self.script = 'NERO_Battle/menu.py'
        self.print_damage = -1
        self.last_damage = []

    def step(self, agent, action):
        """
        2A step for an agent
        """
        # make sure RL agents cannot learn in battle mode.
        agent.alpha = 0.0

        reward = NERO.NeroEnvironment.NeroEnvironment.step(self, agent, action)

        live_agents = sum(len(t) for t in self.teams.itervalues())

        # wait until all agents have spawned before we keep track of damage.
        if self.print_damage < 0 and live_agents < 2 * constants.pop_size:
            return reward

        # print out damages every loop through the entire population.
        self.print_damage += 1
        if self.print_damage >= live_agents:
            self.print_damage = 0
        else:
            return reward

        damages = {}
        winner = None
        for team in constants.TEAMS:
            damages[team] = 0
            agents = self.teams.get(team, set())
            agent_damages = [self.get_state(a).total_damage for a in agents]
            if any(d < self.hitpoints for d in agent_damages):
                damage = sum(agent_damages)
                damage += (constants.pop_size - len(agents)) * self.hitpoints
                damages[team] = damage
            else:
                damages[team] = constants.pop_size * self.hitpoints
                winner = constants.OBJECT_TYPE_TEAM_0
                if team == winner:
                    winner = constants.OBJECT_TYPE_TEAM_1

        if len(damages) == 2:
            ss = []
            for t, d in sorted(damages.iteritems()):
                s = '%s: %d' % (constants.TEAM_LABELS[t], d)
                ss.append(s)
            ss = tuple(ss)
            if self.last_damage != ss:
                print 'damages sustained by: ' +' '.join(ss)
                self.last_damage = ss
                if not constants.getDisplayHint():
                    OpenNero.setWindowCaption('Damage sustained: ' + ' '.join(ss))

        if winner is not None:
            s = constants.TEAM_LABELS[winner] + ' team wins!!!'
            print s
            try:
                if OpenNero.getAppConfig().rendertype != 'null':
                    import tkMessageBox
                    tkMessageBox.showinfo('NERO Battle Results', s)
            except:
                pass
            if not constants.getDisplayHint():
                OpenNero.setWindowCaption(s)
            OpenNero.disable_ai()
            for a in self.teams[winner]:
                self.set_animation(a, self.get_state(a), 'jump')

        return reward

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        # NOOP
        return True
    
    def cleanup(self):
        """
        cleanup the world
        """
        common.killScript("NERO_Battle/menu.py")
        return True

    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        assert self.hitpoints > 0
        if self.get_state(agent).total_damage >= self.hitpoints:
            self.remove_agent(agent)
            return True
        return False
