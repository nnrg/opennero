# player (client) for OpenNERO maze log files

import socket
import sys
import struct
import re
import time
import cPickle as pickle
from math import radians
import observer

re_float = r'[-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?'
re_pose = re.compile(r'^\(M\) \[python\] (%s) (%s) (%s) (%s) (%s)$' % tuple([re_float for i in range(5)]))
re_seed = re.compile(r'^\(M\) \[python\] seed\: ([0-9]+)$')

def play(s, logfile):
    """ take a log file with poses and play it back """
    last_time = 0
    for line in logfile.xreadlines():
        line = line.strip()
        m = re_pose.match(line)
        if m:
            (t,x,y,z,th) = tuple([float(x) for x in m.group(1,2,3,4,5)])
            obsever.send(s, (x,y,radians(th)))
            print t, x, y, z, th
            if t > last_time:
                time.sleep(t - last_time)
            last_time = t
        m = re_seed.match(line)
        if m:
            seed = int(m.group(1))
            observer.send(s, (observer.E_SEED,seed))
            print 'seed:', seed

def drive(s):
    for line in sys.stdin.xreadlines():
        pose = tuple([time.time()] + [float(x) for x in line.strip().split()])
        observer.send(s,pose)
    observer.send(s,observer.E_QUIT)

if __name__ == "__main__":
    s = observer.connect(observer.HOST, observer.PORT)
    filename = '-'
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    if filename == '-':
	observer.send(s, (observer.E_TITLE, 'user input'))
        drive(s)
    else:
	observer.send(s, (observer.E_TITLE, filename))
        logfile = open(filename)
        if not logfile:
            print >> sys.stderr, 'Could not open file', filename
            sys.exit(1)
        play(s,logfile)
    s.close()

