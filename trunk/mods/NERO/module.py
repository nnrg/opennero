import os
import re
import sys
import gzip
import random
import tempfile

import common
import constants
import NeroEnvironment
import OpenNero


def rtneat_rewards():
    """
    Create a reward FeatureVectorInfo to pass to RTNEAT.
    """
    reward = OpenNero.FeatureVectorInfo()
    for f in constants.FITNESS_DIMENSIONS:
        reward.add_continuous(-sys.float_info.max, sys.float_info.max)
    return reward


class NeroModule:
    def __init__(self):
        self.environment = None
        self.agent_id = None

        self.flag_loc = None
        self.flag_id = None

        self.set_speedup(constants.DEFAULT_SPEEDUP)

        x = constants.XDIM / 2.0
        y = constants.YDIM / 3.0
        self.spawn_x = {}
        self.spawn_y = {}
        self.set_spawn(x, y, constants.OBJECT_TYPE_TEAM_0)
        self.set_spawn(x, 2 * y, constants.OBJECT_TYPE_TEAM_1)

    def setup_map(self):
        """
        setup the test environment
        """
        OpenNero.disable_ai()

        if self.environment:
            error("Environment already created")
            return

        # create the environment - this also creates the rtNEAT object
        self.environment = self.create_environment()
        OpenNero.set_environment(self.environment)

        # world walls
        height = constants.HEIGHT + constants.OFFSET
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, 0, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(constants.WIDTH, constants.XDIM, constants.HEIGHT*2),
            label="World Wall0",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(0, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 0),
            scale=OpenNero.Vector3f(constants.WIDTH, constants.YDIM, constants.HEIGHT*2),
            label="World Wall1",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 0),
            scale=OpenNero.Vector3f(constants.WIDTH, constants.YDIM, constants.HEIGHT*2),
            label="World Wall2",
            type=constants.OBJECT_TYPE_OBSTACLE)
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(constants.WIDTH, constants.XDIM, constants.HEIGHT*2),
            label="World Wall3",
            type=constants.OBJECT_TYPE_OBSTACLE)

        # Add an obstacle wall in the middle
        common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM/2, height),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(constants.WIDTH, constants.YDIM / 4, constants.HEIGHT*2),
            label="World Wall4",
            type=constants.OBJECT_TYPE_OBSTACLE)

        # Add some trees
        for i in (0.25, 0.75):
            for j in (0.25, 0.75):
                # don't collide with trees - they are over 500 triangles each
                common.addObject(
                    "data/shapes/tree/Tree.xml",
                    OpenNero.Vector3f(i * constants.XDIM, j * constants.YDIM, constants.OFFSET),
                    OpenNero.Vector3f(0, 0, 0),
                    scale=OpenNero.Vector3f(1, 1, 1),
                    label="Tree %d %d" % (10 * i, 10 * j),
                    type=constants.OBJECT_TYPE_LEVEL_GEOM)
                # collide with their trunks represented with cubes instead
                common.addObject(
                    "data/shapes/cube/Cube.xml",
                    OpenNero.Vector3f(i * constants.XDIM, j * constants.YDIM, constants.OFFSET),
                    OpenNero.Vector3f(0,0,0),
                    scale=OpenNero.Vector3f(1,1,constants.HEIGHT),
                    type=constants.OBJECT_TYPE_OBSTACLE)

        # Add the surrounding Environment
        common.addObject(
            "data/terrain/NeroWorld.xml",
            OpenNero.Vector3f(constants.XDIM/2, constants.YDIM/2, 0),
            scale=OpenNero.Vector3f(1.2, 1.2, 1.2),
            label="NeroWorld",
            type=constants.OBJECT_TYPE_LEVEL_GEOM)

        return True

    def create_environment(self):
        return NeroEnvironment.NeroEnvironment()

    def change_flag(self, loc):
        if self.flag_id:
            common.removeObject(self.flag_id)
        self.flag_loc = OpenNero.Vector3f(*loc)
        self.flag_id = common.addObject(
            "data/shapes/cube/BlueCube.xml",
            self.flag_loc,
            label="Flag",
            scale=OpenNero.Vector3f(1, 1, 10),
            type=constants.OBJECT_TYPE_FLAG)

    def place_basic_turret(self, loc):
        return common.addObject(
            "data/shapes/character/steve_basic_turret.xml",
            OpenNero.Vector3f(*loc),
            type=constants.OBJECT_TYPE_TEAM_1)

    #The following is run when one of the Deploy buttons is pressed
    def deploy(self, ai='rtneat', team=constants.OBJECT_TYPE_TEAM_0):
        OpenNero.disable_ai()
        if ai == 'rtneat':
            OpenNero.set_ai('rtneat-%s' % team, None)
        self.environment.remove_all_agents(team)
        for _ in range(constants.pop_size):
            self.spawnAgent(ai=ai, team=team)
        OpenNero.enable_ai()

    #The following is run when the Save button is pressed
    def save_team(self, location, team=constants.OBJECT_TYPE_TEAM_0):
        location = os.path.relpath("/") + location
        # if there are rtneat agents in the environment, save them as a group.
        rtneat = OpenNero.get_ai("rtneat-%s" % team)
        if rtneat:
            rtneat.save_population(str(location))
        # then, check whether there are any qlearning agents, and save them.
        with open(str(location), 'a') as handle:
            for agent in self.environment.teams[team]:
                if agent.group == 'Agent' and agent.ai == 'qlearning':
                    handle.write('\n\n%s' % agent.to_string())

    #The following is run when the Load button is pressed
    def load_team(self, location, team=constants.OBJECT_TYPE_TEAM_0):
        OpenNero.disable_ai()

        self.environment.remove_all_agents(team)

        location = os.path.relpath("/") + location
        if not os.path.exists(location):
            print location, 'does not exist, cannot load population'
            return

        # parse out different agents from the population file.
        contents = ''
        try:
            try:
                handle = gzip.open(location)
                contents = handle.read()
            finally:
                handle.close()
        except Exception, e:
            with open(location) as handle:
                contents = handle.read()

        if not contents:
            print 'cannot read', location, 'skipping'
            return

        rtneat, qlearning = self._split_population(contents.splitlines(True))

        print 'qlearning agents:', qlearning.count('Approximator')

        # load any qlearning agents first, subtracting them from the population
        # size that rtneat will need to manage. since we cannot deserialize an
        # agent's state until after it's been added to the world, we put the
        # serialized chunk for the agent into a map, then NeroEnvironment#step
        # takes care of the deserialization.
        pop_size = constants.pop_size
        if qlearning.strip():
            for chunk in re.split(r'\n\n+', qlearning):
                if not chunk.strip():
                    continue
                id = self.spawnAgent(ai='qlearning', team=team)
                self.environment.agents_to_load[id] = chunk
                pop_size -= 1
                if pop_size == 0:
                    break

        print 'rtneat agents:', rtneat.count('genomeend')

        # load any rtneat agents from the file, as a group.
        if pop_size > 0 and rtneat.strip():
            tf = tempfile.NamedTemporaryFile(delete=False)
            tf.write(rtneat)
            tf.close()
            OpenNero.set_ai("rtneat-%s" % team, OpenNero.RTNEAT(
                    tf.name, "data/ai/neat-params.dat",
                    pop_size,
                    rtneat_rewards()))
            os.unlink(tf.name)
            while pop_size > 0:
                self.spawnAgent(ai='rtneat', team=team)
                pop_size -= 1

        OpenNero.enable_ai()

    def _split_population(self, lines):
        rtneat = []
        qlearning = []
        state = 'IDLE'
        for i, line in enumerate(lines):
            if state == 'IDLE':
                if line.startswith('genomestart'):
                    state = 'RTNEAT'
                    rtneat.append(line)
                if 'OpenNero' in line and 'Approximator' in line:
                    state = 'QLEARNING'
                    qlearning.append(line)
            elif state == 'RTNEAT':
                rtneat.append(line)
                if line.startswith('genomeend'):
                    state = 'IDLE'
            elif state == 'QLEARNING':
                if line.startswith('genomestart'):
                    qlearning.append('\n\n')
                    rtneat.append(line)
                    state = 'RTNEAT'
                elif line.strip():
                    qlearning.append(line)
                else:
                    qlearning.append('\n\n')
                    state = 'IDLE'
            else:
                assert False, 'error on line %d' % i
        return ''.join(rtneat), ''.join(qlearning)

    def set_speedup(self, speedup):
        OpenNero.getSimContext().delay = 1.0 - (speedup / 100.0)
        print 'speedup delay', OpenNero.getSimContext().delay
        if self.environment:
            self.environment.speedup = speedup / 100.0

    def set_spawn(self, x, y, team=constants.OBJECT_TYPE_TEAM_0):
        self.spawn_x[team] = x
        self.spawn_y[team] = y

    #The following functions are used to let the client update the fitness function
    def set_weight(self, key, value):
        # value in [-1, 1]
        self.environment.set_weight(key, (value - 100) / 100.0)

    def ltChange(self, value):
        self.environment.lifetime = value
        for team in constants.TEAMS:
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_lifetime(value)

    def eeChange(self, value):
        print 'Exploit/Explore:', value / 100.0
        self.environment.epsilon = value / 100.0

    def hpChange(self, value):
        print 'Hit points:', value
        self.environment.hitpoints = value

    def spawnAgent(self, team=constants.OBJECT_TYPE_TEAM_0, ai=None):
        """
        This is the function ran when an agent already in the field
        causes the generation of a new agent.

        Returns the id of the spawned agent.
        """
        if not self.environment:
            return

        if ai == 'rtneat' and not OpenNero.get_ai('rtneat-%s' % team):
            self.start_rtneat(team)

        self.curr_team = team
        color = constants.TEAM_LABELS[team]

        dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        return common.addObject(
            "data/shapes/character/steve_%s_%s.xml" % (color, ai),
            OpenNero.Vector3f(self.spawn_x[team] + dx, self.spawn_y[team] + dy, 2),
            type=team)

    def start_rtneat(self, team=constants.OBJECT_TYPE_TEAM_0):
        # initialize the rtNEAT algorithm parameters
        # input layer has enough nodes for all the observations plus a bias
        # output layer has enough values for all the actions
        # population size matches ours
        # 1.0 is the weight initialization noise
        rtneat = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                 constants.N_SENSORS+1,
                                 constants.N_ACTIONS,
                                 constants.pop_size,
                                 1.0,
                                 rtneat_rewards())

        key = "rtneat-%s" % team
        OpenNero.set_ai(key, rtneat)
        print "get_ai(%s): %s" % (key, OpenNero.get_ai(key))

        rtneat.set_lifetime(self.environment.lifetime)


gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod

def parseInput(strn):
    if strn == "deploy" or len(strn) < 2:
        return
    mod = getMod()
    loc, val = strn.split(' ')
    vali = 1
    if strn.isupper():
        vali = int(val)
    if loc == "SG": mod.set_weight(constants.FITNESS_STAND_GROUND, vali)
    if loc == "ST": mod.set_weight(constants.FITNESS_STICK_TOGETHER, vali)
    if loc == "AE": mod.set_weight(constants.FITNESS_APPROACH_ENEMY, vali)
    if loc == "AF": mod.set_weight(constants.FITNESS_APPROACH_FLAG, vali)
    if loc == "HT": mod.set_weight(constants.FITNESS_HIT_TARGET, vali)
    if loc == "VF": mod.set_weight(constants.FITNESS_AVOID_FIRE, vali)
    if loc == "LT": mod.ltChange(vali)
    if loc == "EE": mod.eeChange(vali)
    if loc == "HP": mod.hpChange(vali)
    if loc == "SP": mod.set_speedup(vali)
    if loc == "save1": mod.save_team(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "load1": mod.load_team(val, constants.OBJECT_TYPE_TEAM_0)
    if loc == "rtneat": mod.deploy('rtneat')
    if loc == "qlearning": mod.deploy('qlearning')

def ServerMain():
    print "Starting mod NERO"
