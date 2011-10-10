#!/usr/bin/env python

# Run a round-robin NERO_Battle tournament.

import logging
import multiprocessing as mp
import optparse
import os
import random
import re
import subprocess
import sys
import threading
import time

FLAGS = optparse.OptionParser(usage='%s [OPTIONS] POPULATION-FILE...' % sys.argv[0])
FLAGS.add_option('-c', '--condor', action='store_true', help='run battles on Condor')
FLAGS.add_option('-d', '--duration', default=60, type=int, metavar='N',
                 help='allow each battle to run for N seconds (default: 60)')
FLAGS.add_option('-n', '--concurrency', default=2, type=int, metavar='N',
                 help='run N battles concurrently (default: 2)')
FLAGS.add_option('-r', '--rounds', default=1, type=int, metavar='N',
                 help='run a tournament with N complete rounds (default: 1)')
FLAGS.add_option('-v', '--verbose', action='store_true', help='be more verbose')


def kill_after(process, delay):
    '''Kill a process after some delay.'''
    time.sleep(delay)
    logging.info('sending TERM to %s', process)
    process.terminate()
    time.sleep(2)
    if process.poll() is None:
        logging.info('sending KILL to %s', process)
        process.kill()


def run_battle_locally(team1, team2, duration):
    '''Run a battle locally (i.e. on the local CPU) for duration seconds.'''
    logging.info('running battle locally: %s vs %s', team1, team2)
    team1 = os.path.abspath(team1)
    team2 = os.path.abspath(team2)
    battle = subprocess.Popen([
        './OpenNERO',
        '--mod', 'NERO_Battle',
        '--modpath', 'NERO_Battle:_NERO:common',
        '--headless',
        '--command', 'Match(%r, %r)' % (team1, team2),
        ],
        stdout=subprocess.PIPE,
        )

    # kill the battle after some duration.
    t = threading.Thread(target=kill_after, args=(battle, duration))
    t.isDaemon = True
    t.start()

    return battle.communicate()[0]


CONDOR_TEMPLATE = '''\
+Group = "GRAD"
+Project = "AI_OPENNERO"
+ProjectDescription = "opennero tournament"
Universe = vanilla
Requirements = Lucid
Input = /dev/null
Error = %(logfile)s.stderr
Output = %(logfile)s.stdout
Log = %(logfile)s.condor
Executable = condor_battle.sh
Arguments = %(team1)s %(team2)s %(duration)d
Queue 1
'''

def run_battle_on_condor(team1, team2, duration):
    '''Run a battle on condor for duration seconds.'''
    logfile = '%s_%s_%d_%d' % (re.sub(r'\W+', '_', team1),
                               re.sub(r'\W+', '_', team2),
                               time.time(),
                               os.getpid())

    logging.info('running battle on condor: %s', logfile)

    submit = subprocess.Popen(
        ['condor_submit'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    submit.communicate(CONDOR_TEMPLATE % dict(
        logfile=logfile,
        duration=duration,
        team1=os.path.abspath(team1),
        team2=os.path.abspath(team2),
        ))

    # wait for the job to complete and return the logging output.
    condition = subprocess.Popen(
        ['condor_wait', '%s.condor' % logfile],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    condition.wait()

    return open('%s.stdout' % logfile).read()


def parse_scores(stdout):
    '''Parse scores out of some opennero logging output.'''
    scores = [(None, None)]
    for line in stdout.splitlines():
        m = re.search(r'Blue Team Score: (\d+) Red Team Score: (\d+)$', line)
        if m:
            scores.append((int(m.group(1)), int(m.group(2))))
    logging.info('parsed %d scores from %dkB of stdout data',
                 len(scores), len(stdout) // 1000)
    return scores


def schedule_battles(fight, team_queue, score_queue):
    logging.debug('starting up scheduling process')
    while True:
        teams = team_queue.get()
        if teams is None:
            break
        team1, team2, duration = teams
        scores = parse_scores(fight(team1, team2, duration))
        score_queue.put((team1, team2, scores))
    logging.debug('shutting down scheduling process')


def main(opts, teams):
    '''Given a set of teams, run a round-robin (all-pairs) tournament.'''
    fight = opts.condor and run_battle_on_condor or run_battle_locally

    # set up a processing pool to schedule battles.
    team_queue = mp.Queue()
    score_queue = mp.Queue()
    workers = [mp.Process(target=schedule_battles,
                          args=(fight, team_queue, score_queue))
               for _ in range(opts.concurrency)]
    [w.start() for w in workers]

    # add all unique pairs of teams to the scheduling queue. because scoring is
    # zero-sum, we don't need to run symmetric pairs twice.
    waiting = 0
    for i, team1 in enumerate(teams):
        for j, team2 in enumerate(teams):
            if j > i:
                for _ in range(opts.rounds):
                    team_queue.put((team1, team2, opts.duration))
                    waiting += 1

    # collect and aggregate scores from the battles.
    dashboard = dict((t, 0) for t in teams)
    while waiting:
        waiting -= 1
        team1, team2, scores = score_queue.get()
        s1, s2 = scores[-1]
        if s1 is not None and s2 is not None:
            dashboard[team1] += s1 - s2
            dashboard[team2] += s2 - s1

    # clean up the workers.
    [team_queue.put(None) for w in workers]
    [w.join() for w in workers]

    print dashboard


if __name__ == '__main__':
    logging.basicConfig(
        stream=sys.stdout,
        level=logging.DEBUG,
        format='%(levelname).1s %(asctime)s %(message)s')
    main(*FLAGS.parse_args())
