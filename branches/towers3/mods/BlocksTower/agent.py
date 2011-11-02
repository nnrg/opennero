from OpenNero import *
from common import *
import BlocksTower
import random
from BlocksTower.environment import TowerEnvironment
from BlocksTower.constants import *

def get_action_index(move):
    if move in TowerEnvironment.MOVES:
        action = TowerEnvironment.MOVES.index(move)
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

###
#
# Action definitions:
# 0 Jump
# 1 Move Forward
# 2 Put Down
# 3 Pick Up
# 4 Rotate Right
# 5 Rotate Left
#
###

class TowerAgent(AgentBrain):
    """
    An agent designed to solve Towers of Hanoi
    """
    def __init__(self):
        AgentBrain.__init__(self) # have to make this call
        
    def log(self, strn):
        self.state.label = strn
        print strn

    def move(self, frm, to):
        if frm == 'a' and to == 'b': return self.atob
        if frm == 'a' and to == 'c': return self.atoc
        if frm == 'b' and to == 'a': return self.btoa
        if frm == 'b' and to == 'c': return self.btoc
        if frm == 'c' and to == 'a': return self.ctoa
        if frm == 'c' and to == 'b': return self.ctob

    def dohanoi(self, n, to, frm, using):
        if n == 0: return
        prefix = '\t'.join(['' for i in range(self.num_towers - n)])
        strn = "Moving depth {n} from {frm} to {to} using {using}".format(n=n, frm=frm, to=to, using=using)
        for a in self.dohanoi(n-1, using, frm, to):
            yield a
        self.state.label = strn
        for a in self.move(frm, to):
            yield a
        for a in self.dohanoi(n-1, to, using, frm):
            yield a

    def queue_init(self):
        self.init_queue = [1,5]
        self.atob = [5,1,4,3,4,1,5,2,]
        self.btoa = [3,5,1,4,2,4,1,5,]
        self.atoc = [5,1,4,3,4,1,1,5,2,5,1,4,]
        self.ctoa = [4,1,5,3,5,1,1,4,2,4,1,5,]
        self.btoc = [3,4,1,5,2,5,1,4,]
        self.ctob = [4,1,5,3,5,1,4,2,]
        self.end_queue = [0,0,0,5,5,1]

        from module import getMod
        self.num_towers = getMod().num_towers

        self.state.label = 'Starting to Solve!'
        for a in self.init_queue:
            yield a
        for a in self.dohanoi(self.num_towers, 'b', 'a', 'c'):
            yield a
        self.state.label = 'Problem Solved!'
        for a in self.end_queue:
            yield a

    def initialize(self,init_info):
        # Create new Agent
        self.action_info = init_info.actions
        return True

    def start(self, time, sensors):
        """
        return first action given the first observations
        """
        self.action_queue = self.queue_init()
        return self.action_queue.next()

    def reset(self):
        self.action_queue = self.queue_init()

    def act(self, time, sensors, reward):
        """
        return an action given the reward for the previous action and the new observations
        """
        try:
            return self.action_queue.next()
        except:
            return 1

    def end(self, time, reward):
        """
        receive the reward for the last observation
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        return True

    def destroy(self):
        """
        called when the agent is done
        """
        return True
