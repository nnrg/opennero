from OpenNero import *
from common import *
import Maze
from Maze.environment import MazeEnvironment, ContMazeEnvironment
from Maze.constants import *

from heapq import heappush, heappop

def dfs_heuristic(r,c):
    return 0

def manhattan_heuristic(r,c):
    return (ROWS - r) + (COLS - c)

def get_action_index(move):
    if move in MazeEnvironment.MOVES:
        action = MazeEnvironment.MOVES.index(move)
        print 'Picking action', action, 'for move', move
        return action
    else:
        return None

class Cell:
    def __init__(self, h, r, c):
        self.h = h
        self.r = r
        self.c = c
    def __cmp__(self, other):
        return cmp(self.h, other.h)

class RandomAgent(AgentBrain):
    """
    A uniform random baseline - a very simple agent
    """
    def __init__(self):
        AgentBrain.__init__(self) # have to make this call

    def initialize(self, init_info):
        """
        create a new agent
        """
        self.action_info = init_info.actions
        return True

    def start(self, time, observations):
        """
        return first action given the first observations
        """
        return self.action_info.random()

    def reset(self):
        pass

    def act(self, time, observations, reward):
        """
        return an action given the reward for the previous action and the new observations
        """
        return self.action_info.random()

    def end(self, time, reward):
        """
        receive the reward for the last observation
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        return True

class SearchAgent(AgentBrain):
    """ Base class for maze search agents """

    def __init__(self):
        """ constructor """
        AgentBrain.__init__(self)
    def highlight_path(self):
        """
        backtrack to highlight the path
        """
        # retrace the path
        node = (ROWS - 1, COLS - 1)
        while node in self.backpointers:
            self.mark_path(node[0],node[1])
            next_node = self.backpointers[node]
            del self.backpointers[node]
            node = next_node
            if node == (0,0):
                break

class DFSSearchAgent(SearchAgent):
    """
    Depth first search implementation
    """
    def __init__(self):
        """
        A new Agent
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        SearchAgent.__init__(self)
        self.visited = set([])
        self.adjlist = {}
        self.parents = {}
        self.backpointers = {}

    def dfs_action(self, observations):
        r = observations[0]
        c = observations[1]
        # if we have not been here before, build a list of other places we can go
        if (r,c) not in self.visited:
            tovisit = []
            for m, (dr,dc) in enumerate(MazeEnvironment.MOVES):
                r2, c2 = r+dr, c+dc
                if not observations[2 + m]: # can we go that way?
                    if (r2,c2) not in self.visited:
                        tovisit.append( (r2,c2) )
                        self.parents[ (r2,c2) ] = (r,c)
            # remember the cells that are adjacent to this one
            self.adjlist[(r, c)] = tovisit
        # if we have been here before, check if we have other places to visit
        adjlist = self.adjlist[(r,c)]
        k = 0
        while k < len(adjlist) and adjlist[k] in self.visited:
            k += 1
        # if we don't have other neighbors to visit, back up
        if k == len(adjlist):
            current = self.parents[(r,c)]
        else: # otherwise visit the next place
            current = adjlist[k]
        self.visited.add((r,c)) # add this location to visited list
        get_environment().mark_maze_blue(r, c) # mark it as blue on the maze
        v = self.constraints.get_instance() # make the action vector to return
        dr, dc = current[0] - r, current[1] - c # the move we want to make
        v[0] = get_action_index( (dr, dc) )
        # remember how to get back
        if (r+dr,c+dc) not in self.backpointers:
            self.backpointers[(r+dr, c+dc)] = (r,c)
        return v

    def initialize(self, init_info):
        self.constraints = init_info.actions
        return True

    def start(self, time, observations):
        # return action
        return self.dfs_action(observations)

    def reset(self):
        self.visited = set([])
        self.parents = {}
        self.backpointers = {}

    def act(self, time, observations, reward):
        # return action
        return self.dfs_action(observations)

    def end(self, time, reward):
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        self.reset()
        return True

    def mark_path(self, r, c):
        get_environment().mark_maze_white(r,c)

class GenericSearchAlgorithm(SearchAgent):
    """
    Generic search algorithm with retrace and a heuristic function
    """
    def __init__(self):
        """
        constructor
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        SearchAgent.__init__(self)
        self.backpointers = {}
        self.reset()

    def reset(self):
        """
        Reset the agent
        """
        self.parents = {}
        self.queue = [] # queue of cells to visit (front)
        self.visited = set([]) # set of nodes we have visited
        self.enqueued = set([]) # set of things in the queue (superset of visited)
        self.backpointers = {} # a dictionary from nodes to their predecessors
        self.goal = None # we have no idea where to go at first

    def initialize(self, init_info):
        """
        initialize the agent with sensor and action info
        """
        self.constraints = init_info.actions
        return True

    def get_next_step(self, r1, c1, r2, c2):
        """
        return the next step when trying to get from current r1,c1 to target r2,c2
        """
        back2 = [] # cells between origin and r2,c2
        r,c = r2,c2 # back track from target
        while (r,c) in self.backpointers:
            r,c = self.backpointers[(r,c)]
            if (r1,c1) == (r,c): # if we find starting point, we need to move forward
                return back2[-1] # return the last step
            back2.append((r,c))
        return self.backpointers[(r1,c1)]

    def enque(self, cell ):
        self.queue.append( cell )

    def deque(self):
        return self.queue.pop(0)

    def get_action(self, r, c, observations):
        """
        Given:
         - some places we could go in our queue
         - some backtracking links on how we got there
        Return:
         - The next step for the agent to take
        """
        if not self.goal: # first, figure out where we are trying to go
            (r2, c2) = self.deque()
            self.mark_target(r2,c2)
            self.goal = (r2, c2)
        # then, check if we can get there
        r2, c2 = self.goal
        dr, dc = r2 - r, c2 - c
        action = get_action_index((dr,dc))
        v = self.constraints.get_instance() # make the action vector to return
        # first, is the node reachable in one action?
        if action is not None and observations[2 + action] == 0:
            v[0] = action # if yes, do that action!
        else:
            # if not, we have to figure out a path to get there from the backtracking dictionary
            (r2,c2) = self.get_next_step(r,c,r2,c2)
            dr, dc = r2 - r, c2 - c # how to get back there?
            v[0] = get_action_index((dr,dc)) # what action is that?
        return v # return the action

    def visit(self, row, col, observations):
        """
        visit the node row, col and decide where we can go from there
        """
        if (row,col) not in self.visited:
            self.mark_visited(row,col)
        # we are at row, col, so we mark it visited:
        self.visited.add((row,col))
        self.enqueued.add((row,col)) # just in case
        # if we have reached our current subgoal, mark it visited
        if self.goal == (row, col):
            print  'reached goal: ' + str((row, col))
            self.goal = None
        # then we queue up some places to go next
        for i, (dr,dc) in enumerate(MazeEnvironment.MOVES):
            if observations[2+i] == 0: # are we free to perform this action?
                # the action index should correspond to sensor index - 2
                r2 = row + dr # compute the row we could move to
                c2 = col + dc # compute the col we could move to
                if (r2,c2) not in self.enqueued:
                    self.mark_the_front(row,col,r2,c2)
                    self.enqueued.add((r2,c2))
                    self.enque( (r2,c2) )
                    assert self.backpointers.get((row,col)) != (r2,c2)
                    self.backpointers[(r2,c2)] = row, col # remember where we (would) come from

    def start(self, time, observations):
        """
        Choose initial action after receiving the first sensor vector.
        For the manual A* search, we enqueue the neighboring nodes and move to one of them.
        """
        # interpret the observations
        row = int(observations[0])
        col = int(observations[1])
        # first, visit the node we are in and queue up some places to go
        self.visit(row, col, observations)
        # now we have some candidate states and a way to return if we don't like it there, so let's try one!
        return self.get_action(row,col,observations)

    def act(self, time, observations, reward):
        """
        Choose an action after receiving the current sensor vector and the instantaneous reward from the previous time step.
        For the manual A* search, we deque our next node and check if we can go there. If we can, we do, and mark the node visited.
        If we cannot, we have to follow the path to the goal.
        """
        # interpret the observations
        row = int(observations[0])
        col = int(observations[1])
        # first, visit the node we are in and queue up some places to go
        self.visit(row, col, observations)
        # now we have some candidate states and a way to return if we don't like it there, so let's try one!
        return self.get_action(row,col,observations)

    def end(self, time, reward):
        """
        at the end of an episode, the environment tells us the final reward
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        self.reset()
        return True

    def mark_the_front(self, r, c, r2, c2):
        get_environment().mark_maze_green(r2,c2)

    def mark_target(self, r, c):
        get_environment().mark_maze_yellow(r,c)

    def mark_visited(self, r, c):
        get_environment().mark_maze_blue(r,c)

    def mark_path(self, r, c):
        get_environment().mark_maze_white(r,c)

class BFSSearchAgent(GenericSearchAlgorithm):
    """
    Egocentric Breadth First Search algorithm.
    The only change is that we use a simple queue instead of a priority queue
    """
    def __init__(self):
        """
        A new Agent
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        GenericSearchAlgorithm.__init__(self)

    def enque(self, cell ):
        self.queue.append(cell)

    def deque(self):
        return self.queue.pop(0)

    def get_action(self, r, c, observations):
        """
        we override the get_action method so that we can spawn marker agents and teleport
        """
        if not self.goal: # first, figure out where we are trying to go
            (r2,c2) = self.deque()
            get_environment().unmark_maze_agent(r2,c2)
            get_environment().mark_maze_yellow(r2,c2)
            self.goal = (r2, c2)
        # then, check if we can get there
        r2, c2 = self.goal
        dr, dc = r2 - r, c2 - c
        action = get_action_index((dr,dc)) # try to find the action (will return None if it's not there)
        v = self.constraints.get_instance() # make the action vector to return
        # first, is the node reachable in one action?
        if action is not None and observations[2 + action] == 0:
            v[0] = action # if yes, do that action!
        else:
            # if not, we should teleport and return null action
            get_environment().teleport(self, r2, c2)
            v[0] = 4
        return v # return the action

    def mark_the_front(self, r, c, r2, c2):
        get_environment().mark_maze_green(r2,c2)
        get_environment().mark_maze_agent("data/shapes/character/SydneyStatic.xml", r, c, r2, c2)

    def mark_visited(self, r, c):
        get_environment().unmark_maze_agent(r,c)
        get_environment().mark_maze_blue(r,c)

class AStarSearchAgent(GenericSearchAlgorithm):
    """
    Egocentric A* algorithm - actually it is just like BFS but the queue is a priority queue
    """
    def __init__(self):
        """
        A new Agent
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        GenericSearchAlgorithm.__init__(self)

    def reset(self):
        GenericSearchAlgorithm.reset(self)
        # minimize the Manhattan distance
        self.heuristic = manhattan_heuristic

    def enque(self, cell ):
        (r,c) = cell
        heappush(self.queue, Cell(self.heuristic(r, c), r, c))

    def deque(self):
        cell = heappop(self.queue)
        h, r, c = cell.h, cell.r, cell.c
        return (r,c)

class FrontAStarSearchAgent(AStarSearchAgent):
    """
    Egocentric A* algorithm with teleportation between fronts
    """
    def get_action(self, r, c, observations):
        """
        we override the get_action method so that we can teleport from place to place
        """
        if not self.goal: # first, figure out where we are trying to go
            (r2, c2) = self.deque()
            get_environment().unmark_maze_agent(r2,c2)
            get_environment().mark_maze_yellow(r2,c2)
            self.goal = (r2, c2)
        # then, check if we can get there
        r2, c2 = self.goal
        dr, dc = r2 - r, c2 - c
        action = get_action_index((dr,dc)) # try to find the action (will return None if it's not there)
        v = self.constraints.get_instance() # make the action vector to return
        # first, is the node reachable in one action?
        if action is not None and observations[2 + action] == 0:
            v[0] = action # if yes, do that action!
        else:
            # if not, we should teleport and return null action
            get_environment().teleport(self, r2, c2)
            v[0] = MazeEnvironment.NULL_MOVE
        return v # return the action

class CloningAStarSearchAgent(FrontAStarSearchAgent):
    """
    Egocentric A* algorithm with teleportation between fronts and fronts marked by stationary agents
    """
    def get_action(self, r, c, observations):
        """
        we override the get_action method so that we can spawn marker agents and teleport
        """
        if not self.goal: # first, figure out where we are trying to go
            (r2,c2) = self.deque()
            get_environment().unmark_maze_agent(r2,c2)
            get_environment().mark_maze_yellow(r2,c2)
            self.goal = (r2, c2)
        # then, check if we can get there
        r2, c2 = self.goal
        dr, dc = r2 - r, c2 - c
        action = get_action_index((dr,dc)) # try to find the action (will return None if it's not there)
        v = self.constraints.get_instance() # make the action vector to return
        # first, is the node reachable in one action?
        if action is not None and observations[2 + action] == 0:
            v[0] = action # if yes, do that action!
        else:
            # if not, we should teleport and return null action
            get_environment().teleport(self, r2, c2)
            v[0] = 4
        return v # return the action

    def mark_the_front(self, r, c, r2, c2):
        get_environment().mark_maze_green(r2,c2)
        get_environment().mark_maze_agent("data/shapes/character/SydneyStatic.xml", r, c, r2, c2)

    def mark_visited(self, r, c):
        get_environment().unmark_maze_agent(r,c)
        get_environment().mark_maze_blue(r,c)

class FirstPersonAgent(AgentBrain):
    """
    A human-controlled agent
    """
    key_pressed = None
    action_map = ContMazeEnvironment.ACTIONS
    def __init__(self):
        AgentBrain.__init__(self) # do not remove!
        self.time = 0
    def initialize(self, init_info):
        self.constraints = init_info.actions
        return True
    def key_action(self):
        self.time += 1
        action = self.constraints.get_instance()
        if FirstPersonAgent.key_pressed:
            action[0] = FirstPersonAgent.action_map[FirstPersonAgent.key_pressed]
            FirstPersonAgent.key_pressed = None
        else:
            action[0] = MazeEnvironment.NULL_MOVE
        return action
    def start(self, time, observations):
        return self.key_action()
    def act(self, time, observations, reward):
        return self.key_action()
    def end(self, time, reward):
        return True

class MoveForwardAndStopAgent(AgentBrain):
    """
    Just move forward and stop!
    """
    def __init__(self):
        AgentBrain.__init__(self) # do not remove!
    def initialize(self, init_info):
        self.actions = init_info.actions # action constraints
        self.idle_action = self.actions.get_instance()
        self.idle_action[0] = MazeEnvironment.NULL_MOVE # do-nothing action
        return True
    def start(self, time, observations):
        marker_states = get_environment().marker_states
        if self.state.id in marker_states:
            ((r1,c1), (r2,c2)) = marker_states[self.state.id]
            v = self.actions.get_instance()
            dr, dc = r2-r1, c2-c1
            a = get_action_index( (dr, dc) )
            if a is not None:
                # return the action in the (dr,dc) direction
                v[0] = a
                return v
        return self.idle_action
    def act(self, time, observations, reward):
        return self.idle_action
    def end(self, time, reward):
        return True

class RTNEATAgent(AgentBrain):
    """
    rtNEAT agent
    """
    def __init__(self):
        """
        Create an agent brain
        """
        # this line is crucial, otherwise the class is not recognized as an AgentBrainPtr by C++
        AgentBrain.__init__(self)

    def initialize(self, init_info):
        """
        Initialize an agent brain with sensor information
        """
        self.actions = init_info.actions # constraints for actions
        self.observations = init_info.observations # constraints for observations
        return True

    def start(self, time, observations):
        """
        start of an episode
        """
        return self.network_action(observations)

    def act(self, time, observations, reward):
        """
        a state transition
        """
        # return action
        return self.network_action(observations)

    def end(self, time, reward):
        """
        end of an episode
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        get_ai("rtneat").release_organism(self)
        return True

    def network_action(self, observations):
        """
        Take the current network
        Feed the observations into it
        Activate the network to produce the output
        Collect and interpret the outputs as valid maze actions
        """
        assert(self.observations.validate(observations))
        inputs = Maze.module.input_to_neurons(self.observations, observations)
        org = get_ai("rtneat").get_organism(self)
        org.time_alive += 1
        net = org.net
        net.load_sensors(inputs)
        net.activate()
        outputs = net.get_outputs()
        actions = Maze.module.neurons_to_output(self.actions, outputs)
        assert(self.actions.validate(actions))
        return actions
