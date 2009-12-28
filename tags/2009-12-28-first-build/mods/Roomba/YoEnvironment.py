import time
from math import *
from copy import copy
from OpenNero import *
from SearchEnvironment import *
from FPSAgent import FPSAgent
from RTNEATAgent import RTNEATAgent
from driver import connect, send, PORT

class YoMazeEnvironment(MazeEnvironment):
    SPEED = 2.5 # per-step translational speed
    TIME_PER_STEP = 0.01 # min time between steps in seconds
    STEPS_PER_EPISODE = 100 # max number of steps per episode

    """
    manual controller environment for the 2-D maze
    """
    def __init__(self, time_per_step = TIME_PER_STEP, max_steps = STEPS_PER_EPISODE):
        """
        Create the environment
        """
        MazeEnvironment.__init__(self) 
        
        self.step_size = time_per_step
        self.max_steps = max_steps
        
        previous_action_template = self.agent_info.actions # add more actions for FPS control
        
        # copy old actions
        new_action_template = FeatureVectorInfo()
        for i in range(len(previous_action_template)):
            new_action_template.add(previous_action_template.bound(i))
        # remember where first person actions start
        self.ACTION_OFFSET = len(new_action_template)
        # add first person actions
        for i in range(len(FPSAgent.action_map)):
            new_action_template.add_discrete(0, 1)
        # make a new agent info
        self.new_agent_info = AgentInitInfo(self.agent_info.sensors, new_action_template, self.agent_info.reward)
        self.connection = connect('127.0.0.1', PORT)
        self.seed = int(get_app_config().seeds.split()[0])
        print 'seed: %d' % self.seed
        if self.connection:
            send(self.connection, ('seed', self.seed))
        random.seed(self.seed)
        self.walls = maze(ROWS, COLS)
    
    def get_agent_info(self, agent):
        """
        return a blueprint for a new agent
        """
        return self.new_agent_info
    
    def world_to_grid(self, position, pose, actual = False):
        """ 
        convert a world coordinate to a 2-D grid world coordinate
        If actual is False (default), the function errs on the safe side of the current cell
        """
        # this is the unadjusted cell position
        r, c = int(round(position.x / GRID_DX - 1)), int(round(position.y / GRID_DY - 1))
        if actual:
            return (r, c)
        # this is how much we are moving (including direction)
        move_dx, move_dy = position.x - pose[0].x, position.y - pose[0].y
        # this is how far we could possibly move
        if move_dx != 0:
            move_dx = YoMazeEnvironment.SPEED * move_dx / abs(move_dx)
        if move_dy != 0:
            move_dy = YoMazeEnvironment.SPEED * move_dy / abs(move_dy)
        # make the phantom move to make sure we don't see through walls
        (r2, c2) = self.world_to_grid(position + Vector3f(move_dx, move_dy, 0), pose, True)
        return r2, c2
    
    def grid_to_world(self, cell):
        return (cell[0] * GRID_DX + GRID_DX, cell[1] * GRID_DY + GRID_DY)
    
    def step(self, agent, action):
        """
        A step for an agent
        """
        state = self.get_state(agent)
        # check if the action is valid
        if state.current_step == 0:
            state.initial_position = agent.sim.position
            state.initial_rotation = agent.sim.rotation
            state.pose = (copy(state.initial_position), copy(state.initial_rotation))
        state.current_step += 1
        (r, c) = self.world_to_grid(agent.sim.position, state.pose, True)
        if r == ROWS - 1 and c == COLS - 1:
            self.reset(agent)
            return MazeEnvironment.REWARD_VALID_MOVE

        position = agent.sim.position
        rotation = agent.sim.rotation
        state.pose = (copy(position), copy(rotation))
        heading = radians(rotation.z)

        if action[FPSAgent.action_map['fwd']] > 0:
            position.x += YoMazeEnvironment.SPEED
            rotation.z = 0
        if action[FPSAgent.action_map['back']] > 0:
            position.x -= YoMazeEnvironment.SPEED
            rotation.z = 180
        if action[FPSAgent.action_map['left']] > 0:
            position.y += YoMazeEnvironment.SPEED
            rotation.z = 90
        if action[FPSAgent.action_map['right']] > 0:
            position.y -= YoMazeEnvironment.SPEED
            rotation.z = 270

        (new_r, new_c) = self.world_to_grid(position, state.pose)
        if not self.in_bounds((new_r, new_c)):
            print "out of bounds: %d, %d" % (new_r, new_c)
            return MazeEnvironment.REWARD_OUT_OF_BOUNDS
        else:
            if self.is_wall((r, c), (new_r, new_c)):
                print "hit wall: %d, %d" % (new_r, new_c)
                return MazeEnvironment.REWARD_HIT_WALL

        # actually apply the move to the body using the true value
        (new_r, new_c) = self.world_to_grid(position, state.pose, True)
        if sum(action) > 0:
            # log the timestamped pose and send update to observer
            # print "%s %s" % (str(position), str(degrees(heading)))
            self.update_observer((position.x, position.y, radians(rotation.z)))
        state.position = (new_r, new_c)
        state.prev_position = (r, c)
        agent.sim.position = position
        agent.sim.rotation = rotation
        if new_r == ROWS - 1 and new_c == COLS - 1:
            state.goal_reached = True
            print "CONGRATULATIONS! Goal reached! Restarting the maze."
            return MazeEnvironment.REWARD_GOAL_REACHED
        elif self.max_steps != 0 and state.current_step >= self.max_steps:
            # if we run out of time, return negative distance left
            return -(ROWS - new_r + COLS - new_c)
        # by default, an action costs 1
        return MazeEnvironment.REWARD_VALID_MOVE
    
    def cleanup(self):
        """
        This is the way the World ends
        """
        if self.connection:
            self.connection.close()
        return True
    
    def update_observer(self, pose):
        if self.connection:
            send(self.connection, pose)
