#!/usr/bin/env python

"""
Convert the output of a double-elimination tournament into a DOT-file and an HTML file
"""

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"

import sys
import os
import re
import csv
import optparse
import subprocess

NODES = {}
TEAM_NAMES = None

FLAGS = optparse.OptionParser(usage='%s [OPTIONS] ...' % sys.argv[0])
FLAGS.add_option('-p', '--prefix', type="string", dest="prefix", default='')
FLAGS.add_option('-t', '--teams', type="string", dest="team_names_file", default='entries.csv')

def get_node(key, silent = False):
    """ Get a unique key for a node """
    global NODES
    if key not in NODES:
        idx = len(NODES)
        NODES[key] = idx
        if not silent: print 'node [ shape = box, label = "%s" ] %d;' % (key, idx)
    return str(NODES[key])

def fmt(s):
    return re.sub(r'\W+', '_', os.path.splitext(s)[0])

def read_team_names(opts):
    team_names = {}
    with open(opts.team_names_file, 'rU') as f:
        reader = csv.reader(f)
        for row in reader:
            fname = row[1].replace('.gz','')
            basename, ext = os.path.splitext(fname)
            fname = fmt(fname)
            team_names[fname] = '%s (%s)' % (basename, row[0])
    return team_names

def get_label(key):
    if key.startswith(TEAM_NAME_PREFIX):
        key = key[len(TEAM_NAME_PREFIX):]
    return key

def get_team_name(opts, key):
    global TEAM_NAMES
    if TEAM_NAMES is None:
        TEAM_NAMES = read_team_names(opts)
    return TEAM_NAMES.get(re.sub(r'\_+', '_', key), key)

def print_graph(opts, hh, winner, top=None):
    """ print the graph starting from the winner """
    if not top:
        top = winner
    if winner not in hh or not hh[winner]:
        return
    team1, score1, team2, score2 = hh[winner].pop(-1)
    next = '%s:%d v \\n%s:%d' % (get_team_name(opts, team1),score1,get_team_name(opts, team2),score2)
    s = get_node(next) + '->' + get_node(top) + ' [color=blue];' #% get_team_name(winner)
    print s
    print_graph(opts, hh, team1, next)
    print_graph(opts, hh, team2, next)

def print_header():
    """ Print the HTML page header """
    print '''<?xml version="1.0" encoding="iso-8859-1"?>
    <!DOCTYPE html
    PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
    <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
    <head>
    <title>OpenNERO Tournament 2011</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
    <link rel="stylesheet" type="text/css" href="Style/default.css" media="screen" title="Default"/>
    <script type="text/javascript" src="Script/util.js"></script>
    </head>
    <body>
    '''

def print_footer():
    """ Pring the HTML page footer """
    print '</body></html>'

def print_html(opts, hh, winner):
    """ Print the HTML tree startig with the winner """
    if winner not in hh or not hh[winner]:
        return
    team1, score1, team2, score2 = hh[winner].pop(-1)
    next = '%s:%d v %s:%d' % (get_team_name(opts, team1),score1,get_team_name(opts, team2),score2)
    nodename = 'node'+get_node(next, True)
    print '''<li><a href="#" onclick="toggleVisibility('%(nodename)s'); return false;">[+/-]</a> %(text)s <div id="%(nodename)s">''' % {'nodename': nodename, 'text': next}
    if team1 in hh and hh[team1] or team2 in hh and hh[team2]:
        print '<ul>'
        print_html(opts, hh, team1)
        print_html(opts, hh, team2)
        print '</ul>'
    print '</div></li>'

def get_the_trees(DTA):
    """ Get the bracket trees from the list of (winner, score, loser, score) tuples """
    losers = set()
    winner_tree = {}
    loser_tree = {}
    for team1, score1, team2, score2 in DTA:
        tree = loser_tree if team1 in losers else winner_tree
        losers.add(team2)
        h = tree.get(team1, [])
        h.append( (team1, score1, team2, score2) )
        tree[team1] = h
    return winner_tree, loser_tree

re_defeats = re.compile(r' ([^:]+):(.\d*) defeats ([^:]+):(.\d*)$')
re_winner = re.compile(r'^Tournament Winner: (.+)$')
re_score = re.compile(r'(.\d*) (\S+)$')

def main():
    """ Convert the output of a double-elimination tournament into a DOT-file and an HTML file """
    opts, rargs = FLAGS.parse_args()
    global TEAM_NAME_PREFIX
    TEAM_NAME_PREFIX = opts.prefix
    fname = rargs[0]
    DTA = []
    SCORES = []
    tourney_winner = None
    with open(fname) as f:
        for line in f:
            m = re_defeats.search(line)
            if m:
                team1, score1, team2, score2 = m.groups()
                DTA.append( (get_label(team1), int(score1), get_label(team2), int(score2)) )
                print DTA[-1]
            if not tourney_winner:
                m = re_winner.match(line)
                if m:
                    tourney_winner = get_team_name(opts, get_label(m.group(1)))
                    print 'tourney_winner:',tourney_winner
            else:
                m = re_score.match(line)
                if m:
                    score = int(m.group(1))
                    team = get_team_name(opts, get_label(m.group(2)))
                    SCORES.append( (score, team) )
                    print 'score:', score, team

    winner_tree, loser_tree = get_the_trees(DTA)

    winner = DTA[-1][0]
    loser = DTA[-1][2]

    sys.stdout = open('winners-bracket.dot','w')
    print 'digraph opennero_tourney_2011_winners {'
    print '  rankdir=RL;'
    print_graph(opts, winner_tree, winner)
    print '}'
    sys.stdout.close()

    sys.stdout = open('losers-bracket.dot','w')
    print 'digraph opennero_tourney_2011_losers {'
    print '  rankdir=RL;'
    print_graph(opts, loser_tree, loser)
    print '}'
    sys.stdout.close()

    for lang in ['pdf','png']:
        for fname in ['winners-bracket', 'losers-bracket']:
            try:
                command = 'dot -T{lang} -o {fname}.{lang} {fname}.dot'.format(fname=fname,lang=lang)
                retcode = subprocess.call(command, shell=True)
                if retcode < 0:
                    print >>sys.stderr, 'Error code {retcode} when running {command}'.format(retcode=-retcode, command=command)
            except OSError, e:
                print >>sys.stderr, 'Error when running "{command}":'.format(command=command), e

    winner_tree, loser_tree = get_the_trees(DTA)

    sys.stdout = open('tourney.html','w')
    print_header()

    print '<h1>OpenNERO Tournament Results</h1>'

    print '<h2>Tournament Winner</h2>'
    print '<p>Congratulations, %s!</p>' % tourney_winner
    print '<hr/>'

    print '<h2>Cumulative Scores</h2>'
    print '<table><thead>'
    print '<tr><th>Score</th><th>Team</th></tr>'
    print '</thead><tbody>'
    for score, team in SCORES:
        print '<tr><td>%d</td><td>%s</td></tr>' % (score, team)
    print '</tbody></table>'

    print "<h2>Winners' Bracket</h2>"
    print '<p><a href="winners-bracket.pdf"><img src="winners-bracket.png"/></a></p>'
    print "<ul>"
    print_html(opts, winner_tree, winner)
    print '</ul><hr/>'

    print "<h2>Losers' Bracket</h2>"
    print '<p><a href="losers-bracket.pdf"><img src="losers-bracket.png"/></a></p>'
    print "<ul>"
    print_html(opts, loser_tree, loser)
    print "</ul>"

    print_footer()
    sys.stdout.close()

if __name__ == "__main__":
    # actually run the script
    main()
