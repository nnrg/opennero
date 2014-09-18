import time
from math import *
from copy import copy
from constants import *
from OpenNero import *
from common import *

class TowerRewardStructure:
    """
    This class defines the rewards that are awarded to the agent
    """
    def valid_move(self, state):
        """
        Any valid move is -1, which makes shorter paths
        to the goal more valuable.
        """
        return -1
    def goal_reached(self, state):
        """
        A large positive reward for reaching the goal
        """
        print 'GOAL REACHED!'
        return 100

class BlockState:
    """
    State that we keep for each block
    """
    def __init__(self, problem):
        self.rc = (0,0)
        self.height = 0
        self.below = None
        self.above = None
        self.name = ''
        self.obj = None
        self.mass = 0
    def __str__(self):
        return "Block '%s' r: %d, c: %d, h: %f, mass: %s" % (self.name, self.rc[0], self.rc[1], self.height, self.mass)

class AgentState:
    """
    State that we keep for each agent
    """
    def __init__(self):
        self.rc = (0, 0)
        self.prev_rc = (0, 0)
        (x,y) = self.rc2xy(0,0)
        self.initial_position = Vector3f(x, y, 0)
        self.initial_rotation = Vector3f(0, 0, 0)
        self.inited = False
        self.time = time.time()
        self.start_time = self.time
        self.sensors = True
        self.animation = 'stand'
        self.current_action = 'stand'
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
        self.holding = None
        self.next_rotation = self.initial_rotation.z

    def update(self, agent):
        """
        Update the state of the agent
        """
        pos = copy(agent.state.position)
        self.prev_rc = self.rc
        self.rc = self.xy2rc(pos.x, pos.y)
        self.time = time.time()

class TowerEnvironment(Environment):
    """
    Tower of Hanoi environment
    """
    MOVES = [(1,0), (0,1), (-1,0), (0,-1)]

    def __init__(self):
        """
        Constructor for the tower environment
        """
        from module import getMod

        Environment.__init__(self)
        self.problem = None#TowerofHanoi.generate(ROWS, COLS, GRID_DX, GRID_DY)
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

    def initialize_blocks(self):
        from module import getMod
        num_disks = getMod().num_disks

        if num_disks >= 1:
            if not self.get_block_state('blue'):
                self.add_block("data/shapes/cube/BlueCube.xml", 0, 1, 0, 5, 'blue')
            else:
                self.set_block('blue',0,1,0,5)

        bstate = self.get_block_state('blue')

        if num_disks >= 2:
            if not self.get_block_state('green'):
                self.add_block('data/shapes/cube/GreenCube.xml', 0, 1, 1, 4, 'green')
            else:
                self.set_block('green',0,1,1,4)

        gstate = self.get_block_state('green')

        if num_disks >= 3:
            if not self.get_block_state('yellow'):
                self.add_block('data/shapes/cube/YellowCube.xml', 0, 1, 2, 3, 'yellow')
            else:
                self.set_block('yellow',0,1,2,3)

        ystate = self.get_block_state('yellow')

        if num_disks >=  4:
            if not self.get_block_state('red'):
                self.add_block('data/shapes/cube/RedCube.xml', 0, 1, 3, 2, 'red', scaler = (1.0/2.5))
            else:
                self.set_block('red',0,1,3,2)
        
        elif self.get_block_state('red'):
            self.remove_block('red')
        
        rstate = self.get_block_state('red')

        if num_disks >=  5:
            if not self.get_block_state('white'):
                self.add_block('data/shapes/cube/BlueCube.xml', 0, 1, 4, 1, 'white')
            else:
                self.set_block('white',0,1,4,1)
        
        elif self.get_block_state('white'):
            self.remove_block('white')
        
        wstate = self.get_block_state('white')
        
        bstate.above = gstate
        gstate.below = bstate
        if num_disks > 2: gstate.above = ystate
        if num_disks > 2: ystate.below = gstate
        if num_disks > 3: ystate.above = rstate
        if num_disks > 3: rstate.below = ystate
        if num_disks > 4: rstate.above = wstate
        if num_disks > 4: wstate.below = rstate

        print 'Initialized TowerEnvironment'

    def xy2rc(self, x, y):
        "convert x y to row col"
        return (int(round(x/GRID_DX))-1, int(round(y/GRID_DY))-1)

    def rc2xy(self, r, c):
        "convert row, col to x,y"
        return ((r+1) * GRID_DX, (c+1) * GRID_DY)

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

    def add_block(self, fil,x,y,z,mass,name,scaler = 1):
        block = addObject(fil,Vector3f((1 + x) * GRID_DX, (1 + y) * GRID_DY, (1 + z) * GRID_DZ), scale=Vector3f((2.0 + .1 * mass) * scaler, (2.0 + mass * .1) * scaler, .25 * 2.5 * scaler))
        self.block_states[block] = BlockState(self.problem)
        self.block_states[block].rc = (x,y)
        self.block_states[block].height = z
        self.block_states[block].name = name
        self.block_states[block].obj = block
        self.block_states[block].mass = mass
        self.block_states[block].above = None
        self.block_states[block].below = None

    def set_block(self, name, x,y,z, mass):
        state = self.get_block_state(name)
        state.rc = (x,y)
        state.height = z
        state.mass = mass
        getSimContext().setObjectPosition(state.obj,Vector3f((1 + x) * GRID_DX, (1 + y) * GRID_DY, (1 + z) * GRID_DZ))

    def remove_block(self, name):
        block = self.get_block_state(name)
        if block is not None:
            getSimContext().removeObject(block.obj)
            self.block_states.pop(block.obj)

    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            self.states[agent] = AgentState()
            assert(self.states[agent].sensors)
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
        print "RESET"
        state = self.get_state(agent)
        state.reset()

        self.initialize_blocks()
        agent.state.position = copy(state.initial_position)
        agent.state.rotation = Vector3f(0,0,0)#copy(state.initial_rotation)
        agent.reset()
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
        actions = ["Do Nothing", "Walk Forward", "Set Down", "Pick Up", "Turn Right", "Turn Left"]

        if len(self.block_states) == 0:
            self.initialize_blocks()
        state = self.get_state(agent)
        if not self.agent_info.actions.validate(action):
            state.prev_rc = state.rc
            return 0
        if state.inited == False:
            state.initial_position = agent.state.position
            state.initial_rotation = agent.state.rotation
            print "init_rot:", state.initial_rotation, type(state.initial_rotation)
            state.next_rotation = state.initial_rotation.z
            state.inited = True


        rot = agent.state.rotation

        rot.z = state.next_rotation
        agent.state.rotation = rot
        state.prev_rotation = state.next_rotation

        (r,c) = state.rc

        a = int(round(action[0]))
        
        (dr,dc) = (0,0)
        state.prev_rc = state.rc
        
        if a == 0: # null action / Jump
            state.current_action = 'jump'
            self.set_animation(agent,state,'jump')
            agent.state.animation_speed = 30.0
            return self.rewards.valid_move(state)
        
        if a == 1: # Move Forward
            if state.holding == None:
                self.set_animation(agent,state,'run')
            else:
                self.set_animation(agent,state,'hold_run');

        if a == 1 or a == 2 or a == 3: # Handles some of rotation
            (dr,dc) = TowerEnvironment.MOVES[int((agent.state.rotation.z % 360) / 90)]
            index = int(agent.state.rotation.z / 90)

        if a == 2: # Put Down
            if state.holding == None:
                state.current_action = 'stand'
                return self.rewards.valid_move(state)
            (dr,dc) = TowerEnvironment.MOVES[int(agent.state.rotation.z / 90)]
            new_r, new_c = r + dr, c + dc
            curr_top = self.get_top_block(new_r,new_c)
            state.current_action = 'set'
            self.set_animation(agent,state,'stand')
            if curr_top != None:
             if curr_top.mass > state.holding.mass:
                curr_top.above = state.holding
                state.holding.below = curr_top
                state.holding.above = None
                state.holding.height = curr_top.height + 1
                state.holding.rc = (new_r,new_c)
                getSimContext().setObjectPosition(state.holding.obj,Vector3f((new_r + 1) * GRID_DX, (new_c + 1) * GRID_DY, (state.holding.height + 1) * GRID_DZ))
                state.holding = None
                return self.rewards.valid_move(state)

             else:
                return self.rewards.valid_move(state)
            else:
                state.holding.below = None
                state.holding.above = None
                state.holding.height = 0
                state.holding.rc = (new_r,new_c)
                getSimContext().setObjectPosition(state.holding.obj,Vector3f((new_r + 1) * GRID_DX, (new_c + 1) * GRID_DY, (state.holding.height + 1) * GRID_DZ))
                state.holding = None
                return self.rewards.valid_move(state)

        if a == 3: #Pick Up
            if state.holding != None:
                state.current_action = 'stand'
                return self.rewards.valid_move(state)
            state.current_action = 'pickup'
            self.set_animation(agent,state,'hold_stand');
            (dr,dc) = TowerEnvironment.MOVES[int(agent.state.rotation.z / 90)]
            new_r, new_c = r + dr, c + dc
            curr_top = self.get_top_block(new_r,new_c)
            if curr_top == None:
                return self.rewards.valid_move(state)
            else:
                if curr_top.below: curr_top.below.above = None
                curr_top.below = None
                curr_top.above = None
                state.holding = curr_top
                pos = getSimContext().getObjectPosition(curr_top.obj)
                pos.x = agent.state.position.x + 6 * cos(radians(agent.state.rotation.z))
                pos.y = agent.state.position.y + 6 * sin(radians(agent.state.rotation.z))
                pos.z = agent.state.position.z + 7
                getSimContext().setObjectPosition(state.holding.obj,pos)
                return self.rewards.valid_move(state)

        if a == 4: #Turn Right
            state.next_rotation = agent.state.rotation.z
            state.next_rotation -= 90
            state.current_action = 'right'
            if state.holding == None:
                self.set_animation(agent,state,'turn_r_lx')
            else:
                self.set_animation(agent,state,'hold_turn_r_lx');
            rot = agent.state.rotation
            rot.z = state.next_rotation
            agent.state.rotation = rot
            agent.state.rotation = rot

        if a == 5: #Turn Left
            state.next_rotation = agent.state.rotation.z
            state.next_rotation += 90
            state.current_action = 'left'
            if state.holding == None:
                self.set_animation(agent,state,'turn_l_lx')
            else:
                self.set_animation(agent,state,'hold_turn_l_lx');
            rot = agent.state.rotation
            rot.z = state.next_rotation
            agent.state.rotation = rot
            agent.state.rotation = rot

        new_r, new_c = r + dr, c + dc
        state.rc = (new_r, new_c)
        (old_r,old_c) = state.prev_rc
        (old_x,old_y) = self.rc2xy(old_r, old_c)
        pos0 = agent.state.position
        pos0.x = old_x
        pos0.y = old_y
        agent.state.position = pos0
        relative_rotation = self.get_next_rotation((dr,dc))
        rot = agent.state.rotation
        rot.z = state.next_rotation
        agent.state.rotation = rot
        pos = agent.state.position
        (pos.x,pos.y) = self.rc2xy(new_r,new_c)
        agent.state.position = pos

        if state.holding != None:
                pos = getSimContext().getObjectPosition(state.holding.obj)
                pos.x = agent.state.position.x + 6 * cos(radians(agent.state.rotation.z))
                pos.y = agent.state.position.y + 6 * sin(radians(agent.state.rotation.z))
                pos.z = agent.state.position.z + 7
                getSimContext().setObjectPosition(state.holding.obj,pos)
        
        if new_r == ROWS - 1 and new_c == COLS - 1:
            state.goal_reached = True
            return self.rewards.goal_reached(state)
        elif agent.step >= self.max_steps - 1:
            return self.rewards.last_reward(state)
        return self.rewards.valid_move(state)

    def sense(self, agent, obs):
        state = self.get_state(agent)
        obs[0] = state.rc[0]
        obs[1] = state.rc[1]
        obs[2] = agent.state.rotation.z % 360
        return obs

    def is_episode_over(self, agent):
         return False

    def cleanup(self):
        print "CLEANUP"
        for key in self.block_states:
            block = self.block_states[key]
            getSimContext().removeObject(block.obj)
        return True
