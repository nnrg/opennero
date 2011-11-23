import copy
import math
import random

import common
import constants
import module
import OpenNero


class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self, agent):
        self.id = agent.state.id
        self.agent = agent
        self.pose = (0, 0, 0)  # current x, y, heading
        self.prev_pose = (0, 0, 0)
        self.initial_position = OpenNero.Vector3f(0, 0, 0)
        self.initial_rotation = OpenNero.Vector3f(0, 0, 0)
        self.total_damage = 0
        self.curr_damage = 0

    def __str__(self):
        x, y, h = self.pose
        px, py, ph = self.prev_pose
        return 'agent { id: %d, pose: (%.02f, %.02f, %.02f), prev_pose: (%.02f, %.02f, %.02f) }' % \
            (self.id, x, y, h, px, py, ph)

    def randomize(self):
        dx = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        dy = random.randrange(constants.XDIM / 20) - constants.XDIM / 40
        self.initial_position.x = module.getMod().spawn_x[self.agent.get_team()] + dx
        self.initial_position.y = module.getMod().spawn_y[self.agent.get_team()] + dy
        self.prev_pose = self.pose = (self.initial_position.x,
                                      self.initial_position.y,
                                      self.initial_rotation.z)

    def reset_pose(self, position, rotation):
        self.prev_pose = self.pose = (position.x, position.y, rotation.z)

    def update_damage(self):
        """
        Update the damage for an agent, returning the current damage.
        """
        self.total_damage += self.curr_damage
        damage = self.curr_damage
        self.curr_damage = 0
        return damage

    def update_pose(self, move_by, turn_by):
        dist = constants.MAX_MOVEMENT_SPEED * move_by
        heading = common.wrap_degrees(self.agent.state.rotation.z, turn_by)
        x = self.agent.state.position.x + dist * math.cos(math.radians(heading))
        y = self.agent.state.position.y + dist * math.sin(math.radians(heading))

        self.prev_pose = self.pose
        self.pose = (x, y, heading)

        # try to update position
        pos = copy.copy(self.agent.state.position)
        pos.x = x
        pos.y = y
        self.agent.state.position = pos

        # try to update rotation
        rot = copy.copy(self.agent.state.rotation)
        rot.z = heading
        self.agent.state.rotation = rot


class NeroEnvironment(OpenNero.Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        """
        Create the environment
        """
        OpenNero.Environment.__init__(self)

        self.lifetime = constants.DEFAULT_LIFETIME
        self.hitpoints = constants.DEFAULT_HITPOINTS
        self.epsilon = constants.DEFAULT_EE / 100.0

        self.curr_id = 0
        self.max_steps = 20
        self.MAX_DIST = math.hypot(constants.XDIM, constants.YDIM)

        self.states = {}
        self.teams = dict((t, set()) for t in constants.TEAMS)
        self.agents_to_load = {}

        self.reward_weights = dict((f, 0.) for f in constants.FITNESS_DIMENSIONS)

        self.script = 'NERO/menu.py'

        # we only want to compute the friend center once per tick
        self.friend_center = None
        # we need to know when to update the friend center
        self.friend_center_cache = {}


    def set_weight(self, key, value):
        self.reward_weights[key] = value
        for team in self.teams:
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_weight(constants.FITNESS_INDEX[key], value)

    def remove_all_agents(self, team):
        for agent in list(self.teams[team]):
            common.removeObject(agent.state.id)
            try:
                self.states.pop(agent)
                self.teams[team].discard(agent)
            except:
                print 'could not remove', agent

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        state = self.get_state(agent)
        state.total_damage = 0
        state.curr_damage = 0
        if agent.group == "Agent":
            state.randomize()
            agent.state.position = copy.copy(state.initial_position)
            agent.state.rotation = copy.copy(state.initial_rotation)
            agent.state.update_immediately()
        return True

    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        for a in constants.WALL_RAY_SENSORS:
            agent.add_sensor(OpenNero.RaySensor(
                    math.cos(math.radians(a)), math.sin(math.radians(a)), 0, 50,
                    constants.OBJECT_TYPE_OBSTACLE,
                    False))
        for a0, a1 in constants.FLAG_RADAR_SENSORS:
            agent.add_sensor(OpenNero.RadarSensor(
                    a0, a1, -90, 90, constants.MAX_VISION_RADIUS,
                    constants.OBJECT_TYPE_FLAG,
                    False))
        for a0, a1 in constants.ENEMY_RADAR_SENSORS:
            sense = constants.OBJECT_TYPE_TEAM_0
            if agent.get_team() == sense:
                sense = constants.OBJECT_TYPE_TEAM_1
            agent.add_sensor(OpenNero.RadarSensor(
                    a0, a1, -90, 90, constants.MAX_VISION_RADIUS,
                    sense,
                    False))
        return agent.info

    def get_state(self, agent):
        """
        Returns the state of an agent
        """
        if agent not in self.states:
            self.states[agent] = AgentState(agent)
            self.teams[agent.get_team()].add(agent)
        return self.states[agent]

    def getFriendFoe(self, agent):
        """
        Returns sets of all friend agents and all foe agents.
        """
        my_team = agent.get_team()
        other_team = constants.OBJECT_TYPE_TEAM_1
        if my_team == other_team:
            other_team = constants.OBJECT_TYPE_TEAM_0
        return self.teams[my_team], self.teams[other_team]

    def target(self, agent):
        """
        Returns the nearest foe in a 2-degree cone from an agent.
        """
        friends, foes = self.getFriendFoe(agent)
        if not foes:
            return None
        pose = self.get_state(agent).pose
        min_f = None
        min_v = None
        for f in foes:
            p = self.get_state(f).pose
            fd = self.distance(pose, p)
            fh = abs(self.angle(pose, p))
            if fh <= 2:
                v = fd / math.cos(math.radians(fh * 20))
                if min_v is None or v < min_v:
                    min_f = f
                    min_v = v
        return min_f

    def step(self, agent, action):
        """
        2A step for an agent
        """
        # if this agent has a serialized representation waiting, load it.
        chunk = self.agents_to_load.get(agent.state.id)
        if chunk is not None:
            print 'loading agent', agent.state.id, 'from', len(chunk), 'bytes'
            agent.from_string(chunk)
            del self.agents_to_load[agent.state.id]

        # set the epsilon for this agent, in case it's changed recently.
        agent.epsilon = self.epsilon

        # check if the action is valid
        assert(agent.info.actions.validate(action))

        state = self.get_state(agent)

        #Initilize Agent state
        if agent.step == 0 and agent.group != "Turret":
            p = agent.state.position
            r = agent.state.rotation
            if agent.group == "Agent":
                r.z = random.randrange(360)
                agent.state.rotation = r
            state.reset_pose(p, r)
            return agent.info.reward.get_instance()

        # spawn more agents if possible.
        self.maybe_spawn(agent)

        # get the desired action of the agent
        move_by = action[0]
        turn_by = math.degrees(action[1])

        # set animation speed
        # TODO: move constants into constants.py
        self.set_animation(agent, state, 'run')
        delay = OpenNero.getSimContext().delay
        agent.state.animation_speed = move_by * 100

        reward = self.calculate_reward(agent, action)

        # tell the system to make the calculated motion
        state.update_pose(move_by, turn_by)

        return reward

    def calculate_reward(self, agent, action):
        reward = agent.info.reward.get_instance()

        state = self.get_state(agent)
        friends, foes = self.getFriendFoe(agent)

        R = dict((f, 0) for f in constants.FITNESS_DIMENSIONS)

        R[constants.FITNESS_STAND_GROUND] = -abs(action[0])

        friend = self.nearest(state.pose, friends)
        if friend:
            d = self.distance(self.get_state(friend).pose, state.pose)
            R[constants.FITNESS_STICK_TOGETHER] = -d * d

        foe = self.nearest(state.pose, foes)
        if foe:
            d = self.distance(self.get_state(foe).pose, state.pose)
            R[constants.FITNESS_APPROACH_ENEMY] = -d * d

        f = module.getMod().flag_loc
        if f:
            d = self.distance(state.pose, (f.x, f.y))
            R[constants.FITNESS_APPROACH_FLAG] = -d * d

        target = self.target(agent)
        if target is not None:
            source_pos = agent.state.position
            target_pos = target.state.position
            source_pos.z = source_pos.z + 5
            target_pos.z = target_pos.z + 5
                obstacles = OpenNero.getSimContext().findInRay(
                    source_pos,
                    target_pos,
                    constants.OBJECT_TYPE_OBSTACLE,
                    True)
            if len(obstacles) == 0:
                    self.get_state(target).curr_damage += 1
                    R[constants.FITNESS_HIT_TARGET] = 1

        damage = state.update_damage()
        R[constants.FITNESS_AVOID_FIRE] = -damage

        if len(reward) == 1:
            for i, f in enumerate(constants.FITNESS_DIMENSIONS):
                reward[0] += self.reward_weights[f] * R[f] / constants.FITNESS_SCALE.get(f, 1.0)
                #print f, self.reward_weights[f], R[f] / constants.FITNESS_SCALE.get(f, 1.0)
        else:
            for i, f in enumerate(constants.FITNESS_DIMENSIONS):
                reward[i] = R[f]

        return reward

    def maybe_spawn(self, agent):
        '''Spawn more agents if there are more to spawn.'''
        if agent.ai != 'rtneat' or agent.group != 'Agent':
            return

        team = agent.get_team()
        rtneat = OpenNero.get_ai('rtneat-%s' % team)
        if not rtneat or not rtneat.ready():
            return

        friends, foes = self.getFriendFoe(agent)
        if len(friends) >= constants.pop_size:
            return

        if agent is tuple(f for f in friends if f.ai == agent.ai)[0]:
            module.getMod().spawnAgent(team=team, ai=agent.ai)

    def sense(self, agent, observations):
        """
        figure out what the agent should sense
        """
        # the last observation is whether there is a target for the agent
        observations[constants.SENSOR_INDEX_TARGETING[0]] = int(self.target(agent) is not None)

        friends, foes = self.getFriendFoe(agent)
        if not friends:
            return observations

        # the 2 before that are the angle and heading to the center of mass of
        # the agent's team
        # TODO: this only needs to be calculated once per tick, instead of once per agent
        ax, ay = agent.state.position.x, agent.state.position.y
        cx, cy = self.get_friend_center(agent, friends)
        fd = self.distance((ax, ay), (cx, cy))
        ah = agent.state.rotation.z
        fh = self.angle((ax, ay, ah), (cx, cy)) + 180.0
        if fd <= constants.MAX_FRIEND_DISTANCE:
            observations[constants.SENSOR_INDEX_FRIEND_RADAR[0]] = fd / 15.0
            observations[constants.SENSOR_INDEX_FRIEND_RADAR[1]] = fh / 360.0
        return observations
        
    def get_friend_center(self, agent, friends):
        ''' get the x, y position of the center of mass of the friends group '''
        id = agent.state.id
        step = agent.step
        # only recompute the center of mass once we find an agent that goes 
        # to the next step since the last computation
        if self.friend_center is None or step != self.friend_center_cache.get(id, step):
            n = float(len(friends))
            cx, cy = 0, 0
            for f in friends:
                fx, fy = f.state.position.x, f.state.position.y
                cx += fx / n
                cy += fy / n
            self.friend_center = (cx, cy)
            self.friend_center_cache.clear()
        self.friend_center_cache[id] = step
        return self.friend_center

    def distance(self, a, b):
        """
        Returns the distance between positions (x-y tuples) a and b.
        """
        return math.hypot(a[0] - b[0], a[1] - b[1])

    def angle(self, a, b):
        """
        Returns the relative angle from a looking towards b, in the interval
        [-180, +180]. a needs to be a 3-tuple (x, y, heading) and b needs to be
        an x-y tuple.
        """
        if self.distance(a, b) == 0:
            return 0
        rh = math.degrees(math.atan2(b[1] - a[1], b[0] - a[0])) - a[2]
        if rh < -180:
            rh += 360
        if rh > 180:
            rh -= 360
        return rh

    def nearest(self, loc, agents):
        """
        Returns the nearest agent to a particular location.
        """
        # TODO: this needs to only be computed once per tick, not per agent
        if not agents:
            return None
        nearest = None
        min_dist = self.MAX_DIST * 5
        for agent in agents:
            d = self.distance(loc, self.get_state(agent).pose)
            if 0 < d < min_dist:
                nearest = agent
                min_dist = d
        return nearest

    def set_animation(self, agent, state, animation):
        """
        Sets current animation
        """
        if agent.state.animation != animation:
            agent.state.animation = animation

    def is_episode_over(self, agent):
        """
        is the current episode over for the agent?
        """
        if agent.group == 'Turret' or agent.ai == 'qlearning':
            return False

        team = agent.get_team()
        state = self.get_state(agent)
        dead = self.hitpoints > 0 and state.total_damage >= self.hitpoints
        old = self.lifetime > 0 and agent.step >= self.lifetime
        rtneat = OpenNero.get_ai("rtneat-%s" % team)
        orphaned = rtneat and not rtneat.has_organism(agent)

        if agent.ai == 'rtneat' and (orphaned or dead or old):
                return True

        return False

    def cleanup(self):
        """
        cleanup the world
        """
        common.killScript(self.script)
        return True
