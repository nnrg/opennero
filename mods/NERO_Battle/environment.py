from NERO.environment import NeroEnvironment
import NERO.constants as constants
import OpenNero
import common


class BattleEnvironment(NeroEnvironment):
    """
    Environment for Nero battle.
    """
    def __init__(self):
        NeroEnvironment.__init__(self)
        self.script = 'NERO_Battle/menu.py'
        self.print_damage = -1
        self.last_damage = []

    def tick(self, dt):
        if self.is_battle_over():
            self.end_battle()
            self.reset_all()

    def is_battle_over(self):
        return any([team.is_destroyed() for team in self.teams.values()])

    def end_battle(self):
        print "Battled Ended"
        blue_damage = self.tally_damage(constants.OBJECT_TYPE_TEAM_0)
        red_damage = self.tally_damage(constants.OBJECT_TYPE_TEAM_1)
        if blue_damage < red_damage:
            self.declare_winner(constants.OBJECT_TYPE_TEAM_0)
        elif blue_damage > red_damage:
            self.declare_winner(constants.OBJECT_TYPE_TEAM_1)
        else:
            print "Battle Tied"

    def declare_winner(self, team_type):
        print "%s team wins!" % constants.TEAM_LABELS[team_type]

    def tally_damage(self, team_type):
        team = self.teams[team_type]
        damages = sum([self.get_state(a).total_damage for a in team.agents | team.dead_agents])
        print "%s damages: %d" % (constants.TEAM_LABELS[team_type], damages)
        return damages

    def reset_all(self):
        for team in self.teams.values():
            self.remove_team(team.team_type)
            team.reset_all()
            for a in team.agents:
                state = self.get_state(a)
                state.total_damage = 0
                state.curr_damage = 0
            self.spawn_team(team)

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        team = self.get_team(agent)
        team.kill_agent(agent)
        self.despawn_agent(agent)
        return True
    
    def cleanup(self):
        """
        cleanup the world
        """
        common.killScript("NERO_Battle/menu.py")
        return True
