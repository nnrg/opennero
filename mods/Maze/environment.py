import time
import random
from math import *
from copy import copy
from mazer import Maze
from constants import *
from OpenNero import *
from common import *
from Maze.agent import MoveForwardAndStopAgent

class MazeRewardStructure:
    """ This defines the reward that the agents get for running the maze """
    def null_move(self, agent):
        """ reward for a null move """
        return -1
    def valid_move(self, agent):
        """ a valid move is just a -1 (to reward shorter routes) """
        return -1
    def out_of_bounds(self, agent):
        """ reward for running out of bounds of the maze (hitting the outer wall) """
        return -100
    def hit_wall(self, agent):
        """ reward for hitting any other wall """
        return -100
    def goal_reached(self, agent):
        """ reward for reaching the goal """
        print 'GOAL REACHED!'
        # reaching a goal is great!
        return 100
    def last_reward(self, agent):
        """ reward for ending without reaching the goal """
        #pos = agent.state.position
        #(r,c) = get_environment().maze.xy2rc(pos.x, pos.y)
        #print 'EPISODE ENDED AT', r, c
        #return 100.0*(r+c)/(ROWS+COLS)
        return -1

class MazeEnvironment(Environment):
    maze = Maze.generate(ROWS, COLS, GRID_DX, GRID_DY)

    """
    The environment is a 2-D maze.
    In the discrete version, the agent moves from cell to cell.
     * Actions (1 discrete action)
        * 0 - move in the +r direction
        * 1 - move in the -r direction
        * 2 - move in the +c direction
        * 3 - move in the -c direction
        * 4 - do nothing
     * Observations (6 discrete observations)
        * o[0] - the current row position
        * o[1] - the current col position
        * o[2] - obstacle in the +r direction?
        * o[3] - obstacle in the -r direction?
        * o[4] - obstacle in the +c direction?
        * o[5] - obstacle in the -c direction?
    """
    def __init__(self):
        """
        generate the maze
        """
        Environment.__init__(self)
        self.rewards = MazeRewardStructure()
        action_info = FeatureVectorInfo()
        observation_info = FeatureVectorInfo()
        reward_info = FeatureVectorInfo()
        action_info.add_discrete(0, len(MAZE_MOVES)-1) # select from the moves we can make
        observation_info.add_discrete(0, ROWS-1)
        observation_info.add_discrete(0, COLS-1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        reward_info.add_continuous(-100,100)
        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        self.max_steps = MAX_STEPS
        self.speedup = 0
        self.marker_map = {} # a map of cells and markers so that we don't have more than one per cell
        self.marker_states = {} # states of the marker agents that run for one cell and stop
        self.agent_map = {} # agents active on the map
        self.agents_at_goal = set() # the set of agents that have reached the goal
        self.handles = {} # handes for the objects used to draw q-values
        self.prepare_for_new_episode = False
        self.initdist = 0
        self.generate_init_pos()
        print 'Initialized MazeEnvironment'

    def can_move(self, agent, move):
        """
        Figure out if the agent can make the specified move
        """
        pos = agent.state.position
        (r,c) = MazeEnvironment.maze.xy2rc(pos.x, pos.y)
        (dr,dc) = move
        return MazeEnvironment.maze.rc_bounds(r+dc, c+dc) and not MazeEnvironment.maze.is_wall(r,c,dr,dc)

    def get_next_rotation(self, move):
        """
        Figure out which way the agent should be facing in order to make the specified move
        """
        return Vector3f(0,0,degrees(atan2(move[1], move[0])))

    def generate_init_pos(self):
        """
        Generate a random initial position in the maze
        """
        r = self.initdist
        c = 0
        if r > ROWS - 1:
            r = ROWS - 1
            c = self.initdist - (ROWS - 1)

        candidates = []
        while r >= 0 and r <= ROWS - 1 and c >= 0 and c <= COLS - 1:
            candidates.append((r, c))
            r -= 1
            c += 1

        self.init_pos = random.choice(candidates)
        print "Initial positions calculated as: %s" % (self.init_pos, )
        return self.init_pos

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        print 'Episode %d complete' % agent.episode

        if not self.prepare_for_new_episode:
            self.generate_init_pos()
        self.prepare_for_new_episode = False
        (r, c) = self.init_pos
        (x, y) = MazeEnvironment.maze.rc2xy(r, c)
        pos = Vector3f(x, y, 0)
        agent.state.position = pos
        agent.state.rotation = Vector3f(0, 0, 0)
        return True

    def get_agent_info(self, agent):
        return self.agent_info

    def set_animation(self, agent, animation):
        """
        set the agent's animation sequence to that named by animation
        """
        if agent.state.animation != animation:
            agent.state.animation = animation
            delay = getSimContext().delay
            animation_speed = agent.state.animation_speed
            if delay > 0:
                agent.state.animation_speed = animation_speed / delay

    def set_position(self, agent, new_pose):
        """
        set the next agent position to new_pose = (r,c,h)
        """
        new_r, new_c, new_heading = new_pose
        (new_x, new_y) = MazeEnvironment.maze.rc2xy(new_r, new_c)
        pos = agent.state.position
        if pos.x == new_x and pos.y == new_y:
            self.set_animation(agent, 'stand')
        else:
            pos.x, pos.y = new_x, new_y
            agent.state.position = pos
            self.set_animation(agent, 'run')

    def step(self, agent, action):
        """
        Discrete version
        """
        (r,c) = MazeEnvironment.maze.xy2rc(agent.state.position.x, agent.state.position.y)

        # check if we reached the goal
        if r == ROWS - 1 and c == COLS - 1 and not isinstance(agent, MoveForwardAndStopAgent):
            self.agents_at_goal.add(agent)
            return self.rewards.goal_reached(agent)

        # check if we ran out of time
        elif agent.step >= self.max_steps - 1 and not isinstance(agent, MoveForwardAndStopAgent):
            return self.rewards.last_reward(agent)


        if not self.agent_info.actions.validate(action):
            # check if we ran out of time
            if agent.step >= self.max_steps - 1 and not isinstance(agent, MoveForwardAndStopAgent):
                return self.rewards.last_reward(agent)
            # check if we reached the goal
            elif r == ROWS - 1 and c == COLS - 1 and not isinstance(agent, MoveForwardAndStopAgent):
                self.agents_at_goal.add(agent)
                return self.rewards.goal_reached(agent)
            else:
                self.set_animation(agent, 'stand')
                return self.rewards.null_move(agent)

        # check for null action
        a = int(round(action[0]))
        if a == MAZE_NULL_MOVE:
            self.set_animation(agent, 'stand')
            return self.rewards.null_move(agent)

        # calculate new pose
        (dr, dc) = MAZE_MOVES[a]
        new_r, new_c = r + dr, c + dc
        next_rotation = self.get_next_rotation((dr,dc))
        new_heading = next_rotation.z
        rotation = agent.state.rotation
        prev_heading = rotation.z

        # if the heading is right
        if new_heading == prev_heading:
            # check if we are in bounds
            if not MazeEnvironment.maze.rc_bounds(new_r, new_c):
                self.set_animation(agent, 'jump')
                return self.rewards.out_of_bounds(agent)
            # check if there is a wall in the way
            elif MazeEnvironment.maze.is_wall(r,c,dr,dc):
                self.set_animation(agent, 'jump')
                return self.rewards.hit_wall(agent)
            # if the heading is right, change the position
            self.set_position(agent, (new_r, new_c, new_heading))
        else:
            # if the heading is not right, just change the heading and run the
            # rotation animation:
            # "run" "stand" "turn_r_xc" "turn_l_xc" "turn_r_lx" "turn_l_lx"
            # "turn_r_xxx" "turn_l_xxx" "pick_up" "put_down"
            # "hold_run" "hold_stand" "hold_r_xc" "hold_l_xc"
            # "hold_turn_r_lx" "hold_turn_l_lx" "hold_turn_r_xxx" "hold_turn_l_xxx"
            # "jump" "hold_jump"
            if new_heading - prev_heading > 0:
                if new_heading - prev_heading > 90:
                    new_heading = prev_heading + 90
                self.set_animation(agent, 'turn_l_lx')
            else:
                if new_heading - prev_heading < 90:
                    new_heading = prev_heading - 90
                self.set_animation(agent, 'turn_r_lx')
            rot0 = copy(agent.state.rotation)
            rot0.z = new_heading
            agent.state.rotation = rot0
            agent.skip() # don't get a new action, just retry this one
            return self.rewards.valid_move(agent)

        # check if we reached the goal
        if new_r == ROWS - 1 and new_c == COLS - 1 and not isinstance(agent, MoveForwardAndStopAgent):
            self.agents_at_goal.add(agent)
            return self.rewards.goal_reached(agent)

        # check if we ran out of time
        elif agent.step >= self.max_steps - 1 and not isinstance(agent, MoveForwardAndStopAgent):
            return self.rewards.last_reward(agent)

        # return a normal reward
        return self.rewards.valid_move(agent)

    def teleport(self, agent, r, c):
        """
        move the agent to a new location
        """
        (x,y) = MazeEnvironment.maze.rc2xy(r,c)
        pos = agent.state.position
        pos.x = x
        pos.y = y
        agent.state.position = pos
        agent.teleport()

    def sense(self, agent, obs):
        """
        Discrete version
        """
        p0 = agent.state.position
        (r,c) = MazeEnvironment.maze.xy2rc(p0.x, p0.y)
        obs[0] = r
        obs[1] = c
        offset = GRID_DX/10.0
        for i, (dr, dc) in enumerate(MAZE_MOVES):
            direction = Vector3f(dr, dc, 0)
            ray = (p0 + direction * offset, p0 + direction * GRID_DX)
            # we only look for objects of type 1, which means walls
            objects = getSimContext().findInRay(ray[0], ray[1], 1, False)
            obs[2 + i] = int(len(objects) > 0)
        return obs

    def is_episode_over(self, agent):
        pos = agent.state.position
        (r,c) = MazeEnvironment.maze.xy2rc(pos.x, pos.y)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        elif agent.__class__.__name__ == 'MoveForwardAndStopAgent':
            return False
        elif r == ROWS-1 and c == COLS-1:
            if hasattr(agent, "highlight_path"):
                disable_ai() # stop running
                self.prepare_for_new_episode = True
                agent.highlight_path() # mark the final path
                self.set_animation(agent, 'stand') # stop animation
            return True
        else:
            return False

    def mark_maze(self, r, c, marker):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        if (r,c) in self.marker_map:
            removeObject(self.marker_map[(r,c)])
        # remember the ID of the marker
        self.marker_map[(r,c)] = addObject(marker, Vector3f( (r+1) * GRID_DX, (c+1) * GRID_DY, -1))

    def mark_maze_blue(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/BlueCube.xml")

    def mark_maze_green(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/GreenCube.xml")

    def mark_maze_yellow(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/YellowCube.xml")

    def mark_maze_white(self, r, c):
        self.mark_maze(r,c,"data/shapes/cube/WhiteCube.xml")

    def unmark_maze_agent(self, r, c):
        """ mark a maze cell with the specified color """
        # remove the previous object, if necessary
        if (r,c) in self.agent_map:
            removeObject(self.agent_map[(r,c)])
            del self.marker_states[self.agent_map[(r,c)]]
            del self.agent_map[(r,c)]

    def mark_maze_agent(self, agent, r1, c1, r2, c2):
        """ mark a maze cell with an agent moving from r1, c1 to r2, c2 """
        # remove the previous object, if necessary
        self.unmark_maze_agent(r2,c2)
        # add a new marker object
        position = Vector3f( (r1+1) * GRID_DX, (c1+1) * GRID_DY, 0)
        rotation = self.get_next_rotation( (r2-r1, c2-c1) )
        agent_id = addObject(agent, position = position, rotation = rotation)
        self.marker_states[agent_id] = ((r1, c1), (r2, c2))
        self.agent_map[(r2,c2)] = agent_id

    def cleanup(self):
        # remove the marker blocks
        for id in self.marker_map.values():
            removeObject(id)
        self.marker_map = {}
        for id in self.agent_map.values():
            removeObject(id)
        for o in self.handles:
            for a in range(len(self.handles[o])):
                h = self.handles[o][a]
                if h is not None:
                    removeObject(h)
        self.handles = {}
        self.agent_map = {}

    def draw_q(self, o, Q):
        aa = Q[o] # get the action values
        min_a = min(aa) # minimum of the action values
        max_a = max(aa)
        aa = [a - min_a for a in aa] # shift to make all >= 0
        sum_a = sum(aa) # sum of action values
        if sum_a != 0: aa = [a/sum_a for a in aa] # normalize
        if o not in self.handles: # create handles list
            self.handles[o] = [None, None, None, None, None]
        (x, y) = self.maze.rc2xy(o[0], o[1])
        center = len(MAZE_MOVES)
        if self.handles[o][center] is not None:
            removeObject(self.handles[o][center])
        lowest_a = 0
        scale_factor = float(max_a - lowest_a) / (100 - lowest_a)
        if max_a > 0:
            cube_file = "data/shapes/cube/GreenCube.xml"
            cube_size = max(scale_factor * 2.4, 0.8)
        else:
            cube_file = "data/shapes/cube/YellowCube.xml"
            cube_size = max(1.4 + scale_factor * 2, 0.6)
        self.handles[o][center] = addObject(cube_file, Vector3f(x, y, 0), \
                scale=Vector3f(cube_size, cube_size, 0.6))
        #print "Adding cube with max_a, scale_factor, size: %s, %s, %s" % (max_a, scale_factor, cube_size)
        for a, (dr, dc) in enumerate(MAZE_MOVES):
            p = Vector3f(x, y, 0)
            value = aa[a] * 6.5
            if dr == 0: dr = 0.1
            else: p.x += dr*value
            if dc == 0: dc = 0.1
            else: p.y += dc*value
            if value == 0 and self.handles[o][a] is not None:
                # don't show 0 values
                removeObject(self.handles[o][a])
                self.handles[o][a] = None
            elif self.handles[o][a] is None:
                # create the cube to show the value
                self.handles[o][a] = \
                    addObject("data/shapes/cube/BlueCube.xml", \
                    p, Vector3f(0, 0, 0), scale=Vector3f(0.5, 0.5, 0.5))
            else:
                # move the existing cube
                getSimContext().setObjectPosition(self.handles[o][a], p)

class EgocentricMazeEnvironment(MazeEnvironment):
    """
    The environment is a 2-D maze.
    This is a slightly more continous version
     * Actions (1 discrete action)
        * 0 - move forward by WALK_BY
        * 1 - turn CW by TURN_BY and move forward by WALK_BY
        * 2 - turn CCW by TURN_BY and move forward by WALK_BY
        * 3 - move backward by WALK_BY
     * Observations ()
        * o[0] - the current x position
        * o[1] - the current y position
        * o[2] - the angle to the target
        * o[3] - the distance to the target
        * o[4] - o[7] - ray sensors cast around the agent (starting with straight ahead and going clockwise)
    """
    def __init__(self, granularity = 1):
        """
        Constructor
        @param granularity - the number of steps it takes to cover the whole WALK_BY distance
        """
        MazeEnvironment.__init__(self)
        action_info = FeatureVectorInfo() # describes the actions
        observation_info = FeatureVectorInfo() # describes the observations
        reward_info = FeatureVectorInfo() # describes the rewards
        action_info.add_discrete(0, CONT_MAZE_N_ACTIONS-1) # action
        ( (xmin, ymin), (xmax, ymax) ) = MazeEnvironment.maze.xy_limits()
        print 'MAZE LIMITS', ( (xmin, ymin), (xmax, ymax) )
        observation_info.add_continuous(xmin, xmax) # x-coord
        observation_info.add_continuous(ymin, ymax) # y-coord
        observation_info.add_continuous(0, CONT_MAZE_MAX_DISTANCE ) # distance to target
        observation_info.add_continuous(-180, 180) # angle to target
        for i in range(CONT_MAZE_N_RAYS):
            observation_info.add_continuous(0,1) # ray sensor
        reward_info.add_continuous(-100,100)
        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        self.granularity = granularity
        self.max_steps = MAX_STEPS * 15 * self.granularity # allow 15 * g actions per cell
        self.initdist = 0
        self.generate_init_pos()
        print 'Initialized EgocentricMazeEnvironment'
                
    def generate_init_pos(self):
        """
        Generate a random initial position in the maze
        """
        r = self.initdist
        c = 0
        if r > ROWS - 1:
            r = ROWS - 1
            c = self.initdist - (ROWS - 1)

        candidates = []
        while r >= 0 and r <= ROWS - 1 and c >= 0 and c <= COLS - 1:
            candidates.append((r, c))
            r -= 1
            c += 1

        self.init_pos = random.choice(candidates)
        print "Initial position calculated as: %s" % (self.init_pos, )
        return self.init_pos

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        print 'Episode %d complete' % agent.episode

        if not self.prepare_for_new_episode:
            self.generate_init_pos()
        self.prepare_for_new_episode = False
        (r, c) = self.init_pos
        (x, y) = MazeEnvironment.maze.rc2xy(r, c)
        agent.state.position = Vector3f(x,y,0)
        agent.state.rotation = Vector3f(0,0,0)
        self.agents_at_goal.discard(agent)
        print 'Episode %d complete' % agent.episode
        return True

    def step(self, agent, action):
        """
        Continuous version
        """
        if not self.agent_info.actions.validate(action):
            if agent.step >= self.max_steps - 1:
                return self.max_steps * self.rewards.last_reward(agent)
            else:
                return self.rewards.null_move(agent)
        a = int(round(action[0]))
        pos = agent.state.position # current position
        rot = agent.state.rotation # current rotation
        (x,y,heading) = (pos.x, pos.y, rot.z) # current pose
        new_x, new_y, new_heading = x, y, heading # pose to be computed
        dx, dy = 0, 0
        if a == CONT_MAZE_ACTIONS['CW']: # clockwise
            new_heading = wrap_degrees(heading, -CONT_MAZE_TURN_BY)
        elif a == CONT_MAZE_ACTIONS['CCW']: # counter-clockwise
            new_heading = wrap_degrees(heading, CONT_MAZE_TURN_BY)
        elif a == CONT_MAZE_ACTIONS['FWD']: # forward
            dx = CONT_MAZE_WALK_BY * cos(radians(new_heading)) / self.granularity
            dy = CONT_MAZE_WALK_BY * sin(radians(new_heading)) / self.granularity
        elif a == CONT_MAZE_ACTIONS['BCK']: # backward
            dx = -CONT_MAZE_WALK_BY * cos(radians(new_heading)) / self.granularity
            dy = -CONT_MAZE_WALK_BY * sin(radians(new_heading)) / self.granularity
        if dx != 0 or dy != 0:
            new_x, new_y = x + dx, y + dy # this is where we are moving to
            print 'move test', x, y, dx, dy,
            # leave a buffer of space to check in the right direction
            if dx != 0: dx = dx * 1.1 # leave a buffer for testing
            if dy != 0: dy = dy * 1.1 # leave a buffer for testing
            test_x, test_y = x + dx, y + dy # this is to check if there are walls there
            print dx, dy, test_x, test_y
            if not MazeEnvironment.maze.xy_bounds(test_x, test_y):
                print "could not move, out of bounds"
                self.set_animation(agent, 'stand')
                return self.rewards.out_of_bounds(agent)
            elif not MazeEnvironment.maze.xy_valid(x,y,test_x,test_y):
                print "could not move, hit a wall"
                self.set_animation(agent, 'stand')
                return self.rewards.hit_wall(agent)
            if new_x != x or new_y != y:
                self.set_animation(agent, 'run')
        # move the agent
        agent.state.rotation = Vector3f(0,0,new_heading)
        pos0 = agent.state.position
        pos0.x = new_x
        pos0.y = new_y
        agent.state.position = pos0
        (new_r, new_c) = MazeEnvironment.maze.xy2rc(new_x, new_y)
        if new_r == ROWS - 1 and new_c == COLS - 1:
            self.agents_at_goal.add(agent)
            return self.max_steps * self.rewards.goal_reached(agent)
        elif agent.step >= self.max_steps - 1:
            return self.max_steps * self.rewards.last_reward(agent)
        return self.rewards.valid_move(agent)

    def sense(self, agent, obs):
        """
        Continuous version
        """
        pos = agent.state.position # current position
        rot = agent.state.rotation # current rotation
        (x,y,heading) = (pos.x, pos.y, rot.z) # current pose
        obs[0] = x # the agent can observe its position
        obs[1] = y # the agent can observe its position
        (tx, ty) = MazeEnvironment.maze.rc2xy(ROWS-1,COLS-1) # coordinates of target
        tx, ty = tx - x, ty - y # line to target
        obs[2] = hypot(tx, ty) # distance to target
        angle_to_target = degrees(atan2(ty, tx)) # angle to target from +x, in degrees
        angle_to_target = wrap_degrees(angle_to_target, -heading) # heading to target relative to us
        obs[3] = angle_to_target
        d_angle = 360.0 / CONT_MAZE_N_RAYS
        p0 = agent.state.position
        for i in range(CONT_MAZE_N_RAYS):
            angle = radians(heading + i * d_angle)
            direction = Vector3f(cos(angle), sin(angle), 0) # direction of ray
            ray = (p0, p0 + direction * GRID_DX)
            # we only look for objects of type 1, which means walls
            result = getSimContext().findInRay(ray[0], ray[1], 1, False)
            # we can now return a continuous sensor since FindInRay returns the hit point
            if len(result) > 0:
                (sim, hit) = result
                len1 = (ray[1] - ray[0]).getLength() # max extent
                len2 = (hit - ray[0]).getLength() # actual extent
                if len1 != 0:
                    obs[4+i] = len2/len1
                else:
                    obs[4+i] = 0
            else:
                obs[4+i] = 1
        if not self.agent_info.sensors.validate(obs):
            print 'ERROR: incorect observation!', obs
            print '       should be:', self.agent_info.sensors
        return obs

class GranularMazeEnvironment(MazeEnvironment):
    """
    The environment is a 2-D maze.
    In the discrete version, the agent moves from cell to cell.
     * Actions (1 discrete action)
        * 0 - move in the +r direction
        * 1 - move in the -r direction
        * 2 - move in the +c direction
        * 3 - move in the -c direction
     * Observations (6 discrete observations)
        * o[0] - the current x position
        * o[1] - the current y position
        * o[2] - obstacle in the +r direction?
        * o[3] - obstacle in the -r direction?
        * o[4] - obstacle in the +c direction?
        * o[5] - obstacle in the -c direction?
    """
    def __init__(self, granularity = 8):
        """
        generate the maze
        """
        MazeEnvironment.__init__(self)
        action_info = FeatureVectorInfo() # describes the actions
        observation_info = FeatureVectorInfo() # describes the observations
        reward_info = FeatureVectorInfo() # describes the rewards
        action_info.add_discrete(0, CONT_MAZE_N_ACTIONS-1) # action
        ( (xmin, ymin), (xmax, ymax) ) = MazeEnvironment.maze.xy_limits()
        print 'MAZE LIMITS', ( (xmin, ymin), (xmax, ymax) )
        observation_info.add_continuous(xmin, xmax) # x-coord
        observation_info.add_continuous(ymin, ymax) # y-coord
        for (dr, dc) in MAZE_MOVES:
            observation_info.add_continuous(0,1) # movement sensor
        reward_info.add_continuous(-100,100)
        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        self.max_steps = MAX_STEPS * (granularity * 2) # allow 2x granularity steps per cell
        self.granularity = granularity
        self.initdist = 0
        self.generate_init_pos()
        print 'Initialized GranularMazeEnvironment'
                
    def generate_init_pos(self):
        """
        Generate a random initial position in the maze
        """
        r = self.initdist
        c = 0
        if r > ROWS - 1:
            r = ROWS - 1
            c = self.initdist - (ROWS - 1)

        candidates = []
        while r >= 0 and r <= ROWS - 1 and c >= 0 and c <= COLS - 1:
            candidates.append((r, c))
            r -= 1
            c += 1

        self.init_pos = random.choice(candidates)
        print "Initial positions calculated as: %s" % (self.init_pos, )
        return self.init_pos

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        print 'Episode %d complete' % agent.episode

        if not self.prepare_for_new_episode:
            self.generate_init_pos()
        self.prepare_for_new_episode = False
        (r, c) = self.init_pos
        (x, y) = MazeEnvironment.maze.rc2xy(r, c)
        agent.state.position = Vector3f(x,y,0)
        agent.state.rotation = Vector3f(0,0,0)
        self.agents_at_goal.discard(agent)
        print 'Episode %d complete' % agent.episode
        return True

    def set_position(self, agent, new_pose):
        """
        set the next agent position to new_pose = (x,y,h)
        """
        new_x, new_y, new_heading = new_pose
        pos = agent.state.position
        if pos.x == new_x and pos.y == new_y:
            self.set_animation(agent, 'stand')
        else:
            pos.x, pos.y = new_x, new_y
            agent.state.position = pos
            self.set_animation(agent, 'run')

    def step(self, agent, action):
        """
        Granular version
        """
        (x,y) = agent.state.position.x, agent.state.position.y
        (r,c) = MazeEnvironment.maze.xy2rc(x, y)
        a = int(action[0])

        # check if we reached the goal
        if r == ROWS - 1 and c == COLS - 1:
            self.agents_at_goal.add(agent)
            return self.rewards.goal_reached(agent) * self.max_steps

        # check if we ran out of time
        elif agent.step >= self.max_steps - 1:
            return self.rewards.last_reward(agent) * self.max_steps

        # check if the action was a null action
        if not self.agent_info.actions.validate(action):
            # check if we ran out of time
            if agent.step >= self.max_steps - 1:
                return self.rewards.last_reward(agent) * self.max_steps
            # check if we reached the goal
            elif r == ROWS - 1 and c == COLS - 1:
                self.agents_at_goal.add(agent)
                return self.rewards.goal_reached(agent) * self.max_steps
            else:
                self.set_animation(agent, 'stand')
                return self.rewards.null_move(agent)

        # calculate new pose
        (dr, dc) = MAZE_MOVES[a]
        dx, dy = dr * GRID_DX / self.granularity, dc * GRID_DY / self.granularity
        new_x, new_y = x + dx, y + dy
        (new_r, new_c) = MazeEnvironment.maze.xy2rc(new_x, new_y)
        next_rotation = self.get_next_rotation((dx,dy))
        new_heading = next_rotation.z
        rotation = agent.state.rotation
        prev_heading = rotation.z
        
        # if the heading is right
        if new_heading == prev_heading:
            # check if we are in bounds
            if not MazeEnvironment.maze.xy_bounds(new_x, new_y):
                self.set_animation(agent, 'jump')
                return self.rewards.out_of_bounds(agent)
            # check if there is a wall in the way
            elif MazeEnvironment.maze.is_wall(r,c, new_r - r, new_c - c):
                self.set_animation(agent, 'jump')
                return self.rewards.hit_wall(agent)
            # if the heading is right, change the position
            self.set_position(agent, (new_x, new_y, new_heading))
        else:
            # if the heading is not right, just change the heading and run the
            # rotation animation:
            # "run" "stand" "turn_r_xc" "turn_l_xc" "turn_r_lx" "turn_l_lx"
            # "turn_r_xxx" "turn_l_xxx" "pick_up" "put_down"
            # "hold_run" "hold_stand" "hold_r_xc" "hold_l_xc"
            # "hold_turn_r_lx" "hold_turn_l_lx" "hold_turn_r_xxx" "hold_turn_l_xxx"
            # "jump" "hold_jump"
            if new_heading - prev_heading > 0:
                if new_heading - prev_heading > 90:
                    new_heading = prev_heading + 90
                self.set_animation(agent, 'turn_l_lx')
            else:
                if new_heading - prev_heading < 90:
                    new_heading = prev_heading - 90
                self.set_animation(agent, 'turn_r_lx')
            rot0 = copy(agent.state.rotation)
            rot0.z = new_heading
            agent.state.rotation = rot0
            agent.skip() # don't get a new action, just retry this one
            return self.rewards.valid_move(agent)

        # check if we reached the goal
        if new_r == ROWS - 1 and new_c == COLS - 1:
            self.agents_at_goal.add(agent)
            return self.rewards.goal_reached(agent) * self.max_steps

        # check if we ran out of time
        elif agent.step >= self.max_steps - 1:
            return self.rewards.last_reward(agent) * self.max_steps

        # return a normal reward
        return self.rewards.valid_move(agent)

    def sense(self, agent, obs):
        """
        Compute granular maze environment sensor observations.
        Fill in the obs vector with 6 values, where:
          obs[0] is the x position
          obs[1] is the y position
          obs[2:5] are "free space" sensors in +r, -r, +c, -c 
            directions, with 0.0 meaning "wall is very near" and 1.0
            meaning "free as far as the sensor can see"
        """
        pos = agent.state.position # current position
        rot = agent.state.rotation # current rotation

        (x,y,heading) = (pos.x, pos.y, rot.z) # current pose
        obs[0] = x # the agent can observe its position
        obs[1] = y # the agent can observe its position

        # current position of the agent
        p0 = agent.state.position

        # calculate free space in the possible move directions
        for i, (dr, dc) in enumerate(MAZE_MOVES):
            direction = Vector3f(dr, dc, 0)
            p1 = p0 + direction * GRID_DX / self.granularity
            hit_result = getSimContext().findInRay(p0, p1, 1, True)
            if len(hit_result) > 0:
                # if the ray hit a wall, return what fraction was clear
                (sim, hit) = hit_result
                len1 = (p1 - p0).getLength() # max extent
                len2 = (hit - p0).getLength() # actual extent
                obs[2 + i] = len2/len1 if len1 != 0 else 0.0
            else:
                # if the ray did not hit a wall, return 1.0
                obs[2 + i] = 1.0

        if not self.agent_info.sensors.validate(obs):
            print 'ERROR: incorect observation!', obs
            print '       should be:', self.agent_info.sensors
        return obs

    def draw_q(self, o, Q):
        aa = Q[o] # get the action values
        min_a = min(aa) # minimum of the action values
        aa = [a - min_a for a in aa] # shift to make all >= 0
        sum_a = sum(aa) # sum of action values
        if sum_a != 0: aa = [a/sum_a for a in aa] # normalize
        if o not in self.handles: # create handles list
            self.handles[o] = [None, None, None, None, None]
        x, y = o[0], o[1]
        for a, (dr, dc) in enumerate(MAZE_MOVES):
            p = Vector3f(x, y, 0)
            value = aa[a] * 10 / self.granularity
            p.x += dr*value
            p.y += dc*value
            if self.handles[o][a] is None:
                self.handles[o][a] = \
                    addObject("data/shapes/cube/BlueCube.xml", \
                    p, Vector3f(0, 0, 0), \
                    scale=Vector3f(0.5, 0.5, 0.5)/self.granularity)
            else:
                getSimContext().setObjectPosition(self.handles[o][a], p)
        center = len(MAZE_MOVES)
        if self.handles[o][center] is None:
            self.handles[o][center] = \
                addObject("data/shapes/cube/YellowCube.xml", \
                    Vector3f(x, y, 0), \
                    scale=Vector3f(0.4,0.4,0.4)/self.granularity)

def wrap_degrees(a,da):
    a2 = a + da
    if a2 > 180:
        a2 = -180 + (a2 % 180)
    elif a2 < -180:
        a2 = 180 - (abs(a2) % 180)
    return a2
