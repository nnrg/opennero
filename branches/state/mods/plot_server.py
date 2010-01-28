#!/usr/bin/env python
"""Process and display performance of AI algorithms in OpenNERO.

plot_server reads a log file (or receives this file over the network) and 
plots the performance of the AI algorithm which is producing this log file.
"""

import sys
import re
import time
import numpy as np
import matplotlib.pyplot as pl
import matplotlib.mlab as mlab
import socket
import SocketServer
import tempfile

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"
__copyright__ = "Copyright 2010, The University of Texas at Austin"
__license__ = "LGPL"
__version__ = "0.1.0"

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4086

ai_tick_pattern = re.compile(r'(?P<date>[^\[]*)\.(?P<msec>[0-9]+) \(m\) \[ai\.tick\]\s+(?P<id>\S+)\s+(?P<episode>\S+)\s+(?P<step>\S+)\s+(?P<reward>\S+)\s+(?P<fitness>\S+)')
timestamp_fmt = r'%Y-%b-%d %H:%M:%S'
file_timestamp_fmt = r'%Y-%m-%d-%H-%M-%S'

def timestamped_filename(prefix = '', postfix = ''):
    return '%s%s%s' % (prefix, time.strftime(file_timestamp_fmt), postfix)

class LearningCurve:
    def __init__(self):
        self.min_time = None
        self.max_time = None
        self.data = []
        self.episodes = []
        self.fig = None

    def save(self):
        if self.fig:
            self.fig.savefig(timestamped_filename('opennero-','-episodes.png'))

    def display(self):
        x = np.array(range(len(self.episodes)))
        y = np.array(self.episodes)
        fig = pl.figure()
        pl.plot(x, y, linewidth=1.0)
        pl.xlabel('episode')
        pl.ylabel('fitness')
        pl.title('By-episode fitness')
        pl.grid(True)
        fig.savefig(timestamped_filename('opennero-','-fitness.png'))
        pl.show()        

    def reset(self):
        self.save()
        self.min_time = None
        self.max_time = None
        self.data = []
        self.episodes = []
        self.fig = pl.figure()
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
        print sec, episode, step, reward, fitness
        self.data.append( (time, msec, episode, step, reward, fitness) )

def process_line(lc, line):
    """Process a line of the log file and record the information in it in the LearningCurve lc
    """
    line = line.strip().lower()
    m = ai_tick_pattern.search(line)
    if m:
        t = time.strptime(m.group('date'), timestamp_fmt)
        ms = int(m.group('msec'))
        episode = int(m.group('episode'))
        step = int(m.group('step'))
        reward = float(m.group('reward'))
        fitness = float(m.group('fitness'))
        lc.append( t, ms, episode, step, reward, fitness )
        
def process_file(fname):
    lc = LearningCurve()
    f = open(fname)
    for line in f:
        line = line.strip()
        process_line(lc, line)
    f.close()
    return lc

class MyTCPHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        lc = LearningCurve()
        for line in self.rfile:
            line = line.strip()
            process_line(lc, line)
        lc.save()
        lc.display()

def main():
    lc = None
    if len(sys.argv) > 1:
        print 'opening OpenNERO log file', sys.argv[1]
        lc = process_file(sys.argv[1])
        lc.save()
        lc.display()
    else:
        # Create the server, binding to localhost on port 9999
        server = SocketServer.TCPServer(ADDR, MyTCPHandler)
        print 'Starting server on', ADDR
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()
    print 'done'

if __name__ == "__main__":
    main()
