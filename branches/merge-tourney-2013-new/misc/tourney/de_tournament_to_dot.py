#!/usr/bin/env python

'''Convert a double-elimination tournament log into a dot script.'''

import re
import sys
import collections

teams = collections.defaultdict(int)

def identify(team):
    name = re.sub(r'\W+', '_', team)
    id = teams[name]
    teams[name] += 1
    return name, id

def label(team):
    return team.split('/')[-1].split('.')[0]

winners = set()

print 'digraph tournament {'
print 'rankdir=LR'

for line in sys.stdin:
    m = re.search(r' ([^:]+):(.\d*) defeats ([^:]+):(.\d*)$', line)
    if m:
        winner, winner_id = identify(m.group(1))
        winner_key = '%s_%d' % (winner, winner_id)
        next_key = '%s_%d' % (winner, winner_id + 1)

        loser, loser_id = identify(m.group(3))
        loser_key = '%s_%d' % (loser, loser_id)

        if winner_id == 0:
            winners.add(winner_key)
        if loser_id == 0:
            winners.add(loser_key)
        if winner_key in winners and loser_key in winners:
            winners.add(next_key)

        print '%s [label="%s"]' % (winner_key, label(m.group(1)))
        print '%s [label="%s"]' % (loser_key, label(m.group(3)))
        print '%s [label="%s"]' % (next_key, label(m.group(1)))

        print '%s -> %s [label="%s"]' % (winner_key, next_key, m.group(2))
        print '%s -> %s [label="%s"]' % (loser_key, next_key, m.group(4))

print 'subgraph cluster_winner {'
for node in winners:
    print node
print '}'

print '}'
