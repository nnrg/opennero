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

class AgentTrace:
    '''
    Class for storing the trace of an agent
    '''
    def __init__(self):
        self.initial_step = None
        self.position = []
        self.rotation = []
        self.velocity = []
        self.sensors = []
        self.actions = []

    def print_trace(self, filename):
        with open(filename, 'w') as file:
            for i in range(len(self.position)):
                file.write(str([self.position[i].x, self.position[i].y, self.position[i].z]))
                file.write(str([self.rotation[i].x, self.rotation[i].y, self.rotation[i].z]))
            for i in range(len(self.sensors)):
                file.write(str(self.sensors[i]))
                file.write(str(self.actions[i]))

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

        # we only want to compute the center of mass of friends once per tick
        self.friend_center = dict((team, None) for team in constants.TEAMS)
        # we need to know when to update the friend center
        self.friend_center_cache = dict((team, {}) for team in constants.TEAMS)
        
        self.tracing = False             # indicates if the agent is being traced
        self.simdisplay = True           # indicates if the simulation of agents is being displayed
        self.trace = None                # trace of agents
        self.use_trace = False           # use loaded trace for calculating reward
        self.run_backprop = False        # whether to run backprop if trace is loaded
        self.path_markers_trace = []     # ids of objects used to mark path of trace
        self.path_markers_champ = []     # ids of objects used to mark path of champ
        self.bbmarkers = []

    def set_weight(self, key, value):
        self.reward_weights[key] = value
        for team in self.teams:
            rtneat = OpenNero.get_ai("rtneat-%s" % team)
            if rtneat:
                rtneat.set_weight(constants.FITNESS_INDEX[key], value)

    def remove_all_agents(self, team):
        for agent in list(self.teams[team]):
            self.remove_agent(agent)

    def remove_agent(self, agent):
        common.removeObject(agent.state.id)
        try:
            self.teams[agent.get_team()].discard(agent)
            if agent in self.states:
                self.states.pop(agent)
        except:
            pass

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
            agent.teleport()
        return True

    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        for a in constants.WALL_RAY_SENSORS:
            agent.add_sensor(OpenNero.RaySensor(
                    math.cos(math.radians(a)), math.sin(math.radians(a)), 0,
                    constants.WALL_SENSOR_RADIUS,
                    constants.OBJECT_TYPE_OBSTACLE,
                    False))
        for a0, a1 in constants.FLAG_RADAR_SENSORS:
            agent.add_sensor(OpenNero.RadarSensor(
                    a0, a1, -90, 90, constants.MAX_VISION_RADIUS,
                    constants.OBJECT_TYPE_FLAG,
                    False))
        sense = constants.OBJECT_TYPE_TEAM_0
        if agent.get_team() == sense:
            sense = constants.OBJECT_TYPE_TEAM_1
        for a0, a1 in constants.ENEMY_RADAR_SENSORS:
            agent.add_sensor(OpenNero.RadarSensor(
                    a0, a1, -90, 90, constants.MAX_VISION_RADIUS,
                    sense,
                    False))
        for a in constants.TARGETING_SENSORS:
            agent.add_sensor(OpenNero.RaySensor(
                    math.cos(math.radians(a)), math.sin(math.radians(a)), 0,
                    constants.TARGET_SENSOR_RADIUS,
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

    def closest_enemy(self, agent):
        """
        Returns the nearest enemy to agent 
        """
        friends, foes = self.getFriendFoe(agent)
        if not foes:
            return None

        min_enemy = None
        min_dist = constants.MAX_FIRE_ACTION_RADIUS
        pose = self.get_state(agent).pose
        color = OpenNero.Color(128, 0, 0, 0)
        for f in foes:
            f_pose = self.get_state(f).pose
            dist = self.distance(pose, f_pose)
            if dist < min_dist:
                source_pos = agent.state.position
                target_pos = f.state.position
                source_pos.z = source_pos.z + 5
                target_pos.z = target_pos.z + 5
                obstacles = OpenNero.getSimContext().findInRay(
                    source_pos,
                    target_pos,
                    constants.OBJECT_TYPE_OBSTACLE,
                    False,
                    color,
                    color)
                if len(obstacles) == 0:
                    min_enemy = f
                    min_dist = dist
        return min_enemy

    def step(self, agent, action):
        """
        2A step for an agent
        """
        # if this agent has a serialized representation waiting, load it.
        chunk = self.agents_to_load.get(agent.state.id)
        if chunk is not None:
            print 'loading agent', agent.state.id, 'from', len(chunk), 'bytes'
            del self.agents_to_load[agent.state.id]
            try:
                agent.from_string(chunk)
            except:
                # if loading fails, remove this agent.
                print 'error loading agent', agent.state.id
                self.remove_agent(agent)

                # if a user has a badly formatted q-learning agent in a mixed
                # population file, the agent won't load and will be properly
                # removed here. however, RTNEAT has only allocated enough brainz
                # to cover (pop_size - num_qlearning_agents) agents, so whenever
                # it comes time to spawn new agents, RTNEAT will think that it
                # needs to spawn an extra agent to cover for this "missing" one.
                # to prevent this exception, we decrement pop_size here.
                #
                # this probably prevents teams from having the proper number of
                # agents if the user clicks on the deploy button after loading a
                # broken pop file ... but that's tricky to fix.
                constants.pop_size -= 1

                return agent.info.reward.get_instance()

        # set the epsilon for this agent, in case it's changed recently.
        agent.epsilon = self.epsilon

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

        # display agent info if neccessary
        if hasattr(agent, 'set_display_hint'):
            agent.set_display_hint()

        # spawn more agents if possible.
        self.maybe_spawn(agent)

        # get the desired action of the agent
        move_by = action[constants.ACTION_INDEX_SPEED]
        turn_by = math.degrees(action[constants.ACTION_INDEX_TURN])
        firing = action[constants.ACTION_INDEX_FIRE]
        firing_status = (firing >= 0.5)

        scored_hit = False
        # firing decision
        if firing_status:
            target = self.closest_enemy(agent)
            if target is not None:
                pose = state.pose
                target_pose = self.get_state(target).pose
                relative_angle = self.angle(pose, target_pose)
                if abs(relative_angle) <= 2:
                    source_pos = agent.state.position
                    target_pos = target.state.position
                    source_pos.z = source_pos.z + 5
                    target_pos.z = target_pos.z + 5
                    dist = target_pos.getDistanceFrom(source_pos)
                    d = (constants.MAX_SHOT_RADIUS - dist)/constants.MAX_SHOT_RADIUS
                    if random.random() < d/2: # attempt a shot depending on distance
                        team_color = constants.TEAM_LABELS[agent.get_team()]
                        if team_color == 'red':
                            color = OpenNero.Color(255, 255, 0, 0)
                        elif team_color == 'blue':
                            color = OpenNero.Color(255, 0, 0, 255)
                        else:
                            color = OpenNero.Color(255, 255, 255, 0)
                        wall_color = OpenNero.Color(128, 0, 255, 0)
                        obstacles = OpenNero.getSimContext().findInRay(
                            source_pos,
                            target_pos,
                            constants.OBJECT_TYPE_OBSTACLE,
                            True,
                            wall_color,
                            color)
                        #if len(obstacles) == 0 and random.random() < d/2:
                        if len(obstacles) == 0:
                            # count as hit depending on distance
                            self.get_state(target).curr_damage += 1
                            scored_hit = True
                else: # turn toward the enemy
                    turn_by = relative_angle

        # set animation speed
        # TODO: move constants into constants.py
        self.set_animation(agent, state, 'run')
        delay = OpenNero.getSimContext().delay
        agent.state.animation_speed = move_by * constants.ANIMATION_RATE

        reward = self.calculate_reward(agent, action, scored_hit)

        # tell the system to make the calculated motion
        state.update_pose(move_by, turn_by)

        return reward

    def calculate_reward(self, agent, action, scored_hit = False):
        reward = agent.info.reward.get_instance()

        state = self.get_state(agent)
        friends, foes = self.getFriendFoe(agent)

        if agent.group != 'Turret' and self.hitpoints > 0 and state.total_damage >= self.hitpoints:
            return reward

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

#        target = self.target(agent)
#        if target is not None:
#            source_pos = agent.state.position
#            target_pos = target.state.position
#            source_pos.z = source_pos.z + 5
#            target_pos.z = target_pos.z + 5
#            dist = target_pos.getDistanceFrom(source_pos)
#            d = (constants.MAX_SHOT_RADIUS - dist)/constants.MAX_SHOT_RADIUS
#            if random.random() < d/2: # attempt a shot depending on distance
#                team_color = constants.TEAM_LABELS[agent.get_team()]
#                if team_color == 'red':
#                    color = OpenNero.Color(255, 255, 0, 0)
#                elif team_color == 'blue':
#                    color = OpenNero.Color(255, 0, 0, 255)
#                else:
#                    color = OpenNero.Color(255, 255, 255, 0)
#                wall_color = OpenNero.Color(128, 0, 255, 0)
#                obstacles = OpenNero.getSimContext().findInRay(
#                    source_pos,
#                    target_pos,
#                    constants.OBJECT_TYPE_OBSTACLE,
#                    True,
#                    wall_color,
#                    color)
#                if len(obstacles) == 0 and random.random() < d/2:
#                    # count as hit depending on distance
#                    self.get_state(target).curr_damage += 1
#                    R[constants.FITNESS_HIT_TARGET] = 1

        if scored_hit:
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
        # update: this is now being done using a RaySensor
        #observations[constants.SENSOR_INDEX_TARGETING[0]] = int(self.target(agent) is not None)

#        friends, foes = self.getFriendFoe(agent)
#        if friends:
#            # the 2 before that are the angle and heading to the center of mass of
#            # the agent's team
#            ax, ay = agent.state.position.x, agent.state.position.y
#            cx, cy = self.get_friend_center(agent, friends)
#            fd = self.distance((ax, ay), (cx, cy))
#            ah = agent.state.rotation.z
#            fh = self.angle((ax, ay, ah), (cx, cy)) + 180.0
#            if fd <= constants.MAX_FRIEND_DISTANCE:
#                observations[constants.SENSOR_INDEX_FRIEND_RADAR[0]] = fd / 15.0
#                observations[constants.SENSOR_INDEX_FRIEND_RADAR[1]] = fh / 360.0


        # The code above miscalculates the friend center by looking at all teammates
        # instead of looking only inside the friend radius.
        # Updated friend sensing code:
        my_team = agent.get_team()
        all_friends = self.teams[my_team] 

        ax, ay = agent.state.position.x, agent.state.position.y
        cx, cy = 0.0, 0.0
        if all_friends:
            n = 0
            for f in all_friends:
                fx, fy = f.state.position.x, f.state.position.y
                if self.distance((ax, ay), (fx, fy)) <= constants.MAX_FRIEND_DISTANCE:
                    n += 1
                    cx += fx
                    cy += fy
            if n > 0:
                cx, cy = cx / n, cy / n
                fd = self.distance((ax, ay), (cx, cy))
                ah = agent.state.rotation.z
                fh = self.angle((ax, ay, ah), (cx, cy)) + 180.0
                value = 1 - (fd / constants.MAX_FRIEND_DISTANCE)
                value = max(0, min(value, 1))
                observations[constants.SENSOR_INDEX_FRIEND_RADAR[0]] = value
                observations[constants.SENSOR_INDEX_FRIEND_RADAR[1]] = fh / 360.0

        if self.tracing:
            if self.trace.initial_step is None:
                self.trace.initial_step = agent.step
            self.trace.position.append(agent.state.position)
            self.trace.rotation.append(agent.state.rotation)
            self.trace.sensors.append([o for o in observations])
        
        from agent import KeyboardAgent
        if self.tracing and isinstance(agent, KeyboardAgent):
            id = common.addObject(
                'data/shapes/cube/YellowCube.xml',
                position = agent.state.position,
                scale = OpenNero.Vector3f(0.25, 0.25, 0.25),
                type = constants.OBJECT_TYPE_LEVEL_GEOM)
            self.path_markers_trace.append(id)

        return observations

    def get_friend_center(self, agent, friends):
        ''' get the x, y position of the center of mass of the friends group '''
        id = agent.state.id
        step = agent.step
        team = agent.get_team()
        # only recompute the center of mass once we find an agent that goes
        # to the next step since the last computation
        if self.friend_center[team] is None or step != self.friend_center_cache[team].get(id, step):
            n = float(len(friends))
            cx, cy = 0, 0
            for f in friends:
                fx, fy = f.state.position.x, f.state.position.y
                cx += fx / n
                cy += fy / n
            self.friend_center[team] = (cx, cy)
            self.friend_center_cache[team].clear()
        self.friend_center_cache[team][id] = step
        return self.friend_center[team]

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
        if agent.group == 'Turret':
            return False

        team = agent.get_team()
        state = self.get_state(agent)
        dead = self.hitpoints > 0 and state.total_damage >= self.hitpoints
        old = self.lifetime > 0 and agent.step > 0 and 0 == agent.step % self.lifetime

        if agent.ai == 'qlearning':
            if dead or old:
                # simulate a respawn by moving this agent towards the spawn location.
                state.total_damage = 0
                state.randomize()
                agent.state.position = copy.copy(state.initial_position)
                agent.state.rotation = copy.copy(state.initial_rotation)
                agent.teleport()
            return False

        rtneat = OpenNero.get_ai("rtneat-%s" % team)
        orphaned = rtneat and not rtneat.has_organism(agent)

        return orphaned or dead or old
    
    def get_hitpoints(self, agent):
        damage = self.get_state(agent).total_damage
        if self.hitpoints > 0 and damage >= 0:
            return float(self.hitpoints-damage)/self.hitpoints
        else:
            return 0

    def cleanup(self):
        """
        cleanup the world
        """
        common.killScript((constants.MENU_JAR, constants.MENU_CLASS))
        return True
    
    def start_tracing(self):
        '''
        create trace object and start tracing
        tracing of only one agent is currently supported
        '''
        self.trace = AgentTrace()
        self.tracing = True

    def stop_tracing(self):
        '''
        stop tracing the agent
        '''
        self.tracing = False

    def save_trace(self, filename):
        '''
        pickle the current trace in the given file
        '''
        if not self.trace:
            print 'empty trace on save'
        with open(filename, 'w') as file:
            pickle.dump(self.trace, file)

    def use_demonstration(self):
        '''
        Start using the currently loaded demonstration
        '''
        if self.trace is not None:
            print 'starting to use demonstration'
            self.use_trace = True
        else:
            print 'no demonstration to use'
            
    def load_trace(self, filename):
        '''
        load previously pickled trace from the given file
        '''
        with open(filename, 'r') as file:
            self.trace = pickle.load(file)
        self.use_trace = True
        for pos in self.trace.position:
            id = common.addObject(
                "data/shapes/cube/YellowCube.xml",
                position = pos,
                scale = OpenNero.Vector3f(0.25,0.25,0.25),
                type = constants.OBJECT_TYPE_LEVEL_GEOM)
            self.path_markers_trace.append(id)

    def cancel_demonstration(self):
        '''
        unload previously loaded trace
        '''
        self.tracing = False
        self.trace = None
        self.use_trace = False
        while len(self.path_markers_trace) > 0:
            id = self.path_markers_trace.pop()
            common.removeObject(id)
