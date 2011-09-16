# This is the overall script for the NL advice experiment.
# It does the following:
#  * generate M random numbers that will act as seeds for the maps
#  * exhaustively generate all possible N-condition sequences (divided into groups of 3)
#  * assign maps to conditions such that each map is used equally frequently in each condition
# To use this script

import random
import cPickle as pickle

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"
__date__ = "2008/03/26"

ROWS = 8 # rows in the maze
COLS = 8 # columns in the maze
N = 9 # number of conditions to use

# a Session is a list of the form:
# ['u','a','r',...] where
#   'u' - unassisted maze running condition
#   'a' - subject 1 assisting subject 2 (assisting)
#   'r' - subject 2 assisting subject 1 (running)

# Maps is a list of numbers (random seeds) used to generate the mazes

class Group:
    """ represents information about a single group's run """
    def __init__(self, session, maps):
        self.session = session
        self.maps = maps
        self.condition = 0

    def next(self):
        if self.condition < len(self.session):
            print 'Starting condition %d' % self.condition
            print 'Using seed %d' % self.maps[self.condition]
            from mazer import Maze
            print Maze.generate(ROWS, COLS, 20, 20)
        else:
            raise StopIteration

# all possible groups of 3
# uar, aur, aru, ura, rua, rau
# since subjects are symmetric, we can just use three of these
# uar, aur, aru
# and we will get the other three for the other person
template = ['u','a','r','u','a','r','u','a','r']
segments = [ [0, 1, 2], [1, 0, 2], [1, 2, 0] ]
assignments = []
for i in range(N):
    assignment = range(N)
    assignment = assignment[N-i:] + assignment[:N-i]
    assignments.append(assignment)

# now we generate N groups by drawing N/3 segments randomly
sessions = []
for i in range(N/3):
    for j in range(N/3):
        index = segments[0] + [x + 3 for x in segments[i]] + [x + 6 for x in segments[j]]
        session = [(template[k], assignments[3 * i + j][k]) for k in index]
        sessions.append(session)

#counts = {}
#for c in ['u','a','r']:
#    for m in range(N):
#        counts[(c,m)] = 0
#for session in sessions:
#    for c, m in session:
#        if c == 'u':
#            counts[(c,m)] += 2
#        else:
#            counts[('a',m)] += 1
#            counts[('r',m)] += 1

f = open('runs.pickle','w')
pickle.dump(sessions,f)
f.close()

from pprint import pprint
for session in sessions:
    print(session)
