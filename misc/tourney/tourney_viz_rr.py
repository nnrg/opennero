#!/usr/bin/env python

"""
Convert the output of a round-robin tournament into a color matrix plots
"""

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"

import re
import os
import os.path
import glob
import gzip
from pylab import *

from tourney_viz_de import FLAGS, get_team_name

# add our own options
FLAGS.add_option('-v', '--verbose', help='print info along the way', action='store_true', default=True)
FLAGS.add_option('-g', '--graphics', help='show graphics on screen', action='store_true', default=False)
FLAGS.add_option('-d', '--teamdir', help='directory with team files', type='string', default='tourney')

def fmt(s):
    return re.sub(r'\W+', '_', os.path.splitext(s)[0])

def main():
    opts, rargs = FLAGS.parse_args()

    prefix = opts.prefix.replace(os.path.sep, r'\_')
    re_fname_str = r'.*%s\_(.+)\_\_%s\_(.+)\_\d+\_\d+.result' % (prefix, prefix)
    re_fname = re.compile(re_fname_str)
    re_damage =  re.compile(r'damages sustained by: blue: (\d+) red: (\d+)$')
    re_fname2_str = r'.*LOG CREATED.*%s\_(.+)\_\_.*%s\_(.+).result' % (prefix, prefix)
    re_fname2 = re.compile(re_fname2_str)
    re_fname3_str = r'.*%s\_(.+)\_\_.*%s\_(.+).result' % (prefix, prefix)
    re_fname3 = re.compile(re_fname3_str)

    data = {}
    datadiff = {}

    counts = {}

    teams = set()

    if rargs:
        lognames = rargs
    else:
        lognames = ['RR.log']

    for logname in lognames:
        opener = gzip.open if logname.endswith('.gz') else open
        with opener(logname) as f:
            team1, team2 = None, None
            for line in f:
                line = line.strip()
                score1, score2 = None, None
                m = re_fname.match(line)
                if not m:
                    m = re_fname2.match(line)
                if not m:
                    m = re_fname3.match(line)
                if m:
                    team1, team2 = m.groups()
                    teams.add(team1)
                    teams.add(team2)
                else:
                    m = re_damage.search(line)
                    if m:
                        score1, score2 = int(m.group(2)), int(m.group(1))
                        #cnt = counts.get((team1, team2), 0)
                        #prv = data.get((team1, team2), 0)
                        #counts[(team1, team2)] = cnt + 1
                        #data[(team1, team2)] = (prv * cnt + score1 - score2) / float(cnt + 1)
                        data[(team1, team2)] = score1 - score2
                        datadiff[(team1, team2)] = (score1, score2)
        print 'Read in %s with %d teams and %d/%d games.' % (logname, len(teams), len(data), len(teams) * (len(teams) - 1))

    with open('tourney-run-RR-raw.txt','w') as f:
        for team1, team2 in data:
            score1, score2 = datadiff[(team1, team2)]
            print >>f, '%s\t%d\t%s\t%d' % (team1, score1, team2, score2)

    N = len(teams)

    matrix = np.zeros((N, N))
    matrix0 = np.zeros((N, N))

    teams = [t for t in sorted(teams)]

    for r, team1 in enumerate(teams):
        for c, team2 in enumerate(teams):
            matrix[r,c] = data.get((team1, team2), data.get((team2, team1), np.nan))
            matrix0[r,c] = data.get((team1, team2), data.get((team2, team1), 0))

    # colormap
    cmap = cm.jet

    # set NaN values as different from everything
    cmap.set_bad('k')

    # get a list of teams
    team_sizes = {}
    team_names = {}
    for fname in os.listdir(opts.teamdir):
        fsize = os.stat(os.path.join(opts.teamdir,fname)).st_size
        key = fmt(fname)
        team_sizes[key] = fsize
        team_names[key] = fname
        c = (np.isnan(matrix)).sum(1)
        idx = c.argsort()
    with open('tourney-run-RR-unplayed.txt', 'w') as f:
        for i in reversed(idx):
            if c[i] > 1:
                team_size = team_sizes.get(teams[i], -1)
                team_name = get_team_name(opts, teams[i])
                print >>f, '%d\t%d\t%s' % (c[i], team_size, team_name)

    figure()
    im = imshow(matrix, cmap=cmap, interpolation='nearest')
    colorbar()
    title('teams sorted alphabetically')
    savefig('tourney-run-RR.png')

    figure()
    m = np.mean(matrix0, axis = 1)
    s = 1.96 * np.std(matrix0, axis = 1) / np.sqrt(N)
    idx = m.argsort()
    errorbar(np.arange(N), m[:,idx], yerr=s[:,idx])
    title('sorted team mean score differentials')
    savefig('tourney-run-RR-scores.png')
    with open('tourney-run-RR-scores.txt','w') as f:
        for i in reversed(idx):
            team_name = get_team_name(opts, teams[i])
            print >>f, '%f\t%f\t%s' % (m[i], s[i], team_name)

    figure()
    c = (matrix0 > 0).sum(1)
    idx = c.argsort()
    scatter(np.arange(N), c[idx])
    title('sorted win counts')
    savefig('tourney-run-RR-wins.png')
    with open('tourney-run-RR-wins.txt','w') as f:
        for i in reversed(idx):
            team_name = get_team_name(opts, teams[i])
            print >>f, '%d\t%s' % (c[i], team_name)

    zr, zc = nonzero(isnan(matrix))

    f = open('tourney-run-RR-unplayed-pairs.txt','w')
    f2 = open('remainder-rr.dat', 'w')
    f3 = open('remainder-rr.sh', 'w')
    for (r,c) in zip(zr, zc):
        if r != c:
            team1, team2 = teams[r], teams[c]
            team1f, team2f = team_names[team1], team_names[team2]
            path = opts.prefix
            print >>f, '%s\t%s' % (team1f, team2f)
            print >>f2, '{path}/{team1f} {path}/{team2f}'.format(
                team1 = team1, team2 = team2, team1f = team1f, team2f = team2f, prefix = prefix, path = path)
            print >>f3, './condor_battle.sh {path}/{team1f} {path}/{team2f} 300 {prefix}_{team1}__{prefix}_{team2}.result'.format(
                team1 = team1, team2 = team2, team1f = team1f, team2f = team2f, prefix = prefix, path = path)

    figure()
    s = np.sum(matrix0, axis = 1)
    # sort the matrix by rows and columns
    matrix = matrix[s.argsort(), :]
    matrix = matrix[:, s.argsort()]
    im = imshow(matrix, cmap=cmap, interpolation='nearest')
    colorbar()
    title('teams sorted by mean score differential')
    savefig('tourney-run-RR-sorted.png')

    if opts.graphics:
        show()

if __name__ == "__main__":
    main()
