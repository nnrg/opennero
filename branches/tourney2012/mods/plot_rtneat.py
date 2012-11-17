import sys
import os
import re
import time
from pylab import *

# timestamp format
timestamp_fmt = r'%Y-%b-%d %H:%M:%S'
# timestamp for file
file_timestamp_fmt = r'%Y-%m-%d-%H-%M-%S'
# general prefix for OpenNERO log lines (date and time with msec resolution)
log_prefix = r'(?P<date>[^\[]*)\.(?P<msec>[0-9]+) \(.\) '

# ----------------------------[ ai.rtneat log line regular expression ]-----------------------------------------
# ai.rtneat lines get printed during rtneat evaluations and can be used to keep track of rtneat progress
# an example line has the format
# 2011-Mar-02 12:52:55.167396 (!) [ai.rtneat] z-min: -0.0898853 z-max: 0.250617 r-min: [ -2.00352e+08 0 -4.97067e+08 -1.64796e+08 0 0 ] r-max: [ -2.59186e+06 0 -7.22071e+07 -6.11801e+06 8 0 ] w: [ 0 0 -1 1 0 0 ] mean: [ -5.74247e+07 0 -1.30798e+08 -2.47332e+07 5.71875 0 ] stdev: [ 6.33428e+07 0 1.22863e+08 4.67422e+07 2.75642 0 ]
# The fields are:
# * z-min: weighted Z-score minimum (based on population average and standard deviation)
# * z-max: weighted Z-score maximum (based on population average and standard deviation)
# * r-min: raw score minimum (D values)
# * r-max: raw score maximum (D values)
# * w: user-assigned weights (D values in [-1,1])
# * mean: average over mature population (D values)
# * stdev: standard deviation over mature population (D values)
ai_rtneat_pattern = re.compile(log_prefix + r'\[ai\.rtneat\] z-min: (?P<zmin>\S+) z-max: (?P<zmax>\S+) r-min: \[(?P<rmin>[^\]]+)\] r-max: \[(?P<rmax>[^\]]+)\] w: \[(?P<w>[^\]]+)\] mean: \[(?P<mean>[^\]]+)\] stdev: \[(?P<stdev>[^\]]+)\]')

# this is the format for the equivalent line in NERO 2.0 files
# opennero evaluateAll z-min: -2.16926 z-max: 0 r-min: [ 0 0 0 0 0 352.409 0 2230.01 1e+10 3.40282e+38 21502.5 5970.86 ] r-max: [ 0 134 22 0 0 1465.59 0 18089.8 1.499e+10 -3.40282e+38 95074.5 170507 ] w: [0.00 0.00 0.00 -0.00 0.00 -0.00 -0.00 -0.00 -0.00 -1.00 0.00] mean: [ 0 17.44 4.12 0 0 769.721 0 9097.14 1.12429e+10 0 57245 87640.1 ] stdev: [ 0 35.9222 6.43938 0 0 275.282 0 4604.4 1.76394e+09 0 17438.9 53606.8 ]
nero_pattern = re.compile(r'opennero evaluateall z-min: (?P<zmin>\S+) z-max: (?P<zmax>\S+) r-min: \[(?P<rmin>[^\]]+)\] r-max: \[(?P<rmax>[^\]]+)\] w: \[(?P<w>[^\]]+)\] mean: \[(?P<mean>[^\]]+)\] stdev: \[(?P<stdev>[^\]]+)\]')

FITNESS_STAND_GROUND = "Stand ground"
FITNESS_STICK_TOGETHER = "Stick together"
FITNESS_APPROACH_ENEMY = "Approach enemy"
FITNESS_APPROACH_FLAG = "Approach flag"
FITNESS_HIT_TARGET = "Hit target"
FITNESS_AVOID_FIRE = "Avoid fire"
FITNESS_DIMENSIONS = [FITNESS_STAND_GROUND, FITNESS_STICK_TOGETHER, 
    FITNESS_APPROACH_ENEMY, FITNESS_APPROACH_FLAG, FITNESS_HIT_TARGET, 
    FITNESS_AVOID_FIRE]

zmin, zmax, rmin, rmax, w, mean, stdev = [], [], [], [], [], [], []

def process_line(line):
    """
    Process a line of the log file and record the information in it in the LearningCurve
    """
    global zmin, zmax, rmin, rmax, w, mean, stdev
    line = line.strip().lower()
    m = ai_rtneat_pattern.search(line)
    if not m:
        m = nero_pattern.search(line)
    if m:
        #t = time.strptime(m.group('date'), timestamp_fmt) # time of the record
        #ms = int(m.group('msec')) / 1000000.0 # the micro-second part in seconds
        #base = time.mktime(t) + ms # seconds since the epoch
        zmin.append(float(m.group('zmin')))
        zmax.append(float(m.group('zmax')))
        rmin.append([float(x) for x in m.group('rmin').strip().split()])
        rmax.append([float(x) for x in m.group('rmax').strip().split()])
        w.append([float(x) for x in m.group('w').strip().split()])
        mean.append([float(x) for x in m.group('mean').strip().split()])
        stdev.append([float(x) for x in m.group('stdev').strip().split()])

def process_file(f):
    line = f.readline()
    while line:
        process_line(line.strip())
        line = f.readline()

def main():
    global zmin, zmax, rmin, rmax, w, mean, stdev
    fname = "nero_log.txt"
    if len(sys.argv) > 1:
        fname = sys.argv[1]
    with open(fname) as f:
        process_file(f)
    zmin, zmax, rmin, rmax, w, mean, stdev = np.array(zmin), np.array(zmax), np.array(rmin), np.array(rmax), np.array(w), np.array(mean), np.array(stdev)
    print np.shape(zmin), np.shape(zmax), np.shape(rmin), np.shape(rmax), np.shape(w), np.shape(mean), np.shape(stdev)

    # plot each dimension in a separate subplot
    dd = np.size(mean,1)
    for d in range(dd):
        figure()
        ax = subplot('211')
        #ax.errorbar(np.arange(np.size(mean,0)), mean[:,d], yerr=stdev[:,d])
        ax.hold(True)
        maxline = ax.plot(rmax[:,d], label='max')
        avgline = ax.plot(mean[:,d], label='avg')
        minline = ax.plot(rmin[:,d], label='min')
        ax.legend()
        #ax.ylabel('Raw Fitness')
        if len(FITNESS_DIMENSIONS) == dd:
            ax.set_title(FITNESS_DIMENSIONS[d])
        if d < np.size(w,1):
            ax = ax = subplot('212')
            ax.plot(w[:,d])
            ax.set_title('Slider Weight')
        #ax.xlabel('Tick')
        
    show()

if __name__ == "__main__":
    main()
