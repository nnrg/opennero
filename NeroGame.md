## Introduction ##

In the NERO game, the user trains intelligent agents to perform well
in battle.  It is a machine learning game, i.e. the focus is on
designing a set of challenges that allow agents to learn the necessary
skills step by step. Learning takes place in real time, as the user is
observing the game and changing the environment and behavioral
objectives on the fly. The challenge for the player is to develop as
proficient a team as possible.

The NERO game in OpenNERO is a simpler research and education version
of the original [NERO](http://nerogame.org) game, focusing on
demonstrating learning algorithms interactively in order to make it
clear how they work. The game environment is first described below,
then the two methods for training the agents (neuroevolution and
reinforcement learning), how a team can be put together for battle,
and then the battle mode itself.  Ways of extending the learning
methods and handcoding the teams, as well as differences from the
original NERO are described in the end. To get a quick introduction to NERO, watch the video below.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=1RRGGIFOU04' target='_blank'><img src='http://img.youtube.com/vi/1RRGGIFOU04/0.jpg' width='425' height=344 /></a>

## NERO Training ##

In the training mode, the user selects one of the two training methods
(neuroevolution or reinforcement learning) and manipulates the
environment and the behavioral goals in order to train them to do what
s/he wants.

Typically the training starts by deploying either an rtNEAT team or Q-learning team, and then setting some of the goals (or fitness coefficients) in the parameter window (the sliders become active after a Deploy button is pressed). They are:

  * Stand Ground:
    * Positive: Punished for nonzero movement velocity
    * Negative: Rewarded for nonzero movement velocity
  * Stick Together
    * Positive: Rewarded for small distance to center of mass of teammates.
    * Negative: Rewarded for large distance to center of mass of teammates.
  * Approach Enemy:
    * Positive: Rewarded for small distance to closest enemy agent
    * Negative: Rewarded for large distance to closest enemy agent
  * Approach Flag:
    * Positive: Rewarded for small distance to flag
    * Negative: Rewarded for large distance to flag
  * Hit Target:
    * Positive: Rewarded for hitting enemy agents
    * Negative: Punished for hitting enemy agents
  * Avoid Fire:
    * Positive: Punished for having hit points reduced
    * Negative: Rewarded for having hit points reduced

There are also a number of parameters that effect learning that should
be set appropriately (the default values usually are a good starting
point):

  * Explore/Exploit: This slider has no effect on neuroevolution. With Reinforcement Learning, it determines the percentage of actions taken greedily (i.e. those with the best Q-values) vs. actions taken to explore the environment (i.e. randomly selected actions)
  * Lifetime: The number of action steps each agent gets to perform before being removed from the simulation (and restarted, or replaced by offspring).
  * Hitpoints: The amount of damage an agent can take before dying (Note: being hit by an enemy is 1 point of damage). In training, the agent is removed from where it is and respawned at the spawn location with lifetime and hitpoints reset.

The second part of the initialization is to set up the environment. An
initial environment is already provided, and it is the same as the
battle environment. The user can, however, add objects to it to
design the training curriculum, through right clicking with the mouse:

When right clicking on empty space:
![http://www.cs.utexas.edu/users/risto/pictures/menu1.png](http://www.cs.utexas.edu/users/risto/pictures/menu1.png)
  * Add Wall: Generates a standard wall where you clicked
  * Place Flag: Generates a flag to the place where you clicked, or moves the flag there if one already exists. The flag has the appearance of a blue pole. The flags are useful for demonstration purposes, but not necessary in training for battle.
  * Place Turret: Generates an enemy at a location where you clicked. The enemy rotates and fires at anything in its line-of-fire, with the same probabilistic method as the agents themselves. It does not die no matter how many times it is hit.
  * Set Spawn location: moves the location around which the agents are created to the location where you clicked. The locations and orientations of the agents are randomly choces within a small circle around that point. The team is blue in training; in battle there are red and blue teams.

When right clicking on an object (i.e. a wall or a turret) that you placed:
![http://www.cs.utexas.edu/users/risto/pictures/menu2.png](http://www.cs.utexas.edu/users/risto/pictures/menu2.png)
  * Rotate Object: Rotates the object around the z-axis until the user left clicks.
  * Scale Object: Scales the object until the user left clicks.
  * Move Object: Lets you move the object until you left click.
  * Remove Object: Removes the object

The trees are sensed as small walls; in the current version they
cannot be created or modified though.

#### Over-head display ####

By hitting the **F2** key, you can cycle through additional information about
each agent that may be useful during training. This "over-head" display
shows up as a bit of text above each agent on the field. When an over-head
display is active, the window title will change to say what is being
displayed. Some of information is specific to Neuroevolution, and some is
specific to RL.

  * fitness
    * for RL, this is the cumulative reward over the agent's lifetime. Because the meaning of the reward values can change with the adjustment of sliders, the exact meaning and units of this value depend on the current slider setting.
    * for rtNEAT, this is the **relative** fitness of the organism compared to the rest of the population. This is calculated as the weighted sum of the Z-scores (the number of standard deviations above or below population average) of the agent in each of the fitness slider categories.
  * time alive
    * for RL, this is simply the number of steps on the field that the current individual has been trained for. Generally, the longer an agent is fielded, the more experience it has, and, if using a hash table, the larger its representation in the team file.
    * for rtNEAT, this is the total time (in frames) that the phenotype has been on the field. Note that this can be larger than a single lifetime because the same network can be "re-spawned" several times if it is considered good enough, because rtNEAT is an elitist steady-state algorithm.
  * id
    * for RL, this is the **body id** of the individual, allowing you to keep track of its behavior over time.
    * for rtNEAT, this is the **genome id**, which you can use to track the behavior and to extract individuals from saved populations for use in combat teams.
  * species id
    * for RL, this shows the value 'q' for the default q-learner to allow you to distinguish RL agents from rtNEAT ones.
    * for rtNEAT, this shows the unique species number that the individual belongs to. rtNEAT uses speciation and fitness sharing in order to protect diversity within the evolving population.
  * champion:
    * not available for RL
    * for rtNEAT, this shows the label 'champ!' above the highest-ranked individual within the current population, allowing you to quickly check what the best behavior so far is according to the current fitness profile.


### Neuroevolution (rtNEAT) ###

The rtNEAT neuroevolution algorithm is a method for evolving (through
genetic algorithms) a population of neural networks to control the
agents. See the [paper on rtNEAT](http://nn.cs.utexas.edu/?stanley:ieeetec05) for more details.

When you press the "Deploy rtNEAT" button, a population of 50 agents
is created and spawned on the field. Each agent is controlled by a
simply neural network connecting the input sensors directly to
outputs, with random weights. Over their lifetime, fitness is
accumulated based on the behavior objectives specified with the
sliders: if e.g. the approach enemy is rewarded, the time they spend
near the enemy is multiplied by a large constant and added to the
fitness.

After their lifetime expires, they are removed from the field one at a
time. If their fitness was low, they are simply discarded. If their
fitness was high, they will be put back into the field, and in
addition, a new agent is generated by mutating the neural network
(i.e. adding nodes and connections and/or changing connection weights)
and crossing over its representation with another network with a high
fitness. A balance of about 50% new individuals and 50% repeats is
maintained in the field in the steady state (the explore/exploit
slider has no effect on evolution).  In this manner, evolution is
running incrementally in the background, constantly evaluating and
reproducing individuals.

Over time, evolution is thus likely to come up with more complex
networks, including those with recurrent connections. Recurrency is
useful e.g. when an agent needs to pursue an enemy around the corner
(i.e. even though the enemy disappeared from view, activation in a
recurrent network will retain that information). In other word, it
allows disambiguating the state in a POMDP problem (where the state is
partially observable).

When the population is saved, the genomes of each agents are written
into a text file. That file can be edited to form composite teams,
reloaded for further training, or loaded into battle.

The rtNEAT algorithm is parameterized using the file [neat-params.dat](http://code.google.com/p/opennero/source/browse/trunk/mods/_NERO/data/ai/neat-params.dat); you can edit
it in order to experiment with different versions of the method (such
as mutation and speciation rates, balance of old and new agents, etc.)


## Reinforcement Learning (Q-learning) ##

The reinforcement learning method in NERO is a version of Q-learning
(familiar from the [Q-learning demo](QLearning.md)), using either static, linear discretization or a tile-coding function approximator. The agents learn during their
lifetime to optimize the behavioral objectives.

When you press the "Deploy Q-learning" button, a Q-learning agent is
created according to the specs in the file `mods/_NERO/data/shapes/character/steve_blue_qlearning.xml`. The `<Python agent="NERO.agent.QLearningAgent()">` XML element can be changed to include keyword arguments that will be passed to the QLearning constructor. These parameters are:
  * **gamma** - reinforcement learning discount factor
  * **alpha** - learning rate
  * **epsilon** - exploration factor for epsilon-greedy action selection, note that this can also be changed during the NERO simulation by manipulating the "Exploit/Explore" slider
  * **action\_bins** - discretize each continuous dimension in the action space into this many linear bins (there are 2 action dimensions in NERO: turning and moving)
  * **state\_bins** - discretize each continuous dimension in the state space into this many linear bins (there are about 15 state dimensions in NERO)
  * **num\_tiles** - the number of tiles you want to use in the tile-coding approximator
  * **num\_weights** - the amount of memory reserved for storing function approximations in the tile-coding approximator

The last four parameters specify the
discretization of the state and action dimensions so that the agent's
state can be represented as a discretized table of Q-values, one for each
state/action pair (these values are initialized to zero). If you choose to use the tile-coding approximator, be sure to set **action\_bins** and **state\_bins** to 0; conversely, if you wish to use the static bins, be sure to set **num\_tiles** and **num\_weights** to 0. The default Q-Learning agents are created with **action\_bins** set to 3 and **state\_bins** set to 5.

The population for the game is generated by cloning this agent 50
times; each agent gets its Q-table to update, so different agents can learn different Q-values depending on their experiences.

Q-learning progresses as usual during the lifetime of these
individuals, modifying the values in the table. Using the
Exploit-Explore slider you can adjust the fraction of the actions
taken greedily (i.e. those with the best Q-values) vs. actions taken
to explore the environment (i.e. randomly selected actions).  When the
lifetime of an agent expires, it is respawned, and continues from the
spawn location with its current Q-tables.

When the population is saved, the Q-tables of each individual are
saved together with its parameters and the function approximation
parameters, so that they can be loaded for further training and
battle.

## Training strategy ##

The game consists in trying to come up with a sequence of increasingly
demanding goals, so that the agents will perform well in the end. It
is a good idea to start with something simple, such as approaching the
enemy. Once the agents learn that, place the enemy behind a wall so
they learn to go around it. Then reward the agents for hitting the enemy as well. Then start penalizing them for getting hit.  Introduce more enemies, and walls behind which the agents can take cover. You can also explore the effects of staying close or apart from teammates, and standing ground or moving a lot. In this manner, you can create agents with distinctly different personalities, and thus possibly serving different roles in battle.

Achieving each objective will take some time. Within a couple of
minutes you should see some of the agents perform the task sometimes;
within 10-15 minutes, almost the entire team may converge. Using the
F2 displays you can follow the behavior of the current champion, which
agents are drawing fire and which are avoiding it, and with rtNEAT,
observe which agents are new and which are old, and how speciation is
progressing.  Note that it is not always good to converge completely,
because it may be difficult to learn new skills then. The trick is to
discover a sequence where later skills build on earlier ones so that
little has to be unlearned between them.

It is a good idea to train several teams, and then test them in the
battle mode. In this manner, you can develop an understanding of what
works and why, and can focus your training better. Based on that
knowledge you can also decide how to put a good team together from
several different trained teams, as will be described next.


## Composing a Team for Battle ##

Note that you can train several different teams to perform different
behaviors, for instance a team of attackers, defenders, snipers,
etc. It may then be useful to combine agents with such different
behaviors into a single team. Because the save files are simply text,
you can form such composite teams simply by editing them by hand. You
can also "clone" agents by copying them multiple times. You can even
combine agents created by neuroevolution and reinforment learning into
a single team. The first 50 in the save file will be used in the
battle; if there are fewer than 50 agents in the file, they will be
copied until 50 are created in battle.

The basic structure of the file is like this for rtNEAT teams:

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

<p>In words, a population consists of one or more genomes. Each genome<br>
starts with a <strong>genomestart</strong> (followed by its ID) line<br>
and ends with a <strong>genomeend</strong> line. Between these lines,<br>
there are one or more trait lines followed by one or more input<br>
(sensor) lines, followed by some other node lines, followed by the<br>
gene lines.<br>
<br>
<p>For RL teams, the file looks like this:<br>
<br>
<pre><code>22 serialization::archive 5 0 0 0.8 0.8 0.1 3 3 ... 1 7 27 OpenNero::TableApproximator 1 0<br>
0 0 0 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 18 0 ...<br>
<br>
22 serialization::archive 5 0 0 0.8 0.8 0.1 3 3 ... 1 7 27 OpenNero::TableApproximator 1 0<br>
0 0 0 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 18 0 ...<br>
<br>
...<br>
</code></pre>

<p>Each team member is represented by a bunch of numbers representing<br>
the stored Q table for the agent. Unlike rtNEAT teams, RL agents in<br>
this file are separated by one blank line.<br>
<br>
Either way, you will probably want to pick and choose the<br>
individual agents from your training episodes that perform the best<br>
for the tasks you anticipate. You should assemble these agents into<br>
one file for the battle.<br>
<br>
(Note: If you include reinforcement learning agents, you need to separate all agents in your submission file with one blank line. Also note: if you form a team by combining individuals from different rtNEAT runs, you current cannot train such a combo team further (because rtNEAT training depends on historical markings that then would not match)).<br>
<br>
Before you submit to the tournament, you should test your file by loading it into NERO_Battle and making sure it runs correctly.<br>
If you want, you can test your team e.g. against this <a href='http://opennero.googlecode.com/svn/wiki/SampleTeam.pop'>sample team</a>.<br>
<br>
<h2>NERO Battle</h2>

In the NERO-battle environment the user first loads the two teams: one<br>
is identified as Red and the other as Blue based on how the top of<br>
the head of the robots is painted. By default they spawn on the opposite<br>
sides of the central wall in the standard environment (the environment<br>
and the spawn locations can be changed as in training mode).<br>
<br>
The Hitpoints slider specifies how many times each agent can be hit<br>
before it dies and is removed from the battle.  The game ends when one<br>
team is completely eliminated or when the time runs out, in which case the team that has more hits on the opponent wins. The current hitpoints are displayed in the title bar of the NERO window; the agent that delivered the winning shot will jump up and down in jubilation :-).<br>
<br>
The game starts when the user presses the Continue button. The agents<br>
are spawned only once, and they then have to move around in the<br>
environment and engage the other team. This is where the training pays<br>
off: the agents need to respond appropriately to the opponents'<br>
actions, emploing different skills in different situations, such as<br>
attacking, retreating, sniping, ambushing, sometimes perhaps working<br>
together with teammates and sometimes independently of them. There is<br>
no a-priori winning strategy; the performance of the team depends on<br>
the ingenuity of its creator!<br>
<br>
To see how the battle mode works, or see how well your team is doing, you can use this <a href='http://opennero.googlecode.com/svn/wiki/SampleTeam.pop'>sample team</a>.<br>
<br>
<h2>NERO Tournament</h2>

A fun event in e.g. AI or machine learning courses is to organize a<br>
NERO tournament. The students develop teams, and the teams are then<br>
played against each other in a round-robin or a double-elimination<br>
tournament. One such tournament was held in Fall 2011 for the Stanford<br>
Online AI course; the tournament assignment is <a href='NeroTournamentExercise.md'>here</a>.<br>
<br>
<br>
<h2>Extending NERO Methods</h2>

The ingenuity is not limited to simply training the agents with the<br>
methods that have been implemented in OpenNERO. The game is open<br>
source, and you can modify all aspects of it by changing the python<br>
code (and in some case, the C++ code). The main files are...<br>
<br>
For instance, you can implement more sophisticated versions of the<br>
sensors and effectors, or entirely new ones such as line-of-fire<br>
sensors, or sending and receiving signals between the agents. You can<br>
implement more sophisticated function approximators for reinforcement<br>
learning, and even other neuroevolution and reinforcement learning<br>
algorithms. If you so desire, you can also program the agent behaviors<br>
entirely by hand.<br>
<br>
Note that many such changes will require making corresponding changes<br>
into the battle mode as well, and therefore it will not be possible to<br>
use them in the <a href='NeroTournamentExercise.md'>NERO Tournament</a>. However,<br>
note that as long as your team is represented in terms of genomes and<br>
Q-tables, it doesn't matter how that representation is created. That<br>
is, if your changes apply to training only, and your team can still be<br>
saved in the existing format, the team can be entered into the<br>
tournament. For instance, you can express behaviors in terms of rules<br>
and finite state automata based on the sensors and effectors in NERO,<br>
and then mechanically translate them into neural networks (see<br>
e.g. <a href='http://nn.cs.utexas.edu/?yong:aiide06'>this paper</a>). Those networks can then be represented as a genome and entered into tournament.<br>
<br>
<h2>Differences between OpenNERO and Original NERO</h2>

The NERO game in OpenNERO differs from the original NERO game in<br>
several important ways. First of all, whereas the original NERO was<br>
based on the Torque game engine, OpenNERO is entirely open source<br>
(based on the Irrlicht game engine and many other open-source<br>
components).  This design makes it a good platform for research and<br>
education, i.e. it is possible for the users to extend it and to<br>
understand it fully.<br>
<br>
Second, the original NERO was designed to demonstrate that machine<br>
learning games can be viable. It therefore aimed to be a more<br>
substantial game, and included many features such as more advanced<br>
graphics, sound, and user interface, as well as more detailed<br>
environments that made gameplay more enjoyable. The 2.0 version of<br>
NERO also included interactive battle where the human players<br>
specified targets and composed teams dynamically.<br>
<br>
Third, OpenNERO includes reinforcement learning as an alternative<br>
method of learning for NERO agents. The idea is to demonstrate more<br>
broadly how learning can take place in intelligent agents, both for<br>
research and education.<br>
<br>
Fourth, the original NERO included several features that have not yet<br>
been implemented in OpenNERO, but could in the future. They include a<br>
sensor for line-of-fire (which may help develop more sophisticated<br>
behaviors); taking friendly fire into account; collisions among NERO<br>
agents; different types of turrets in training; a button that<br>
converges a population to a single individual, and a button that<br>
removes an individual from the population. We invite the users to<br>
implement such features, and perhaps others, in the game, and<br>
contributed them to OpenNERO!<br>
<br>
Fifth, much of OpenNERO is written in Python (instead of C++), making<br>
it easier to understand and modify, again supporting research and<br>
education. Unfortunately, it has the result of slowing down the<br>
simulation by an order of magnitude. However, we believe that<br>
researchers and students have the patience it takes to "play"<br>
OpenNERO, in order to gain the better insight into the learning in it.<br>
<br>
<h2>Software Issues</h2>

OpenNERO is academic software and under (hyper)active development. It<br>
is possible that you will come across a bug in it, or a feature that<br>
should be implemented. If so, please report it <a href='http://code.google.com/p/opennero/issues/list'>here</a>, so that<br>
everyone can see it and track it (please first check whether it has<br>
already been reported).