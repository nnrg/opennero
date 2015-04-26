# Running OpenNERO in Headless Mode #

When running experiments, you might want to run OpenNERO multiple times to explore parameter settings or make statistically significant comparisons. When doing so, it is nice to automate this process or even to distribute multiple runs of OpenNERO to a bunch of machines in a cluster. This is how we, for example, ran the [OpenNERO Tournament](TournamentResults2011.md).

# Details #

The basic process is pretty straightforward:
  1. make OpenNERO run your experiment immediately after it starts and record any data you might want later into a log file
  1. write a script that starts OpenNERO in _headless_ mode (without GUI) and stops it after the experiment is completed
  1. either run this script manually or use a cluster system like Condor to run it for you

We will use the OpenNERO tournament runner to illustrate how this is done. See [Running Your Own Tournament](http://code.google.com/p/opennero/wiki/TournamentResults2011#Running_your_own_tournament) for an explanation of what it allows you to do. The scripts used in these examples can be found in [misc/tourney](http://code.google.com/p/opennero/source/browse/#svn%2Ftrunk%2Fmisc%2Ftourney).

## Make OpenNERO start doing stuff ##

In order to make OpenNERO start a battle between two teams of agents immediately after it is loaded, we had to do two things:

### Write a function that will start your experiment ###

In this case we want a function that will take the two teams as arguments and then start a battle between them. We want this function to be available on startup of the NERO\_Battle module, so we added it to [mods/NERO\_Battle/main.py](http://code.google.com/p/opennero/source/browse/trunk/mods/NERO_Battle/main.py):

```
def Match(team0, team1):
    '''Run a single battle between two population files.'''
    mod = module.getMod()
    mod.load_team(team0, constants.OBJECT_TYPE_TEAM_0)
    mod.load_team(team1, constants.OBJECT_TYPE_TEAM_1)
    mod.set_speedup(100)
    OpenNero.enable_ai()
```

This function calls another method, `load_team()`, to load two teams and sets the speedup slider to "full speed" (100%) using the `set_speedup` call. Finally, it calls the `enable_ai()` method to actually start the battle.

### Start OpenNERO with the module you want to run on startup ###

Now we just need to tell OpenNERO to start with the NERO\_Battle module instead of the hub (main menu).

To do this, we need to pass it command line arguments, namely `--mod` and `--modpath`. The first parameter tells OpenNERO which mod to run and the second is a colon-separated list tells it where to look for resources such as images or 3D models. To start in NERO\_Battle, we can use `--mod=NERO_Battle` and `--modpath=NERO_Battle:_NERO:common`.

### Invoke the startup code ###

To actually call the `Match()` function that we wrote earlier, we need to pass the `--command "Match('team1', 'team2')` parameters, where team1 and team2 should be actual teams you want to fight each other.

## Record your data ##

Since we will be running without a graphical user interface (or without anyone watching), we need to record all the relevant information so that we can analyze the results (in this case, the battle score) later. To do this, we can just add print statements inside the python code (NERO\_Battle prints out a line every time the score changes) and then redirect the log output to some file where we will find it later. In this case, we can use something like `--log=battle_between_team1_and_team2.log`.

## Using headless mode ##

Finally, once you tested the previous steps and verified that OpenNERO starts and runs the battle that you want, you can make it run in headless mode by adding the `--headless` parameter. Note that unless your function exits from OpenNERO after some time, you will have to externally kill this process since there is no other way to interact with it, once you are done. Below, we show the complete script, which starts the OpenNERO battle, waits some amount of time, and then closes it.

## Putting it all together ##

The script used to run a single battle between two teams is [condor\_battle.sh](http://code.google.com/p/opennero/source/browse/trunk/misc/tourney/condor_battle.sh):

```
#!/bin/bash

# prepare to run
cd $(dirname $(readlink -f $0))

# start child process
./OpenNERO \
  --log $4 \
  --mod NERO_Battle \
  --modpath NERO_Battle:_NERO:common \
  --headless \
  --command "Match('$1', '$2')" &
OPENNERO_PID=$!
echo STARTED OpenNERO with PID $OPENNERO_PID

# sleep for some time
sleep $3

# kill child process
kill -HUP $OPENNERO_PID
echo KILLED OpenNERO PID $OPENNERO_PID
```

Basically the idea is that we first change into the directory where the script is located, then we start the OpenNERO executable from that directory (remembering its PID, or Process ID), and then wait for, e.g. 5 minutes (by specifying 5m as the third argument to the script). After the time is done, the script kills the OpenNERO process.