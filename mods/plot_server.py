#!/usr/bin/env pythonw
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

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4086
SAVE_EVERY = 5

ai_tick_pattern = re.compile(r'(?P<date>[^\[]*)\.(?P<msec>[0-9]+) \(m\) \[ai\.tick\]\s+(?P<id>\S+)\s+(?P<episode>\S+)\s+(?P<step>\S+)\s+(?P<reward>\S+)\s+(?P<fitness>\S+)')
timestamp_fmt = r'%Y-%b-%d %H:%M:%S'
file_timestamp_fmt = r'%Y-%m-%d-%H-%M-%S'

def timestamped_filename(prefix = '', postfix = ''):
    return '%s%s%s' % (prefix, time.strftime(file_timestamp_fmt), postfix)

class AgentHistory():
    def __init__(self):
        self.episode_time = []
        self.episode_fitness = []
        self.time = []
        self.fitness = []
    def append(self, ms, fitness):
        self.episode_time.append(ms)
        self.episode_fitness.append(fitness)
    def episode(self):
        if len(self.episode_time) > 0:
            self.time.append(self.episode_time[-1])
            self.fitness.append(self.episode_fitness[-1])
        self.episode_time = []
        self.episode_fitness = []
    def plot(self):
        if len(self.time) > 0:
            x = np.array(self.time)
            y = np.array(self.fitness)
            pl.scatter(x, y, linewidth=1.0)        

class LearningCurve:
    def __init__(self):
        self.histories = {}
        self.unsaved_for = 0
         
    def append(self, id, ms, episode, step, reward, fitness):
        record = None
        if id in self.histories:
            record = self.histories[id]
        else:
            record = AgentHistory()
            self.histories[id] = record
        if step == 0:
            record.episode()
            if self.unsaved_for > SAVE_EVERY:
                self.save()
                self.unsaved_for = 0
            else:
                self.unsaved_for += 1
        record.append(ms, fitness)
        
    def save(self):
        fig = pl.figure()
        pl.xlabel('episode')
        pl.ylabel('fitness')
        pl.title('By-episode fitness')
        pl.grid(True)
        pl.hold(True)
        for id in self.histories:
            self.histories[id].plot()
        #fname = timestamped_filename('opennero-','-fitness.png')
        fname = 'opennero-fitness.png'
        print 'saving to:', fname
        fig.savefig(fname)
        #pl.show()

    def process_line(self, line):
        """Process a line of the log file and record the information in it in the LearningCurve
        """
        line = line.strip().lower()
        m = ai_tick_pattern.search(line)
        if m:
            id = int(m.group('id')) # id of the agent
            t = time.strptime(m.group('date'), timestamp_fmt) # time of the record
            ms = int(m.group('msec')) # milliseconds
            episode = int(m.group('episode'))
            step = int(m.group('step'))
            reward = float(m.group('reward'))
            fitness = float(m.group('fitness'))
            ms += time.mktime(t) * 1000000
            self.append( id, ms, episode, step, reward, fitness )

    def process_file(self, f):
        line = f.readline()
        while line:
            self.process_line(line.strip())
            line = f.readline()

class PlotTCPHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        lc = LearningCurve()
        lc.process_file(self.rfile)
        lc.save()
        #lc.display()

def main():
    if len(sys.argv) > 1:
        print 'opening OpenNERO log file', sys.argv[1]
        f = open(sys.argv[1])
        lc = LearningCurve()
        lc.process_file(f)
        f.close()
        lc.save()
        #lc.display()
    else:
        # Create the server, binding to localhost on port 9999
        server = SocketServer.TCPServer(ADDR, PlotTCPHandler)
        print 'Listening on ', ADDR
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()
    print 'done'

if __name__ == "__main__":
    main()
