import time
from math import *
from copy import copy
from constants import *
from OpenNero import *
from collections import deque
from common import *

import observer

class TowerRewardStructure:
    """ This defines the reward that the agents get for running the maze """
    def valid_move(self, state):
        """ a valid move is just a -1 (to reward shorter routes) """
        return -1
    def goal_reached(self, state):
        """ reward for reaching the goal """
        print 'GOAL REACHED!'
        # reaching a goal is great!
        return 100

class BlockState:
    """
    State that we keep for each block
    """
    def __init__(self, maze):
        self.rc = (0,0)
        self.height = 0
        self.below = None
        self.above = None
        self.name = ''
        self.obj = None
        self.mass = 0


class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.rc = (0, 0)
        self.prev_rc = (0, 0)
        (x,y) = self.rc2xy(0,0)
        self.pose = (x,y,0)
        self.prev_pose = (x,y,0)
        self.initial_position = Vector3f(x, y, 0)
        self.initial_rotation = Vector3f(0, 0, 0)
        self.goal_reached = False
        self.time = time.time()
        self.start_time = self.time
        self.sensors = True
        self.animation = 'stand'
        self.current_action = 'stand'
        self.observation_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.action_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.reward_history = deque([ 0 for x in range(HISTORY_LENGTH)])
        self.next_rotation = 0
        self.prev_rotation = 0
        self.holding = None
    
    def xy2rc(self, x, y):
        "convert x y to row col"
        return (int(round(x/GRID_DX))-1, int(round(y/GRID_DY))-1)

    def rc2xy(self, r, c):
        "convert row, col to x,y"
        return ((r+1) * GRID_DX, (c+1) * GRID_DY)

    def reset(self):
        self.rc = (0,0)
        self.prev_rc = (0,0)
        self.goal_reached = False
        self.observation_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.action_history = deque([ [x] for x in range(HISTORY_LENGTH)])
        self.reward_history = deque([ 0 for x in range(HISTORY_LENGTH)])

    def update(self, agent):
        """
        Update the state of the agent
        """
        pos = copy(agent.state.position)
        self.prev_rc = self.rc
        self.rc = self.xy2rc(pos.x, pos.y)
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

class TowerEnvironment(Environment):
    MOVES = [(1,0), (0,1), (-1,0), (0,-1)]

    """
    The environment is a 2-D maze.
    In the discrete version, the agent moves from cell to cell.
     * Observations (6 discrete observations)
        * o[0] - the current row position
        * o[1] - the current col position
        * o[2] - obstacle in the +r direction?
        * o[3] - obstacle in the -r direction?
        * o[4] - obstacle in the +c direction?
        * o[5] - obstacle in the -c direction?


        IGNORE THE ABOVE:
        Actions (1 discrete action)
        * 0 do nothing
        * 1 walk forward
        * 2 walk forward and set down
        * 3 walk forward and pick up
        * 4 turn right
        * 5 turn left
    """
    def __init__(self):
        """
        generate the maze
        """

        from module import getMod
        
        Environment.__init__(self)
        self.problem = None#Tower.generate(ROWS, COLS, GRID_DX, GRID_DY)
        self.rewards = TowerRewardStructure()
        self.states = {}
        self.block_states = {}
        action_info = FeatureVectorInfo()
        observation_info = FeatureVectorInfo()
        reward_info = FeatureVectorInfo()
        action_info.add_discrete(0, 5) # select from the moves we can make
        observation_info.add_discrete(0, ROWS-1)
        observation_info.add_discrete(0, COLS-1)
        observation_info.add_discrete(0,360)
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
        

    def initilize_blocks(self):
        from module import getMod
        num_towers = getMod().num_towers
        #blue = addObject("data/shapes/cube/BlueCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 1 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.75*2.5,.75*2.5,.75*2.5))
        #green = addObject("data/shapes/cube/GreenCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 2 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.7*2.5,.7*2.5,.7*2.5))
        #yellow = addObject("data/shapes/cube/YellowCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 3 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.65*2.5,.65*2.5,.65*2.5))
        #if num_towers > 3: red = addObject("data/shapes/cube/RedCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 4 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.6,.6,.6))
        #if num_towers > 4: white = addObject("data/shapes/cube/BlueCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 5 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.55*2.5,.55*2.5,.55*2.5))
        
        blue = addObject("data/shapes/cube/BlueCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 1 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(1.0*2.5,1.0*2.5,.25*2.5))
        green = addObject("data/shapes/cube/GreenCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 2 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.9*2.5,.9*2.5,.25*2.5))
        yellow = addObject("data/shapes/cube/YellowCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 3 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.8*2.5,.8*2.5,.25*2.5))
        if num_towers > 3: red = addObject("data/shapes/cube/RedCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 4 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.7,.7,.25))
        if num_towers > 4: white = addObject("data/shapes/cube/BlueCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 5 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(.6*2.5,.6*2.5,.25*2.5))

        print "BLUE == ", blue
        print "GREEN == ", green
        print "YELLOW == ", yellow
        if num_towers > 3: print "RED == ", red
        if num_towers > 4: print "WHITE == ", white

        #Can use id to set things. Use getSimContext().setObjectPosition()  from ID. 

        """
        blue = '''getMod().'''addObject("data/shapes/cube/BlueCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 1 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(2.5,2.5,2.5))
        green = '''getMod().'''addObject("data/shapes/cube/GreenCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 2 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(2.4,2.4,2.4))
        yellow = '''getMod().'''addObject("data/shapes/cube/YellowCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 3 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(2.3,2.3,2.3))
        red = '''getMod().'''addObject("data/shapes/cube/RedCube.xml", Vector3f(1 * GRID_DX, 2 * GRID_DY, 4 * GRID_DZ), Vector3f(0,0,0),scale=Vector3f(2.2/2.5,2.2/2.5,2.2/2.5))
        """
        self.block_states[blue] = BlockState(self.problem)
        bstate = self.block_states[blue]
        bstate.rc = (0,1)
        bstate.height = 0
        bstate.name = "blue"
        bstate.obj = blue
        bstate.mass = 5
       
        self.block_states[green] = BlockState(self.problem)
        gstate = self.block_states[green]
        gstate.rc = (0,1)
        gstate.height = 1
        gstate.name = "green"
        gstate.obj = green
        gstate.mass = 4
        
        if num_towers > 2:
            self.block_states[yellow] = BlockState(self.problem)
            ystate = self.block_states[yellow]
            ystate.rc = (0,1)
            ystate.height = 2
            ystate.name = "yellow"
            ystate.obj = yellow
            ystate.mass = 3
    
        if num_towers > 3:
            self.block_states[red] = BlockState(self.problem)
            rstate = self.block_states[red]
            rstate.rc = (0,1)
            rstate.height = 3
            rstate.name = "red"
            rstate.obj = red
            rstate.mass = 2
        
        if num_towers > 4:
            self.block_states[white] = BlockState(self.problem)
            wstate = self.block_states[white]
            wstate.rc = (0,1)
            wstate.height = 4
            wstate.name = "white"
            wstate.obj = white
            wstate.mass = 1

        bstate.above = gstate
        gstate.below = bstate
        gstate.above = ystate
        ystate.below = gstate
        if num_towers > 3: ystate.above = rstate
        if num_towers > 3: rstate.below = ystate
        if num_towers > 4: rstate.above = wstate
        if num_towers > 4: wstate.below = rstate


        print 'Initialized TowerEnvironment'
        
    def xy2rc(self, x, y):
        "convert x y to row col"
        return (int(round(x/GRID_DX))-1, int(round(y/GRID_DY))-1)

    def rc2xy(self, r, c):
        "convert row, col to x,y"
        return ((r+1) * GRID_DX, (c+1) * GRID_DY)
    
    def get_delay(self,ca):
        if ca == "pickup" or ca == "set":
            return 0
        elif ca == "right" or ca == "left":
            return (self.step_delay / 6.0) * (1.0 - self.speedup)
        else:
            return (self.step_delay / 2.0) * (1.0 - self.speedup)

    def get_block_state(self,name):
        for state in self.block_states:
            if self.block_states[state].name == name:
                return self.block_states[state]
        return None

    def get_top_block(self,r,c):
        curr = None
        for state in self.block_states:
            if self.block_states[state].rc == (r,c):
                curr = self.block_states[state]
                break
        if curr == None: return curr

        while curr.above != None:
            curr = curr.above

        return curr

    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            self.states[agent] = AgentState()
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
        return self.problem.rc_bounds(r+dc, c+dc)

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
        if agent.state.animation != animation:
            agent.state.animation = animation

    def step(self, agent, action):
        """
        Discrete version
        """
        if len(self.block_states) == 0:
            self.initilize_blocks()
        state = self.get_state(agent)
        state.record_action(action)
        if not self.agent_info.actions.validate(action):
            state.prev_rc = state.rc
            return 0
        if agent.step == 0:
            state.initial_position = agent.state.position
            state.initial_rotation = agent.state.rotation
            state.next_rotation = state.initial_rotation.z
        

        #print "block states:", state.holding
        #for block in self.block_states:
        #    print self.block_states[block].name , self.block_states[block].rc, self.block_states[block], self.block_states[block].above

        rot = agent.state.rotation
        rot.z = state.next_rotation
        agent.state.rotation = rot
        state.prev_rotation = state.next_rotation

        (r,c) = state.rc
        a = int(round(action[0]))
        #if len(self.action_queue) > 0:
        #    a = self.action_queue.pop(0)#int(round(action[0]))
        #    if type(a) != type(3):
        #        if len(self.action_queue) > 0:
        #            a = self.action_queue.pop(0)
        #        else:
        #            a = 0
        #else:
        #    a = 0
        (dr,dc) = (0,0)
        state.prev_rc = state.rc
        if a == 0: # null action
            state.current_action = 'stand'
            return state.record_reward(self.rewards.valid_move(state))
        if a == 1 or a == 2 or a == 3:
            (dr,dc) = TowerEnvironment.MOVES[int((agent.state.rotation.z % 360) / 90)]
            index = int(agent.state.rotation.z / 90)
            state.current_action = 'walk'
        if a == 2:
            if state.holding == None:
                state.current_action = 'stand'
                return state.record_reward(self.rewards.valid_move(state))
            (dr,dc) = TowerEnvironment.MOVES[int(agent.state.rotation.z / 90)]
            new_r, new_c = r + dr, c + dc
            curr_top = self.get_top_block(new_r,new_c)
            state.current_action = 'set'
            if curr_top != None:
             if curr_top.mass > state.holding.mass:
                curr_top.above = state.holding
                state.holding.below = curr_top
                state.holding.above = None
                state.holding.height = curr_top.height + 1
                state.holding.rc = (new_r,new_c)
                getSimContext().setObjectPosition(state.holding.obj,Vector3f((new_r + 1) * GRID_DX, (new_c + 1) * GRID_DY, (state.holding.height + 1) * GRID_DZ))
                state.holding = None
                return state.record_reward(self.rewards.valid_move(state))

             else:
                return state.record_reward(self.rewards.valid_move(state))
            else:
                state.holding.below = None
                state.holding.above = None
                state.holding.height = 0
                state.holding.rc = (new_r,new_c)
                getSimContext().setObjectPosition(state.holding.obj,Vector3f((new_r + 1) * GRID_DX, (new_c + 1) * GRID_DY, (state.holding.height + 1) * GRID_DZ))
                state.holding = None
                return state.record_reward(self.rewards.valid_move(state))
        if a == 3:
            if state.holding != None:
                state.current_action = 'stand'
                return state.record_reward(self.rewards.valid_move(state))
            state.current_action = 'pickup'
            (dr,dc) = TowerEnvironment.MOVES[int(agent.state.rotation.z / 90)]
            new_r, new_c = r + dr, c + dc
            curr_top = self.get_top_block(new_r,new_c)
            if curr_top == None:
                return state.record_reward(self.rewards.valid_move(state))
            else:
                if curr_top.below: curr_top.below.above = None
                curr_top.below = None
                curr_top.above = None
                state.holding = curr_top
                return state.record_reward(self.rewards.valid_move(state))
        if a == 4:
            state.next_rotation = agent.state.rotation.z
            state.next_rotation -= 90
            state.current_action = 'right'
        if a == 5:
            state.next_rotation = agent.state.rotation.z
            state.next_rotation += 90
            state.current_action = 'left'
        new_r, new_c = r + dr, c + dc
        state.rc = (new_r, new_c)
        (old_r,old_c) = state.prev_rc
        (old_x,old_y) = self.rc2xy(old_r, old_c)
        pos0 = agent.state.position
        pos0.x = old_x
        pos0.y = old_y
        agent.state.position = pos0
        relative_rotation = self.get_next_rotation((dr,dc))
        #agent.state.rotation = state.initial_rotation + relative_rotation
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
        (x,y) = self.rc2xy(r,c)
        pos0 = agent.state.position
        pos0.x = x
        pos0.y = y
        agent.state.position = pos0

    def sense(self, agent, obs):
        """
        Discrete version
        """
        state = self.get_state(agent)
        obs[0] = state.rc[0]
        obs[1] = state.rc[1]
        obs[2] = agent.state.rotation.z % 360
        offset = GRID_DX/10.0
        p0 = agent.state.position
        for i, (dr, dc) in enumerate(TowerEnvironment.MOVES):
            direction = Vector3f(dr, dc, 0)
            ray = (p0 + direction * offset, p0 + direction * GRID_DX)
            # we only look for objects of type 1, which means walls
            objects = getSimContext().findInRay(ray[0], ray[1], 1, True)
            obs[2 + i] = int(len(objects) > 0)
        state.record_observation(obs)
        return obs

    def is_active(self, agent):
        state = self.get_state(agent)
        # here, we interpolate between state.prev_rc and state.rc
        #if state.holding == None: state.holding = self.get_block_state("red")
        (r0,c0) = state.prev_rc
        (r1,c1) = state.rc
        dr, dc = r1 - r0, c1 - c0
        fraction = 1.0
        if self.get_delay(state.current_action) != 0:
            fraction = min(1.0,float(time.time() - state.time)/self.get_delay(state.current_action))
        if dr != 0 or dc != 0:
            (x0,y0) = self.rc2xy(r0,c0)
            (x1,y1) = self.rc2xy(r1,c1)
            pos = agent.state.position
            fraction = 1.0
            if self.get_delay(state.current_action) != 0:
                fraction = min(1.0,float(time.time() - state.time)/self.get_delay(state.current_action))
            pos.x = x0 * (1 - fraction) + x1 * fraction
            pos.y = y0 * (1 - fraction) + y1 * fraction
            agent.state.position = pos
            if state.holding == None: self.set_animation(agent, state, 'run')
            else:
                self.set_animation(agent,state,'hold_run')
                pos = getSimContext().getObjectPosition(state.holding.obj)
                pos.x = agent.state.position.x + 6 * cos(radians(agent.state.rotation.z))
                pos.y = agent.state.position.y + 6 * sin(radians(agent.state.rotation.z))
                pos.z = agent.state.position.z + 7
                getSimContext().setObjectPosition(state.holding.obj,pos)
        elif state.current_action == 'left':
                curr_rot = agent.state.rotation
                curr_rot.z = (state.prev_rotation * (1-fraction) + state.next_rotation * fraction)#%360
                agent.state.rotation = curr_rot
                if state.holding == None: self.set_animation(agent,state,'stand')
                else:
                    self.set_animation(agent,state,'hold_stand')
                    pos = getSimContext().getObjectPosition(state.holding.obj)
                    pos.x = agent.state.position.x + 6 * cos(radians(curr_rot.z))
                    pos.y = agent.state.position.y + 6 * sin(radians(curr_rot.z))
                    pos.z = agent.state.position.z + 5
                    getSimContext().setObjectRotation(state.holding.obj,curr_rot)
                    getSimContext().setObjectPosition(state.holding.obj,pos)
        elif state.current_action == 'right':
                self.set_animation(agent,state,'stand')
                curr_rot = agent.state.rotation
                curr_rot.z = (state.prev_rotation * (1-fraction) + state.next_rotation * fraction)#%360
                agent.state.rotation = curr_rot
                if state.holding == None: self.set_animation(agent,state,'stand')
                else:
                    self.set_animation(agent,state,'hold_stand')
                    pos = getSimContext().getObjectPosition(state.holding.obj)
                    pos.x = agent.state.position.x + 6 * cos(radians(agent.state.rotation.z))
                    pos.y = agent.state.position.y + 6 * sin(radians(agent.state.rotation.z))
                    pos.z = agent.state.position.z + 5
                    getSimContext().setObjectRotation(state.holding.obj,agent.state.rotation)
                    getSimContext().setObjectPosition(state.holding.obj,pos)
        else:
            if state.holding == None: self.set_animation(agent, state, 'stand')
            else: self.set_animation(agent,state,'hold_stand')
        if time.time() - state.time > self.get_delay(state.current_action):
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
