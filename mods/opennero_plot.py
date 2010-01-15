#!/usr/bin/env python
"""Process and display performance of AI algorithms in OpenNERO.

opennero_plot reads a log file (or receives this file over the network) and 
plots the performance of the AI algorithm which is producing this log file.
"""

import sys
import re
import time
import numpy as np
import matplotlib.pyplot as pl
import matplotlib.mlab as mlab
from threading import Thread

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"
__copyright__ = "Copyright 2010, The University of Texas at Austin"
__license__ = "LGPL"
__version__ = "0.1.0"

ai_tick_pattern = re.compile(r'(?P<date>[^\[]*)\.(?P<msec>[0-9]+) \(m\) \[ai\.tick\]\s+(?P<episode>\S+)\s+(?P<step>\S+)\s+(?P<reward>\S+)\s+(?P<fitness>\S+)')
timestamp_format = r'%Y-%b-%d %H:%M:%S'

class LearningCurve:
    def __init__(self):
        self.min_time = None
        self.max_time = None
        self.data = []
        self.episodes = []

    def reset(self):
        self.min_time = None
        self.max_time = None
        self.data = []
        self.episodes = []
        pl.figure()
        pl.hold(True)
        pl.xlabel('step')
        pl.ylabel('fitness')
        pl.title('Episodes')
        pl.grid(True)

    def plot_episode(self):
        M = np.array(self.data)
        steps = M[:,-3]
        fitness = M[:,-1]
        pl.plot(steps, fitness, linewidth=1.0)

    def append(self, t, msec, episode, step, reward, fitness):
        #only remember the most recent set of episodes
        if episode == 0 and step == 0:
            self.reset()
        elif step == 0:
            # on episode start, append the previous episode's fitness
            self.episodes.append(self.data[-1][-1])
            self.plot_episode()
            self.data = []
        sec = time.mktime(t) * 1000000
        sec += msec
        if not self.min_time:
            self.min_time = sec
        self.max_time = sec
        #print sec, episode, step, reward, fitness
        self.data.append( (time, msec, episode, step, reward, fitness) )

def process_log(f):
    """Process AI ticks in a log
    ai tick looks like this:
    2010-Jan-14 10:35:31.669395 (M) [ai.tick] 0	 0	 -5	 -5
    fields are: tstamp, episode, step, reward, fitness
    """
    lc = LearningCurve()
    for line in f.xreadlines():
        line = line.strip().lower()
        m = ai_tick_pattern.search(line)
        if m:
            t = time.strptime(m.group('date'), timestamp_format)
            ms = int(m.group('msec'))
            episode = int(m.group('episode'))
            step = int(m.group('step'))
            reward = float(m.group('reward'))
            fitness = float(m.group('fitness'))
            lc.append( t, ms, episode, step, reward, fitness )
    return lc

def main():
    f = sys.stdin
    if len(sys.argv) > 1:
        f = open(sys.argv[1])
    lc = process_log(f)
    f.close()
    x = np.array(range(0,len(lc.episodes)))
    y = np.array(lc.episodes)
    pl.figure()
    pl.plot(x, y, linewidth=1.0)
    pl.xlabel('episode')
    pl.ylabel('fitness')
    pl.title('By-episode fitness')
    pl.grid(True)
    pl.show()
    print 'done'

if __name__ == "__main__":
    main()