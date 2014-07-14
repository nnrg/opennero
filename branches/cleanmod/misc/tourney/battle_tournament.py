#!/usr/bin/env python

# Run a NERO_Battle tournament.

import collections
import logging
import math
import multiprocessing as mp
import optparse
import os
import random
import re
import subprocess
import sys
import time

FLAGS = optparse.OptionParser(usage='%s [OPTIONS] POPULATION-FILE...' % sys.argv[0])
FLAGS.add_option('-v', '--verbose', action='store_true', help='be more verbose')
FLAGS.add_option('-c', '--condor', action='store_true', help='run battles on Condor')
FLAGS.add_option('-d', '--duration', default=60, type=int, metavar='N',
                 help='allow each battle to run for N seconds (default: 60)')
FLAGS.add_option('-n', '--concurrency', default=2, type=int, metavar='N',
                 help='run N battles concurrently (default: 2)')
FLAGS.add_option('-r', '--round-robin', default=0, type=int, metavar='N',
                 help='run a round-robin tournament with N rounds')
FLAGS.add_option('-t', '--test', action='store_true',
                 help='print out matchups without running OpenNero')


def test_bracket(team1, team2, duration):
    """Just print out the teams we're pitting against each other."""
    logging.info('bracket matchup: %s vs %s', team1, team2)
    return 'damages sustained by: blue: 0 red: 1'

def fmt(s):
    return re.sub(r'\W+', '_', os.path.splitext(s)[0])

def run_battle_locally(team1, team2, duration):
    """Run a battle locally (i.e. on the local CPU) for duration seconds."""
    logging.info('running battle locally: %s vs %s', team1, team2)
    opa = os.path.abspath
    logfile = 'nero_battle_%s_%s_%d_%d' % (
        fmt(team1), fmt(team2), time.time(), os.getpid())
    battle = subprocess.Popen(
        ['./condor_battle.sh', opa(team1), opa(team2), str(duration), logfile],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return battle.communicate()[0]


CONDOR_TEMPLATE = """\
+Group = "GRAD"
+Project = "AI_OPENNERO"
+ProjectDescription = "opennero tournament"
Universe = vanilla
Requirements = Arch == "X86_64" && Memory >= 2000
Image_Size = 250000
Input = /dev/null
Error = %(logfile)s.stderr
Output = %(logfile)s.stdout
Log = %(logfile)s.condor
Executable = condor_battle.sh
Arguments = %(team1)s %(team2)s %(duration)d %(logfile)s.result
Queue 1
"""

def run_battle_on_condor(team1, team2, duration):
    """Run a battle on condor for duration seconds."""
    logfile = 'nero_battle_%s_%s_%d_%d' % (
        fmt(team1), fmt(team2), time.time(), os.getpid())
    logging.info('running battle on condor: %s', logfile)

    submit = subprocess.Popen(
        ['condor_submit'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    submit_command = CONDOR_TEMPLATE % dict(
        logfile=logfile,
        duration=duration,
        team1=os.path.abspath(team1),
        team2=os.path.abspath(team2),
        )

    with open(logfile + '.submit','w') as f:
        print >> f, submit_command

    submit.communicate(submit_command)

    # wait for the job to complete and return the logging output.
    condition = subprocess.Popen(
        ['condor_wait', '%s.condor' % logfile],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    condition.wait()

    return open('%s.result' % logfile).read()


def parse_scores(stdout):
    """Parse scores out of some opennero logging output."""
    scores = [(-1, -1)]
    for line in stdout.splitlines():
        m = re.search(r'damages sustained by: blue: (\d+) red: (\d+)$', line)
        if m:
            # NB -- scores here are reversed from the log file because a team
            # gets points for causing damage to its opponent.
            scores.append((int(m.group(2)), int(m.group(1))))
    #logging.debug('parsed %d scores from %dkB of stdout data: %s',
    #              len(scores), len(stdout) // 1000, scores[-1])
    return scores[-1]


def schedule_battles(fight, team_queue, score_queue):
    logging.debug('starting up scheduling process')
    while True:
        teams = team_queue.get()
        if teams is None:
            break
        team1, team2, duration = teams
        score1, score2 = parse_scores(fight(team1, team2, duration))
        score_queue.put(((team1, score1), (team2, score2)))
    logging.debug('shutting down scheduling process')


def round_robin(opts, teams, team_queue, score_queue):
    """Given a set of teams, run a round-robin (all-pairs) tournament.

    Returns a map from team name to total score.
    """
    waiting = 0
    for i, team1 in enumerate(teams):
        for j, team2 in enumerate(teams):
            for _ in range(opts.round_robin):
                if team1 != team2:
                    team_queue.put((team1, team2, opts.duration))
                    waiting += 1
    scoreboard = collections.defaultdict(int)
    while waiting > 0:
        waiting -= 1
        (team1, score1), (team2, score2) = score_queue.get()
        scoreboard[team1] += score1
        scoreboard[team2] += score2
    return scoreboard


def parallel_matchups(opts, team_pairs, team_queue, score_queue):
    """Given a list of team pairs, run each pair in parallel.

    Generates a sequence of ((winner, score), (loser, score)) pairs.
    """
    waiting = 0
    for team1, team2 in team_pairs:
        if team1 is None or team2 is None:
            continue  # this match is a buy.
        if random.random() < 0.5:
            team1, team2 = team2, team1
        team_queue.put((team1, team2, opts.duration))
        waiting += 1
    tries = {}
    while waiting > 0:
        (team1, score1), (team2, score2) = score_queue.get()
        n = tries.get( (team1, team2), 0 )
        # check the match actually ran and retry a few times
        if score1 >= 0 and score2 >= 0 or n > 10:
            waiting -= 1
        else:
            tries[(team1,team2)] = n+1
            team_queue.put((team1, team2, opts.duration))
            logging.info('failed to play %s v %s, retrying %d', team1, team2, n+1)
            continue
        if score2 > score1 or (score1 == score2 and random.random() < 0.5):
            logging.info('%s:%d defeats %s:%d', team2, score2, team1, score1)
            yield (team2, score2), (team1, score1)
        else:
            logging.info('%s:%d defeats %s:%d', team1, score1, team2, score2)
            yield (team1, score1), (team2, score2)


def world_cup(opts, teams, team_queue, score_queue):
    """Run a world-cup-style tournament.
    """
    # TODO: not finished
    raise NotImplementedError

    # first we run round-robin within small groups to get a pool of teams for
    # seeding the bracket. this also lets us ensure that the number of teams in
    # the bracket is a power of 2.
    assert 0 == opts.bracket_size & (opts.bracket_size - 1), \
        '--bracket-size must be a power of 2'

    def partition(s, n):
        """Divide set s into n approximately equal-sized groups."""
        groups = [[] for _ in range(n)]
        i = 0
        while s:
            groups[i].append(s.pop())
            i += 1
        return groups

    firsts = []
    seconds = []
    for group in partition(teams, opts.bracket_size):
        results = round_robin(opts, group, team_queue, score_queue)
        winners = sorted(results.iteritems(), key=lambda x: -x[0])
        firsts.append(winners[0])
        seconds.append(winners[1])

    L = len(seconds) // 2
    matchups = zip(firsts, seconds[L:] + seconds[:L])

    results = parallel_matchups(opts, matchups, team_queue, score_queue)


def double_elimination(opts, teams, team_queue, score_queue):
    """Run a double-elimination tournament.

    Returns (winner, scoreboard). The scoreboard is a map from team to score.
    """
    scoreboard = dict((t, 0) for t in teams)

    def pairs(s):
        """Create a list of pairs from elements in list s."""
        h = len(s) // 2
        return zip(s[0:h],reversed(s[h:]))

    def matches(ps):
        """Run the given set of pairs in parallel, assembling the results."""
        results = parallel_matchups(opts, ps, team_queue, score_queue)
        for (winner, ws), (loser, ls) in results:
            scoreboard[winner] += ws
            scoreboard[loser] += ls
            yield winner, loser

    def pad_to_power_of_2(s):
        """Append sufficient Nones to s to make its length a power of 2."""
        l = 2 ** int(math.ceil(math.log(len(s), 2))) - len(s)
        return s + [None] * l

    def pad_to_multiple_of_2(s):
	"""Append sufficient Nones to s to make it have even length."""
	return s + [None] * (len(s) % 2)

    winners = teams
    losers = []

    level = 0
    while len(winners) > 1 or len(losers) > 1:
        logging.debug('level %d, %d winners, %d losers', level, len(winners), len(losers))
        if len(winners) > 1:
            for _, loser in matches(pairs(pad_to_power_of_2(winners))):
                winners.remove(loser)
                losers.append(loser)
        logging.debug(' - level %d, %d winners, %d losers', level, len(winners), len(losers))
        while len(losers) >= len(winners) and len(losers) > 1:
            for _, loser in matches(pairs(pad_to_power_of_2(losers))):
                losers.remove(loser)
            logging.debug(' - level %d, %d winners, %d losers', level, len(winners), len(losers))
        level += 1

    # If the winners-bracket team loses the final match, both teams will have
    # lost once, so the final match needs to be played again.
    final = (winners[0], losers[0])
    winner = None
    w, _ = matches([final]).next()
    if w == winners[0]:
        winner = w
    else:
        winner, _ = matches([final]).next()
    return winner, scoreboard


if __name__ == '__main__':
    opts, teams = FLAGS.parse_args()

    logging.basicConfig(
        stream=sys.stdout,
        level=opts.verbose and logging.DEBUG or logging.INFO,
        format='%(levelname).1s %(asctime)s %(message)s')

    fight = opts.condor and run_battle_on_condor or run_battle_locally
    if opts.test:
        fight = test_bracket

    # set up a processing pool to schedule battles.
    team_queue = mp.Queue()
    score_queue = mp.Queue()
    workers = [mp.Process(target=schedule_battles,
                          args=(fight, team_queue, score_queue))
               for _ in range(opts.concurrency)]
    [w.start() for w in workers]

    # run the actual tournament.
    random.shuffle(teams)
    if opts.round_robin:
        results = round_robin(opts, teams, team_queue, score_queue)
    else:
        w, results = double_elimination(opts, teams, team_queue, score_queue)
        print 'Tournament Winner:', w
    print '\n'.join('%d %s' % (s, t) for t, s in results.iteritems())

    # clean up workers.
    [team_queue.put(None) for w in workers]
    [w.join() for w in workers]
