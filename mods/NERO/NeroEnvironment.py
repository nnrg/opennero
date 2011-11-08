import copy
import math
import random
import sys

import common
import OpenNero
import module
import constants

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self, agent):
        self.id = agent.state.id
        self.agent = agent
        # current x, y, heading pose
        self.pose = (0, 0, 0)
        # previous x, y, heading pose
        self.prev_pose = (0, 0, 0)
        # starting position
        self.initial_position = OpenNero.Vector3f(0, 0, 0)
        # starting orientation
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
        self.initial_position.x = module.getMod().spawn_x + dx
        self.initial_position.y = module.getMod().spawn_y + dy
        self.prev_pose = self.pose = (self.initial_position.x,
                                      self.initial_position.y,
                                      self.initial_rotation.z)

    def reset_pose(self, position, rotation):
        self.initial_position = position
        self.initial_rotation = rotation
        self.prev_pose = self.pose = (position.x, position.y, rotation.z)

    def update_damage(self):
        """
        Update the damage for an agent, returning the current damage.
        """
        self.total_damage += self.curr_damage
        damage = self.curr_damage
        self.curr_damage = 0
        return damage


class NeroEnvironment(OpenNero.Environment):
    """
    Environment for the Nero
    """
    def __init__(self):
        """
        Create the environment
        """
        OpenNero.Environment.__init__(self)

        self.curr_id = 0
        self.max_steps = 20
        self.MAX_DIST = math.hypot(constants.XDIM, constants.YDIM)
        self.states = {}
        self.teams = {}

        abound = OpenNero.FeatureVectorInfo() # actions
        sbound = OpenNero.FeatureVectorInfo() # sensors
        rbound = OpenNero.FeatureVectorInfo() # rewards

        # actions
        abound.add_continuous(-1, 1) # forward/backward speed
        abound.add_continuous(-0.2, 0.2) # left/right turn (in radians)

        # sensor dimensions
        for a in range(constants.N_SENSORS):
            sbound.add_continuous(0, 1);

        # Rewards
        # the enviroment returns the raw multiple dimensions of the fitness as
        # they get each step. This then gets combined into, e.g. Z-score, by
        # the ScoreHelper in order to calculate the final rtNEAT-fitness
        for f in constants.FITNESS_DIMENSIONS:
            # we don't care about the bounds of the individual dimensions
            rbound.add_continuous(-sys.float_info.max, sys.float_info.max) # range for reward

        # initialize the rtNEAT algorithm parameters
        # input layer has enough nodes for all the observations plus a bias
        # output layer has enough values for all the actions
        # population size matches ours
        # 1.0 is the weight initialization noise
        rtneat = OpenNero.RTNEAT("data/ai/neat-params.dat",
                                 constants.N_SENSORS,
                                 constants.N_ACTIONS,
                                 constants.pop_size,
                                 1.0,
                                 rbound)

        OpenNero.set_ai("rtneat", rtneat)
        print "get_ai(rtneat):", OpenNero.get_ai("rtneat")

        # set the initial lifetime
        lifetime = module.getMod().lt
        rtneat.set_lifetime(lifetime)
        print 'rtNEAT lifetime:', lifetime

        self.agent_info = OpenNero.AgentInitInfo(sbound, abound, rbound)

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
            if agent.get_team() == 0:
                sense = constants.OBJECT_TYPE_TEAM_1
            agent.add_sensor(OpenNero.RadarSensor(
                    a0, a1, -90, 90, constants.MAX_VISION_RADIUS,
                    sense,
                    False))
        return self.agent_info

    def get_state(self, agent):
        """
        Returns the state of an agent
        """
        if agent not in self.states:
            self.states[agent] = AgentState(agent)
            if agent.get_team() not in self.teams:
                self.teams[agent.get_team()] = {}
            self.teams[agent.get_team()][agent] = self.states[agent]
        return self.states[agent]

    def getFriendFoe(self, agent):
        """
        Returns lists of all friend agents and all foe agents.
        """
        friend = []
        if agent.get_team() in self.teams:
            friend = self.teams[agent.get_team()]
        foe = []
        if 1 - agent.get_team() in self.teams:
            foe = self.teams[1 - agent.get_team()]
        return friend, foe

    def target(self, agent):
        #Get list of all targets
        friends, foes = self.getFriendFoe(agent)
        if not friends:
            return None
        state = self.get_state(agent)
        min_a = None
        min_v = None
        for f in foes:
            p = foes[f].pose
            fd = self.distance(state.pose, p)
            fh = abs(self.angle(state.pose, p))
            if fh <= 2:
                v = fd / math.cos(math.radians(fh * 20))
                if min_v is None or v < min_v:
                    min_a = a
                    min_v = v
        return min_a

    def step(self, agent, action):
        """
        2A step for an agent
        """
        # check if the action is valid
        assert(self.agent_info.actions.validate(action))

        state = self.get_state(agent)

        # get the reward (which has multiple components)
        reward = self.agent_info.reward.get_instance()

        #Initilize Agent state
        if agent.step == 0 and agent.group != "Turret":
            p = copy.copy(agent.state.position)
            r = copy.copy(agent.state.rotation)
            if agent.group == "Agent":
                r.z = random.randrange(360)
                agent.state.rotation = r
            state.reset_pose(p, r)
            return reward

        # Spawn more agents if there are more to spawn
        if OpenNero.get_ai("rtneat").ready():
            if module.getMod().getNumToAdd() > 0:
                module.getMod().addAgent()

        # the position and the rotation of the agent on-screen
        position = copy.copy(agent.state.position)
        rotation = copy.copy(agent.state.rotation)

        # get the current pose of the agent
        x, y, heading = position.x, position.y, rotation.z
        state.pose = (x, y, heading)

        # get the desired action of the agent
        move_by = action[0]
        turn_by = math.degrees(action[1])

        # set animation speed
        # TODO: move constants into constants.py
        self.set_animation(agent, state, 'run')
        delay = OpenNero.getSimContext().delay
        if delay > 0.0: # if there is a need to show animation
            agent.state.animation_speed = move_by * 28.0 / delay

        # figure out the new heading
        new_heading = common.wrap_degrees(heading, turn_by)

        # figure out the new x, y location
        new_x = x + constants.MAX_MOVEMENT_SPEED * math.cos(math.radians(new_heading)) * move_by
        new_y = y + constants.MAX_MOVEMENT_SPEED * math.sin(math.radians(new_heading)) * move_by

        # calculate if we hit anyone
        hit = 0
        data = self.target(agent)
        if data != None:#len(data) > 0:
            objects = OpenNero.getSimContext().findInRay(
                    position,
                    data.state.position,
                    constants.OBJECT_TYPE_OBSTACLE | constants.OBJECT_TYPE_TEAM_0 | constants.OBJECT_TYPE_TEAM_1,
                    True)
            if len(objects) > 0: sim = objects[0]
            else: sim = data
            if len(objects) == 0 or objects[0] == sim:
                target = self.get_state(data)
                if target != -1:
                    target.curr_damage += 1
                    hit = 1

        # calculate friend/foe
        friends, foes = self.getFriendFoe(agent)
        if not friends:
            return reward #Corner Case

        #calculate fitness accrued during this step
        R = dict([(f, 0) for f in constants.FITNESS_DIMENSIONS])

        R[constants.FITNESS_STAND_GROUND] = -action[0]

        friend = self.nearest(state.pose, state.id, friends)
        if friend:
            d = self.distance(self.get_state(friend).pose, state.pose)
            R[constants.FITNESS_STICK_TOGETHER] = -d*d

        foe = self.nearest(state.pose, state.id, foes)
        if foe:
            d = self.distance(self.get_state(foe).pose, state.pose)
            R[constants.FITNESS_APPROACH_ENEMY] = -d*d

        f = module.getMod().flag_loc
        d = self.distance(state.pose, (f.x, f.y))
        R[constants.FITNESS_APPROACH_FLAG] = -d*d

        R[constants.FITNESS_HIT_TARGET] = hit

        # Update Damage totals
        damage = state.update_damage()
        R[constants.FITNESS_AVOID_FIRE] = -damage

        # put the fitness dimensions into the reward vector in order
        for i, f in enumerate(constants.FITNESS_DIMENSIONS):
            reward[i] = R[f]

        # tell the system to make the calculated motion
        state.prev_pose = state.pose
        state.pose = (new_x, new_y, new_heading)

        # try to update position
        pos = copy.copy(agent.state.position)
        pos.x = new_x
        pos.y = new_y
        agent.state.position = pos

        # try to update rotation
        rot = copy.copy(agent.state.rotation)
        rot.z = new_heading
        agent.state.rotation = rot

        return reward

    def sense(self, agent, observations):
        """
        figure out what the agent should sense
        """
        state = self.get_state(agent)
        x, y, _ = state.pose
        friends, foes = self.getFriendFoe(agent)
        if not friends:
            return v
        for f in friends:
            x += friends[f].pose[0]
            y += friends[f].pose[1]
        x /= len(friends)
        y /= len(friends)
        fd = self.distance(state.pose, (x, y))
        fh = self.angle(state.pose, (x, y)) + 180.0
        if fd <= 15:
            observations[-3] = fd / 15.0
            observations[-2] = fh / 360.0
        if observations[-2] < 0:
            observations[-2] += 1
        observations[-1] = int(self.target(agent) is not None)
        return observations

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

    def nearest(self, loc, id, agents):
        """
        Returns the nearest agent in a list of agents to an agent with a
        given id at a given location.
        """
        # TODO: this needs to only be computed once per tick, not per agent
        nearest = None
        min_dist = self.MAX_DIST * 5
        for a in agents:
            if id == agents[a].id:
                continue
            d = self.distance(loc, agents[a].pose)
            if d < min_dist:
                nearest = a
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
        if agent.group == "Turret": return False
        self.max_steps = module.getMod().lt
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        if not OpenNero.get_ai("rtneat").has_organism(agent):
            return True
        if module.getMod().hp != 0 and state.total_damage >= module.getMod().hp:
            return True
        else:
            return False

    def cleanup(self):
        """
        cleanup the world
        """
        common.killScript('NERO/menu.py')
        return True
