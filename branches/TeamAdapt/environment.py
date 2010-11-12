import time
from math import *
from copy import copy
from mazer import Maze
from constants import *
from OpenNero import *
from collections import deque

import observer

class MazeRewardStructure:
    """ This defines the reward that the agents get for running the maze """
    def valid_move(self, state):
        """ a valid move is just a -1 (to reward shorter routes) """
        return -1
    def out_of_bounds(self, state):
        """ reward for running out of bounds of the maze (hitting the outer wall) """
        return -5
    def hit_wall(self, state):
        """ reward for hitting any other wall """
        return -5
    def goal_reached(self, state):
        """ reward for reaching the goal """
        print 'GOAL REACHED!'
        # reaching a goal is great!
        return 100
    def last_reward(self, state):
        """ reward for ending without reaching the goal """
        (r,c) = state.rc
        print 'EPISODE ENDED AT', r, c
        return 100.0*(r+c)/(ROWS+COLS)

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self, maze):
        self.rc = (0, 0)
        self.prev_rc = (0, 0)
        (x,y) = maze.rc2xy(0,0)
        self.pose = (x,y,0)
        self.prev_pose = (x,y,0)
        self.initial_position = Vector3f(x, y, 0)
        self.initial_rotation = Vector3f(0, 0, 0)
        self.goal_reached = False
        self.time = time.time()
        self.start_time = self.time
        self.sensors = True
        self.animation = 'stand'
        self.observation_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.action_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.reward_history = deque([ 0 for x in range(HISTORY_LENGTH)])
        
    def reset(self):
        self.rc = (0,0)
        self.prev_rc = (0,0)
        self.goal_reached = False
        self.observation_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.action_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.reward_history = deque([ 0 for x in range(HISTORY_LENGTH)])

    def update(self, agent, maze):
        """
        Update the state of the agent
        """
        pos = copy(agent.state.position)
        self.prev_rc = self.rc
        self.rc = maze.xy2rc(pos.x, pos.y)
        self.prev_pose = self.pose
        self.pose = (pos.x, pos.y, agent.state.rotation.z + self.initial_rotation.z)
        self.time = time.time()
        
    def record_action(self, action):
        self.action_history.popleft()
        self.action_history.append(action)
        
    def record_observation(self, observation):
        self.observation_history.popleft()
        self.observation_history.append(observation)
    
    def record_reward(self, reward):
        self.reward_history.popleft()
        self.reward_history.append(reward)
        return reward
    
    def is_stuck(self):
        """ for now the only way to get stuck is to have the same state-action pair """
        if not is_uniform(self.action_history):
            return False
        if not is_uniform(self.observation_history):
            return False
        return True
        
    def get_reward(self):
        r0 = self.reward_history.popleft()
        for r in self.reward_history:
            assert(r == r0)
        return r0

class MazeEnvironment(Environment):
    MOVES = [(1,0), (-1,0), (0,1), (0,-1)]

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
        self.maze = Maze.generate(ROWS, COLS, GRID_DX, GRID_DY)
        self.rewards = MazeRewardStructure()
        self.states = {}
        action_info = FeatureVectorInfo()
        observation_info = FeatureVectorInfo()
        reward_info = FeatureVectorInfo()
        action_info.add_discrete(0, len(MazeEnvironment.MOVES)-1) # select from the moves we can make
        observation_info.add_discrete(0, ROWS-1)
        observation_info.add_discrete(0, COLS-1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        observation_info.add_discrete(0,1)
        reward_info.add_continuous(-100,100)
        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        self.max_steps = MAX_STEPS
        self.step_delay = STEP_DELAY
        self.speedup = 0
        self.shortcircuit = False
        print 'Initialized MazeEnvironment'
        
    def get_delay(self):
        return self.step_delay * (1.0 - self.speedup)

    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            self.states[agent] = AgentState(self.maze)
            assert(self.states[agent].sensors)
            if hasattr(agent, 'epsilon'):
                print 'epsilon:', self.epsilon
                agent.epsilon = self.epsilon
            return self.states[agent]

    def can_move(self, state, move):
        """
        Figure out if the agent can make the specified move
        """
        (r,c) = state.rc
        (dr,dc) = move
        return self.maze.rc_bounds(r+dc, c+dc) and not self.maze.is_wall(r,c,dr,dc)

    def get_next_rotation(self, move):
        """
        Figure out which way the agent should be facing in order to make the specified move
        """
        return Vector3f(0,0,degrees(atan2(move[1], move[0])))

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        state = self.get_state(agent)
        print 'Episode %d complete' % agent.episode
        state.reset()
        agent.state.position = copy(state.initial_position)
        agent.state.rotation = copy(state.initial_rotation)
        return True

    def get_agent_info(self, agent):
        return self.agent_info

    def set_animation(self, agent, state, animation):
        if state.animation != animation:
            agent.state.setAnimation(animation)
            state.animation = animation

    def step(self, agent, action):
        """
        Discrete version
        """
        state = self.get_state(agent)
        state.record_action(action)
        if not self.agent_info.actions.validate(action):
            state.prev_rc = state.rc
            return 0
        if agent.step == 0:
            state.initial_position = agent.state.position
            state.initial_rotation = agent.state.rotation
        (r,c) = state.rc
        a = int(round(action[0]))
        state.prev_rc = state.rc
        if a == len(MazeEnvironment.MOVES): # null action
            return state.record_reward(self.rewards.valid_move(state))
        (dr,dc) = MazeEnvironment.MOVES[a]
        new_r, new_c = r + dr, c + dc
        if not self.maze.rc_bounds(new_r, new_c):
            return state.record_reward(self.rewards.out_of_bounds(state))
        elif self.maze.is_wall(r,c,dr,dc):
            return state.record_reward(self.rewards.hit_wall(state))
        state.rc = (new_r, new_c)
        (old_r,old_c) = state.prev_rc
        (old_x,old_y) = self.maze.rc2xy(old_r, old_c)
        pos0 = agent.state.position
        pos0.x = old_x
        pos0.y = old_y
        agent.state.position = pos0
        relative_rotation = self.get_next_rotation((dr,dc))
        agent.state.rotation = state.initial_rotation + relative_rotation
        if new_r == ROWS - 1 and new_c == COLS - 1:
            state.goal_reached = True
            return state.record_reward(self.rewards.goal_reached(state))
        elif agent.step >= self.max_steps - 1:
            return state.record_reward(self.rewards.last_reward(state))
        return state.record_reward(self.rewards.valid_move(state))

    def teleport(self, agent, r, c):
        """
        move the agent to a new location
        """
        state = self.get_state(agent)
        state.prev_rc = (r,c)
        state.rc = (r,c)
        (x,y) = self.maze.rc2xy(r,c)
        pos0 = agent.state.position
        pos0.x = x
        pos0.y = y
        agent.state.position = pos0

    def sense(self, agent):
        """
        Discrete version
        """
        state = self.get_state(agent)
        v = self.agent_info.sensors.get_instance()
        v[0] = state.rc[0]
        v[1] = state.rc[1]
        offset = GRID_DX/10.0
        p0 = agent.state.position
        for i, (dr, dc) in enumerate(MazeEnvironment.MOVES):
            direction = Vector3f(dr, dc, 0)
            ray = (p0 + direction * offset, p0 + direction * GRID_DX)
            # we only look for objects of type 1, which means walls
            objects = getSimContext().findInRay(ray[0], ray[1], 1, True)
            v[2 + i] = int(len(objects) > 0)
        state.record_observation(v)
        return v

    def is_active(self, agent):
        state = self.get_state(agent)
        # here, we interpolate between state.prev_rc and state.rc
        (r0,c0) = state.prev_rc
        (r1,c1) = state.rc
        dr, dc = r1 - r0, c1 - c0
        if dr != 0 or dc != 0:
            (x0,y0) = self.maze.rc2xy(r0,c0)
            (x1,y1) = self.maze.rc2xy(r1,c1)
            pos = agent.state.position
            fraction = 1.0
            if self.get_delay() != 0:
                fraction = min(1.0,float(time.time() - state.time)/self.get_delay())
            pos.x = x0 * (1 - fraction) + x1 * fraction
            pos.y = y0 * (1 - fraction) + y1 * fraction
            agent.state.position = pos
            self.set_animation(agent, state, 'run')
        else:
            self.set_animation(agent, state, 'stand')
        if time.time() - state.time > self.get_delay():
            state.time = time.time()
            return True # call the sense/act/step loop
        else:
            return False

    def is_episode_over(self, agent):
        state = self.get_state(agent)
        if self.max_steps != 0 and agent.step >= self.max_steps:
            return True
        elif state.goal_reached:
            return True
        #elif self.shortcircuit and state.is_stuck():
        #    return False
        else:
            return False

    def cleanup(self):
        pass

class ContMazeEnvironment(MazeEnvironment):
    TURN_BY = 30 # how many degrees to turn by every time
    WALK_BY = 2.5 # how many units to advance by every step forward
    ACTIONS = {'FWD':0, 'CW':1, 'CCW':2, 'BCK':3}
    N_ACTIONS = 4 # number of actions
    N_RAYS = 4 # number of rays around the agent, starting from the front
    MAX_DISTANCE = hypot(ROWS*GRID_DX, COLS*GRID_DY) # max distance within the maze
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
    def __init__(self):
        """
        generate the maze
        """
        MazeEnvironment.__init__(self)
        action_info = FeatureVectorInfo() # describes the actions
        observation_info = FeatureVectorInfo() # describes the observations
        reward_info = FeatureVectorInfo() # describes the rewards
        action_info.add_discrete(0, ContMazeEnvironment.N_ACTIONS-1) # action
        ( (xmin, ymin), (xmax, ymax) ) = self.maze.xy_limits()
        print 'MAZE LIMITS', ( (xmin, ymin), (xmax, ymax) )
        observation_info.add_continuous(xmin, xmax) # x-coord
        observation_info.add_continuous(ymin, ymax) # y-coord
        observation_info.add_continuous(0, ContMazeEnvironment.MAX_DISTANCE ) # distance to target
        observation_info.add_continuous(-180, 180) # angle to target
        for i in range(ContMazeEnvironment.N_RAYS):
            observation_info.add_continuous(0,1) # ray sensor
        reward_info.add_continuous(-100,100)
        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        self.max_steps = MAX_STEPS * 15 # allow 15 actions per cell
        self.step_delay = STEP_DELAY/10.0 # smaller actions, but faster
        print 'Initialized ContMazeEnvironment'

    def get_next_rotation(self, move):
        """
        Figure out which way the agent should be facing in order to make the specified move
        """
        return Vector3f(0,0,degrees(atan2(move[1], move[0])))

    def reset(self, agent):
        """
        reset the environment to its initial state
        """
        state = self.get_state(agent)
        state.pose = (state.initial_position.x, state.initial_position.y, state.initial_rotation.z)
        agent.state.position = copy(state.initial_position)
        agent.state.rotation = copy(state.initial_rotation)
        state.goal_reached = False
        print 'Episode %d complete' % agent.episode
        return True

    def step(self, agent, action):
        """
        Continuous version
        """
        state = self.get_state(agent)
        state.record_action(action)
        if not self.agent_info.actions.validate(action):
            return 0
        a = int(round(action[0]))
        (x,y,heading) = state.pose # current pose
        new_x, new_y, new_heading = x, y, heading # pose to be computed
        dx, dy = None, None
        if a == ContMazeEnvironment.ACTIONS['CW']: # clockwise
            new_heading = wrap_degrees(heading, -ContMazeEnvironment.TURN_BY)
        elif a == ContMazeEnvironment.ACTIONS['CCW']: # counter-clockwise
            new_heading = wrap_degrees(heading, ContMazeEnvironment.TURN_BY)
        elif a == ContMazeEnvironment.ACTIONS['FWD']: # forward
            dx = ContMazeEnvironment.WALK_BY * cos(radians(new_heading))
            dy = ContMazeEnvironment.WALK_BY * sin(radians(new_heading))
        elif a == ContMazeEnvironment.ACTIONS['BCK']: # backward
            dx = -ContMazeEnvironment.WALK_BY * cos(radians(new_heading))
            dy = -ContMazeEnvironment.WALK_BY * sin(radians(new_heading))
        if dx or dy:
            test_x, test_y = x + 1.5 * dx, y + 1.5 * dy # leave a buffer of space
            new_x, new_y = x + dx, y + dy
            if not self.maze.xy_bounds(test_x, test_y):
                # could not move, out of bounds
                self.set_animation(agent, state, 'stand')
                return self.rewards.out_of_bounds(state)
            elif not self.maze.xy_valid(x,y,test_x,test_y):
                # could not move, hit a wall
                self.set_animation(agent, state, 'stand')
                return self.rewards.hit_wall(state)
            if new_x != x or new_y != y:
                self.set_animation(agent, state, 'run')
        if agent.step == 0:
            state.initial_position = agent.state.position
            state.initial_rotation = agent.state.rotation
        # move the agent
        agent.state.rotation = state.initial_rotation + Vector3f(0,0,new_heading)
        pos0 = agent.state.position
        pos0.x = new_x
        pos0.y = new_y
        agent.state.position = pos0
        # update agent state
        state.update(agent, self.maze)
        (new_r, new_c) = state.rc
        if new_r == ROWS - 1 and new_c == COLS - 1:
            state.goal_reached = True
            return self.rewards.goal_reached(state)
        elif agent.step >= self.max_steps - 1:
            return self.rewards.last_reward(state)
        return self.rewards.valid_move(state)

    def sense(self, agent):
        """
        Continuous version
        """
        state = self.get_state(agent)
        v = self.agent_info.sensors.get_instance()
        (x,y,heading) = state.pose # current agent pose
        v[0] = x # the agent can observe its position
        v[1] = y # the agent can observe its position
        (tx, ty) = self.maze.rc2xy(ROWS-1,COLS-1) # coordinates of target
        tx, ty = tx - x, ty - y # line to target
        v[2] = hypot(tx, ty) # distance to target
        angle_to_target = degrees(atan2(ty, tx)) # angle to target from +x, in degrees
        angle_to_target = wrap_degrees(angle_to_target, -heading) # heading to target relative to us
        v[3] = angle_to_target
        d_angle = 360.0 / ContMazeEnvironment.N_RAYS
        p0 = agent.state.position
        for i in range(ContMazeEnvironment.N_RAYS):
            angle = radians(heading + i * d_angle)
            direction = Vector3f(cos(angle), sin(angle), 0) # direction of ray
            ray = (p0, p0 + direction * GRID_DX)
            # we only look for objects of type 1, which means walls
            result = getSimContext().findInRay(ray[0], ray[1], 1, True)
            # we can now return a continuous sensor since FindInRay returns the hit point
            if len(result) > 0:
                (sim, hit) = result
                len1 = (ray[1] - ray[0]).getLength() # max extent
                len2 = (hit - ray[0]).getLength() # actual extent
                if len1 != 0:
                    v[4+i] = len2/len1
                else:
                    v[4+i] = 0
            else:
                v[4+i] = 1
        if not self.agent_info.sensors.validate(v):
            print 'ERROR: incorect observation!', v
            print '       should be:', self.agent_info.sensors
        state.record_observation(v)
        return v

    def is_active(self, agent):
        state = self.get_state(agent)
        # TODO: interpolate
        fraction = min(1.0,float(time.time() - state.time)/self.get_delay())
        if time.time() - state.time > self.get_delay():
            state.time = time.time()
            return True # call the sense/act/step loop
        else:
            return False

def is_uniform(vv):
    """ return true iff all the feature vectors in v are identical """
    l = len(vv)
    if l == 0:
        return False
    v0 = [x for x in vv[0]]
    for i in range(1, len(vv)):
        vi = [x for x in vv[i]]
        if v0 != vi:
            return False
    return True

def wrap_degrees(a,da):
    a2 = a + da
    if a2 > 180:
        a2 = -180 + (a2 % 180)
    elif a2 < -180:
        a2 = 180 - (abs(a2) % 180)
    return a2
