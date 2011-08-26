from OpenNero import *
from common import *
import Tower
import random
from Tower.environment import TowerEnvironment
from Tower.constants import *

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

class TowerAgent(AgentBrain):
    """
    An agent designed to solve Towers of Hanoi
    """
    def __init__(self):
        AgentBrain.__init__(self) # have to make this call
        self.init_queue = [1,5]
        self.atob = [5,1,4,3,4,1,5,2,]
        self.btoa = [3,5,1,4,2,4,1,5,]
        self.atoc = [5,1,4,3,4,1,1,5,2,5,1,4,]
        self.ctoa = [4,1,5,3,5,1,1,4,2,4,1,5,]
        self.btoc = [3,4,1,5,2,5,1,4,]
        self.ctob = [4,1,5,3,5,1,4,2,]
        self.end_queue = [0,0,0,5,5,1]

        from module import getMod
        num_towers = getMod().num_towers

        if num_towers == 3:
            self.action_queue = self.init_queue
            self.action_queue += (self.atoc)
            self.action_queue += (self.atob)
            self.action_queue += (self.ctob)
            self.action_queue += (self.atoc)
            self.action_queue += (self.btoa)
            self.action_queue += (self.btoc)
            self.action_queue += (self.atoc)
            self.action_queue += (self.end_queue)

        if num_towers == 4:
            self.action_queue = self.init_queue #0
            self.action_queue += (self.atob) #1
            self.action_queue += (self.atoc) #2
            self.action_queue += (self.btoc) #3
            self.action_queue += (self.atob) #4
            self.action_queue += (self.ctoa) #5
            self.action_queue += (self.ctob) #6
            self.action_queue += (self.atob) #7
            self.action_queue += (self.atoc) #8
            self.action_queue += (self.btoc) #9
            self.action_queue += (self.btoa) #10
            self.action_queue += (self.ctoa) #11
            self.action_queue += (self.btoc) #12
            self.action_queue += (self.atob) #13
            self.action_queue += (self.atoc) #14
            self.action_queue += (self.btoc) #15
            self.action_queue += (self.end_queue)

    	if num_towers == 5:
            self.action_queue = self.init_queue #0
            self.action_queue += (self.atoc)
            self.action_queue += (self.atob)
            self.action_queue += (self.ctob)
            self.action_queue += (self.atoc)
            self.action_queue += (self.btoa)
            self.action_queue += (self.btoc)
            self.action_queue += (self.atoc)
            self.action_queue += (self.atob)
            self.action_queue += (self.ctob)
            self.action_queue += (self.ctoa)
            self.action_queue += (self.btoa)
            self.action_queue += (self.ctob)
            self.action_queue += (self.atoc)
            self.action_queue += (self.atob)
            self.action_queue += (self.ctob)
            self.action_queue += (self.atoc)
            self.action_queue += (self.btoa)
            self.action_queue += (self.btoc)
            self.action_queue += (self.atoc)
            self.action_queue += (self.btoa)
            self.action_queue += (self.ctob)
            self.action_queue += (self.ctoa)
            self.action_queue += (self.btoa)
            self.action_queue += (self.btoc)
            self.action_queue += (self.atoc)
            self.action_queue += (self.atob)
            self.action_queue += (self.ctob)
            self.action_queue += (self.atoc)
            self.action_queue += (self.btoa)
            self.action_queue += (self.btoc)
            self.action_queue += (self.atoc)
            self.action_queue += (self.end_queue)

    def initialize(self,init_info):
        # Create new Agent
        self.action_info = init_info.actions
        return True
    
    def start(self, time, sensors):
        """
        return first action given the first observations
        """
        return self.action_queue.pop(0)

    def reset(self):
        pass

    def face_at(self,x,y,z,nx,ny):
        if(x - nx) == -1:
            return self.face(z,270)
        if(x - nx) == 1:
            return self.face(z,90)
        if(y - ny) == 1:
            return self.face(z,0)
        if(y - ny) == -1:
            return self.face(z,180)

    def face(self,z,nz):
        if ((z - nz) % 360) == 180:
            self.action_queue += [4,4]
        if ((z - nz) % 360) == 90:
            self.action_queue += [5]
        if ((z - nz) % 360) == 270:
            self.action_queue += [4]
        return nz

    def goto_1(self,x,y,z,nx,ny):
        if (x - nx) * (x - nx) + (y - ny) * (y - ny) != 1: return (x,y,z)
        nz = self.face_at(x,y,z,nx,ny)
        self.action_queue += [1]
        return (nx,ny,nz)

    def pickup(self):
        self.action_queue += [3]

    def setdown(self):
        self.action_queue += [2]
    
    def act(self, time, sensors, reward):
        """
        return an action given the reward for the previous action and the new observations
        """
        x = sensors[0]
        y = sensors[1]
        rot = sensors[2]
        if len(self.action_queue) == 0:
            self.action_queue.append(1)
            """
            rand = random.randint(0,3)
            if rand == 0:
                self.goto_1(x,y,rot,x+1,y)
            if rand == 1:
                self.goto_1(x,y,rot,x-1,y)
            if rand == 2:
                self.goto_1(x,y,rot,x,y+1)
            if rand == 3:
                self.goto_1(x,y,rot,x,y-1)
            """
        return self.action_queue.pop(0)

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
