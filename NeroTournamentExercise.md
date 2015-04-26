# NERO Machine Learning Tournament #

In this exercise, you will use either the rtNEAT algorithm or
Reinforcement Learning to train a team OpenNero agents to defeat your
opponents in battle. We will run a tournament in class, where we will
pit the teams against each other in an apocalyptic battle for world
domination (or something). Follow the steps below, unless of course
you're comfortable getting destroyed by your opponents who did.

<a href='http://www.cs.utexas.edu/users/risto/pictures/nerogame2.png'><img src='http://www.cs.utexas.edu/users/risto/pictures/nerogame2.png' width='400' height='300' /></a>

## 1. Install prerequisites for running OpenNERO ##

See OpenNeroPrerequisites for detailed instructions for your platform.

## 2. Install the OpenNERO Platform ##

OpenNERO runs on Linux, Mac OSX, and Windows platforms. In order to
install it on your system, you can either:

  * [Download a prebuilt binary](DownloadingOpenNero.md)
  * [Build OpenNERO from the source code](BuildingOpenNero.md)
If you build from the source, you can modify and extend the game
yourself (for the tournament, or otherwise), but no such modifications
are necessary to train and enter the tournament.

## 3. Training Interface ##

To run the training simulator, first start the OpenNERO binary, and choose the Multi-agent Systems Training module from the menu. The NERO simulator will start, showing a training arena, and a separate Training Window will open with several tabs, showing the main tab with command buttons. Click the "New Neuroevolution Team" button to start training a team.

SAVE OFTEN. You don't want to accidentally close something and find that your long-trained team is nowhere to be found. So, after every step, save your team, and take notes of what you're doing and what the results are. Note: Make sure that you give your team name a file extension (.txt is fine), otherwise when you go to load them OpenNERO won't be able to find the file.

TAKE NOTES. Make notes about what behaviors you were trying to train, how close you got, and what versions of the teams you started/finished with. Make your save names descriptive enough so that you can remember what they were after you eat a giant turkey dinner. For your report, you can right click on the graphs and save or copy them for your notes as well. Many web platforms such as Google Docs or Evernote support free note taking with images. These notes will make writing your report a breeze. Your future self will thank you!

TROUBLE? If you should run into trouble or if OpenNero crashes we'd like to know as much about it as possible! Specifically, please save the file ~/.opennero/nero\_log.txt, try to replicate the problem, and try to remember what exactly you did right before.

Most of the training centers around the Training Window, an external Java window that acts as a remote control for the OpenNERO program. The only exception is the Environment Shaping, where you can modify the environment by right-clicking inside the OpenNERO window and using the menus.

### 3a. Adjusting Fitness and Rewards. ###

In the Change Rewards tab, you can adjust the reward components that affect the fitness of the agents. For example, to reward agents that move closer to the flag, slide the "Approach Flag" slider all the way to the right (positive) extreme. If, on the other hand, you want both agents that approach flags and agents that avoid fire, slide those sliders to similar values. You can also adjust the rewards to negative values to punish the agents that score highly in the corresponding dimension.

### 3b. Monitoring Evolution Progress ###

In the Monitor Progress tab, you can see the recent history of the top performing individual (champ) in the various categories they can be rewarded. OpenNERO uses a weighted average of Z-scores formula to combine these fitness components into a single value that reflects the slider preferences. A history of the slider preferences can be seen in the Fitness Weights sub-tab under Monitor Progress.

See something interesting in the graphs? SAVE IT! (right click on the chart and select "save"

## 4. Train your tournament team ##

You're free to use whatever strategy you want to evolve your team, but the steps below seem to be a good idea.

### 4a Evolve flag behavior ###

In sandbox, set the flag slider to "approach" and train your team to approach flags in different places, starting from different spawn locations. Evolve until you're happy.

### 4b Flag around the wall ###

Next, add a wall and evolve until they find a way around. Keep evolving until you're confident that the agents will approach the flag even when walls are in the way.

### 4c Approach Enemy ###

Replace the flag with a static enemy and evolve agents to approach it using the "approach enemy" slider. Once you're happy, add walls like before and evolve agents that can get around them.
4d Approach enemy while firing

Finally, use the sliders for approaching and for hitting the enemy to evolve agents that can do all at once: approach and hit the enemy while avoiding obstacles.

## 5. Assemble your team ##

rtNEAT and RL can come up with many unique solutions to the tasks you give them, and OpenNero agents (even in the same population) can vary quite a bit, both in terms of what they do and how well they perform. It's a good idea to assemble your team using the best individuals from several runs (so your agents solve problems more robustly and so they don't all do the same thing). Here's how to do that.

The team files (or population files, or genome files) that you save are ASCII text files, so you can open them with your favorite text editor such as notepad, emacs or gedit. The basic structure of the file is like this for rtNEAT teams:

```
genomestart 120
trait 1 0.112808 0.000000 0.305447 0.263380 0.991934 0.000000 0.306283 0.855288
...
node 1 1 1 1 FriendRadarSensor 3 90 -90 15 0
...
node 21 1 1 3
...
gene 1 1 22 0.041885 0 1.000000 0.041885 1
...
genomeend 120
```

Each genome also has a corresponding stats entry elsewhere in the file:

The id corresponds to the id of the genome. The fitness corresponds to the cumulative fitness value based on the current Z-score and fitness weights set during training. The raw fitness dimensions are average fitness values in each of the fitness dimensions.

So basically a population consists of one or more genomes. Each genome starts with a genomestart (followed by its ID) line and ends with a genomeend line. Between these lines, there are one or more trait lines followed by one or more input (sensor) lines, followed by some other node lines, followed by the gene lines.

For RL teams, the file looks like this:

```
22 serialization::archive 5 0 0 0.8 0.8 0.1 3 3 ... 1 7 27 OpenNero::TableApproximator 1 0
0 0 0 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 18 0 ...

22 serialization::archive 5 0 0 0.8 0.8 0.1 3 3 ... 1 7 27 OpenNero::TableApproximator 1 0
0 0 0 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 18 0 ...

...
```

Each team member is represented by a bunch of numbers representing the stored Q table for the agent. Unlike rtNEAT teams, RL agents in this file are separated by one blank line.

Either way, you will probably want to pick and choose the individual agents from your training episodes that perform the best for the tasks you anticipate. You should assemble these agents into one file to submit to the tournament.

Please, separate all agents in your submission file with one blank line.

## 6. See how your team performs in an actual game. ##

You don't have to wait until you are done to do this, but you will at least want to have agents that do something before you try (unless you are just curious).

To see a team play an actual game instead of just watching it train load the Multi-agent Systems Battle module instead of Multi-agent Systems Training. You will see a window with the arena, and a dialog box that will let you load a red team and a blue team. You can pit your own previously-saved teams against each other. Don't spend too much time on this, as no training is taking place in battle mode. However this mode will allow you to confirm that you team ats intelligently against opponents that move around, and that the agents that you have choosen work well together.

If you want, you can test how well your team fares against these practice opponents: [team1](http://opennero.googlecode.com/svn/wiki/teams/team1.txt), [team2](http://opennero.googlecode.com/svn/wiki/teams/team2.txt), [team3](http://opennero.googlecode.com/svn/wiki/teams/team3.txt), [team4](http://opennero.googlecode.com/svn/wiki/teams/team4.txt), [team5](http://opennero.googlecode.com/svn/wiki/teams/team5.txt).

## 7. Report ##

Write a detailed report highlighting the strategy/approach you took for training the teams. Clearly explain the reasoning behind the parameter settings at each training step. What methods did you end up using during your training? How did you test your team's behavior? The report will carry significant weight during grade assignment.

## 8. What to do with your finished team ##

Submit the following files:

Your final tournament team file named as follows: team-$USER.txt, where $USER is the username(s) of trainer(s) of the team.

Your final report summarizing what strategy you used and how you proceeded with the training process using neuroevolution (or RL), named as follows: report-$USER.pdf

Your team and your report should be turned in separately.

## 9. Debugging ##

If you run into any bugs in your program, you can find the error log file for OpenNERO at this location:

```
~/.opennero/nero_log.txt 
```

Note that the training window runs as a separate process. If the window is not properly closed before you exit OpenNERO, the program might fail to launch the window again and report that port 8888 is busy. If this happens, you'd need to kill the process manually:

```
$ ps auwx | grep java

user 17604 29.5 1.7 2631232 67944 pts/0 Sl+ 10:34 0:02 java -classpath TrainingWindow.jar edu.utexas.cs.nn.opennero.gui.TrainingUI

$ kill 17604
```










<a href='Hidden comment: 
==3. Submit Your Team to the Tournament==

* The tournament submissions are now closed. We will be evaluating the teams and post detailed tournament results soon!

After you are satisfied with your team, you"ll save it into a file, and submit that file into the tournament.  Note that

* You should test your team first by loading it into NERO_battle and making sure it works as intended.
* You can submit either the team text file itself (fist rename it so that it will have a .txt extension) or a   gzipped version of it (i.e. with the extension .gz). Gzipping is a good idea if the file is large and would take a long time to upload.
* In the "Name" field, give your team a fun, recognizable name: it will be used to identify it in the tournament.
* You can submit multiple teams. Just make sure the teams are different, i.e. trained differently, or consisting of different agents.

==4. Eagerly Await the Results==

We will collect all submissions after the deadline and run a
double-elimination tournament on them. The tournament will be seeded
randomly.  The standard NERO Battle environment will be used for the
games; because the environment is symmetric, each team can be assigned
as Red or Blue randomly without providing an advantage to either one.
Each game will be run until one team is eliminated, or when a timeout
of 5 minutes is reached, in which case the team that has more hits
will win. In case of a tie, the winner will be chosen randomly, except
for the final game, which will be repeated until there is an actual
winner based on hits.

The tournament results will be posted a few days after the deadline on
[TournamentResults2011 this website], including videos of the final games. The teams
will be identified by the name provided in the submission.

==Debugging, Discussion, and Bug Fixes==

If you run into any bugs in your program, you can find the error log file for OpenNERO (see OpenNeroLogs)

We encourage discussion of topics related to the tournament, and have
established [http://groups.google.com/group/opennero OpenNERO Google group] to support it. Questions can be posted in that discussion forum
and answered by anyone on the forum, hopefully making it
easier to figure out issues as they happen.


In order to provide the best experience for everyone, we may release
bug fixes before the tournament submissions are due. Below is a concise summary of such fixes:

====Most recent release: <font color="red">2011-12-14</font> ====

==== Notes for 2011-12-14 ====
* Cosmetic fixes for battle shooting display and heads-up battle speed.

==== Notes for 2011-12-11 ====
* View additional information about reinforcement learning agents, such as fitness, lifetime, hitpoints and agent id.
* Fix assertion at the end of agent lifetime in Maze
* Scoring and reporting tweaks for Battle

==== Recently fixed issues ====
If you are experiencing any of the following, try upgrading to the most recent version:
* <a href="http://code.google.com/p/opennero/issues/detail?id=116">problem saving files

Unknown end tag for &lt;/a&gt;


* <a href="http://code.google.com/p/opennero/issues/detail?id=114">crashing when removing turrets

Unknown end tag for &lt;/a&gt;


* <a href="http://code.google.com/p/opennero/issues/detail?id=124">Q-learning agents not loading properly

Unknown end tag for &lt;/a&gt;


* battle never ends
* agents removed from battle continue to accumulate damage
* <a href="http://code.google.com/p/opennero/issues/detail?id=128">rtNEAT sliders disabled when loading team for further training

Unknown end tag for &lt;/a&gt;



'></a>


#### Known issues ####
  * See [issues list](http://code.google.com/p/opennero/issues/list) for a general list of known issues