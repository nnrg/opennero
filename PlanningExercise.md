# Extending Planning Methods #

In this exercise, you will implement an extension to the STRIPS planner in OpenNERO.

## Running the Goal-Stack (STRIPS) Agents ##

a) To run the planners with the 3D animations:
Launch the OpenNERO GUI -> Select "Planning" -> Press "Start" -> Select a planner -> Press "Start"

For this assignment you will be concerned with the "Goal Stacking" planners, though you should take a look at the first two options as well (Problem Reduction, State Space Search). Look at the OpenNERO website to understand how to interpret the output of the planners.

Note that the 3 block planner gets stuck in an infinite loop, you will be fixing this as your assignment.

b) To run the strips planner from the command line directly (without the 3D gui), execute the following command in the OpenNERO installation directory:

```
python -m TowerofHanoi.strips TowerofHanoi/towers2_strips.txt
```

This run STRIPS for the 2-disk version of the problem. The file strips.py contains the python code for the strips planner and the file towers2\_strips.txt contains the problem definition for the 2-disk Tower of Hanoi problem. Execute the following command for running 3 disk case:

```
python -m TowerofHanoi.strips TowerofHanoi/towers3_strips.txt
```

## Problem Statement ##

In this assignment, we will be solving a famous planning problem called the Tower of Hanoi. We will be extending the STRIPS planner (also called Goal-Stack planner) to solve this. First, to get an intuition for the problem, run OpenNERO and select the Planning environment as described above.

## Extending STRIPS Planner ##

The STRIPS planner in its current implementation fails to solve the 3-disk problem. STRIPS is linear, i.e. it adds actions to the plan in a linear order as soon as they are found. In many cases there are interactions between the actions such that a later action clobbers a goal that was already achieved earlier. Goal reintroduction allows fixing such a "clobbered" goal again, which makes it possible to solve e.g. the 2-disk problem. However, the planner still fails in the 3-disk case, by getting into an infinite loop of clobbering goals and reintroducting them. Your assignment is to find a solution to this problem.
As a first step to solve this assignment, go through the planning steps printed in output window for the STRIPS planner. Notice, how it solves the 2-disk problem. Now, think about how STRIPS will solve a 3 disk problem and figure out why it ends up into infinite loop.
Once you have understood this infinite loop problem, modify the copy of STRIPS planner in the homework skeleton files (linked below) to fix it.
Now analyze your implementation for the above fix and see whether its a general solution. What are the scenarios under which this implementation can fail (say in a 4 disk problem etc.)?
Explain your solutions for all the parts in the report.
Hints

To fix the STRIPS hang problem in 3 disk case, first understand the function linear\_solver\_helper() in strips.py. See why it sorts the possible actions before picking one of them. Now based on your understanding of the 3 disk hang situation, can you modify the rule for sorting the possible actions to fix the problem. There could be more than one solution to the 3 disk hang problem. I have hinted the one that worked for me. Other methods could involve keeping a history of state-action pairs and using them to select the next action from the possible actions. If you can find a different approach which is neater than the one I described above, you will be eligible for extra credits. In your report, also explain possible scenarios where your suggested fix can fail. It should at least work for 2 and 3 disk cases though.
Homework skeleton files

Start with downloading the skeleton files in [Hw2.tar.gz](http://opennero.googlecode.com/svn/wiki/Hw2.tar.gz). Copy the folder named Hw2 from the archive to your OpenNERO installation folder. At this point, a new mod named Hw2 should show up in OpenNERO whenever you run it. This mod contains two agents named My Planner 2 Disks and My Planner 3 Disks. These agents runs the external copy of STRIPS provided in mystrips.py on the problem definition files towers2\_strips.txt and towers3\_strips.txt. The file mystrips.py is also provided for you inside folder Hw2. Note that the problem definition files in towers2\_strips.txt and towers3\_strips.txt are not duplicated in folder Hw2. Instead, the planning agents run your planner directly on original copies of these files inside folder TowerofHanoi like this:

```
python -m Hw2.mystrips TowerofHanoi/towers2_strips.txt
python -m Hw2.mystrips TowerofHanoi/towers3_strips.txt
```

You can run the above commands inside the OpenNERO installation folder to execute your planner from the command line directly (without the 3D gui.)

The code provided in Hw2/mystrips.py is almost identical to TowerofHanoi/strips.py. Modify Hw2/mystrips.py to implement the assignment.

<a href='Hidden comment: 
In this exercise, you will implement extensions to each of the three planning methods in the [Planning Planning demo] to make them more general and effective.

==Creating Your Agent==

To create your extended planning agent, open the
[http://code.google.com/p/opennero/source/browse/trunk/mods/TowerofHanoi/agent.py
agent.py] file (located in trunk/mods/TowerofHanoi).  Create a new
class called MyPlanningAgent by copying the existing
PlanningAgent.


==Making the Problem Reduction Planner More General==

In the current implementation of the problem reduction planner (class
ProblemReductionAgent) the problem decomposition is hand-coded using
domain knowledge, i.e. the fact that smaller disks need to be moved
out of the way first before a larger disk can be moved.

A more principled approach will be based on means-ends analysis,
i.e. identifying the differences between the current state of the plan
and the goal state, and specifying the operators that reduce those
differences. This analysis takes a form of a table where actions are
the rows and differences are the columns, and entries in the table
cells indicate which operature reduces which difference. The
differences are ordered from the smallest to the largest, so that the
table becomes triangular.

Your assignment is to use this table to decide how to do the problem
decomposition systematically. That is, you identify reducing the largest
difference as one subproblem; before you can reduce it, you have to
satisfy its preconditions, which becomes another subproblem. Then you
have to satisfy the next largest remaining difference, creating
another subproblem, and so on. In the end, this analysis should result
in a problem reduction similar to the one currently coded by hand.

==Generating Optimal Plans with State-Space Search==

The current implementation uses depth-first search, which results in
minimal memory requirements, but requires a depth cutoff. As a result,
it is possible for the planner to find action sequences that find the
goal, but include unnecessary steps.

The solution is to implement iterative deepening in the cutoff,
similar to that in the [HeuristicSearchExercise search exercise]. That is, the
planner first runs with the cutoff of 1, i.e. trying to get to the
goal state in one action. If that is unsuccessful, the cutoff is
increased to 2, and the planner is run again (from scratch). The
cutoff is increased one level at a time until a solution is found. It
is then guaranteed to be minimal (optimal) plan.

==Nonlinear Planning===

The Goal-Stack planner is linear, i.e. it adds actions to the plan in
a linear order as soon as they are found. In many cases there are
interactions between the actions such that a later action clobbers a
goal that was already achieved earlier. Goal reintroduction allows
fixing such a "clobbered" goal again, which makes it possible to solve
e.g. the 2-disk problem. However, the planner still fails in the
3-disk case, by getting into an infinite loop of clobbering goals and
reintroducting them.

Your assignment is to find a solution to this problem. Hint: you can
make the planner nonlinear in that you do not establish an ordering
between the actions unless you determine that one has to precede
another. The planner therefore results in a partially ordered plan; in
the end you establish total order (arbitrarily).

'></a>

## Debugging ##

If you run into any bugs in your program, you can find the error log file for OpenNERO at one of the following locations:

  * **Linux or Mac:** `~/.opennero/nero_log.txt`
  * **Windows:** `"AppData\Local\OpenNERO\nero_log.txt"` or `"Local Settings\Application Data\OpenNERO\nero_log.txt"` depending on the version of Windows you have.