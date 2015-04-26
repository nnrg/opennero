## Introduction ##

Planning means deciding on a course of action before acting. In this
demo, three different planning methods (goal-stacking, forward
search, and problem reduction) are demonstrated in the Towers of Hanoi
task. The planning process is first run in the background,
illustrating its progress in a text window. The Steve robot then
executes the completed plan in the environment.  Watch the videos
below to see how the planning demo works, or run OpenNERO yourself to
try it out interactively.

## Running the Demo ##

To run the demo, first:

  1. [Start OpenNERO](RunningOpenNero.md)
  1. Start the Planning Experiment
  1. Select the method you want to run from the pull-down menu:
    1. Goal-stacking runs a STRIPS-style planner on 2-3 disks
    1. Forward search runs a state-space search planner on 2-4 disks
    1. Problem-reduction search runs a top-down search on 2-5 disks
  1. Click on the Start button to start the demo

Upon Start, a text window pops up to show the progress of the planner.
Pressing the "Step" button in this window allows proceeding through
the planning process one step at a time; pressing the "Run" button
runs the process until the end without interruption. The final plan is
printed at the end of the text window

This plan is then given to Steve to execute. Press "Pause" to
temporarily pause its execution, and "Continue" to resume.

## Tower of Hanoi ##

Somewhere in Hanoi there's a monastery with a courtyard with three
tall poles and 64 disks (each with a different size and with a hole in
the middle) on one of the poles. The monks have a peculiar tasks: they
have to move the disks from the original pole to the target pole. They
can move only one disk at a time, they can place each disk only on top
of a larger disk, on any of the poles (but not on the ground). Once
they finish the task, the world will end...(but it will take a long
time so no need to worry!).

Being clever AI programmers, the monks implemented three methods for
planning their work ahead of time. In this demo you can test these
methods with a small number of disks, and you can evaluate how well
they might scale up to the full task.

In all three methods, the initial state is expressed in a logical form
as

```
(ON disk1 pole1) &
(ON disk2 disk1) &
(ON disk3 disk2) &
(CLEAR disk3) &
(CLEAR pole2) &
(CLEAR pole3)
```

and the goal state as
```
(ON disk1 pole3) &
(ON disk2 disk1) &
(ON disk3 disk2)
```

The actions are defined through a set of preconditions and
postconditions, e.g.
```
(MOVE disk1 x y)
Preconditions:  (CLEAR disk1) & (ON disk1 x) & (CLEAR y) & (< disk1 y)
Postconditions:
add (CLEAR x), (ON disk1 y)
delete (CLEAR y), (ON disk1 x)
```

Planning consists of finding a sequence of actions that transforms the
initial state to the goal state. Three methods for doing so are
outlined below.


## Goal-Stacking Planner ##

The goal-stacking (i.e. STRIPS-like) planner is	a simple linear
solver, i.e. a total order solver that assumes all steps can be
planned one after the other with a strict ordering.  It works
backwards by starting at the goal state and searching for a grounded
action (one with all variables replaced with literals) that will
satisfy a precondition of the goal state. When it finds an acceptable
action, it adds it to the plan and the preconditions of the newly
added action are added to the set of goals. If an action is added that
conflicts with another already-satisfied goal, then the clobbered goal
is reintroduced and resatisfied. The algorithm keeps working
backwards, stacking subgoals on top of each other via a depth-first
search until it either satisfies all the goals or exhausts all
possible plans and gives up.

This approach allows solving the 2-disk task, but it fails on three or more disks (it gets into an infinite loop; see "Next Steps"). One way to solve it is to consider all potential plans systematically, as is done in state-space search.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=wQ_GVEl0QZk' target='_blank'><img src='http://img.youtube.com/vi/wQ_GVEl0QZk/0.jpg' width='425' height=344 /></a>


## State-Space Search ##

In State-Space search, the planner searches from the starting state
forward by trying out all actions systematically in turn, in a
depth-first fashion.  The depth cutoff is set at eight, i.e. if the
planner has not solved the task in eight actions, it will backtrack
and try alternatives. Eight steps are enough to solve the 4-disk
problem.

Associated with this method there is a 2-D visualization window that
shows what the planner is currently thinking, i.e. the current
sequence that it is currently exploring. As you can see, this planner
spends a lot of time backtracking near the cutoff, and in general, it
is a very inefficient method. Also, since it stops as soon as the goal
is achieved, it may come up with a plan with extra actions. The
problem reduction search is more effective in this case.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=fIhBxcaJQt0' target='_blank'><img src='http://img.youtube.com/vi/fIhBxcaJQt0/0.jpg' width='425' height=344 /></a>



## Problem Reduction ##

In Problem Reduction, the planner executes a top-down planning
process. It starts with the high-level goal and decomposes it into
smaller goals that are then solved through a recursive call to the
planner. Therefore, it uses domain knowledge to structure the search,
which results in a more effective and scalable planning process. The
plan is found with minimal backtracking, the plans are optimal, and
scale up to large numbers of disks (upto 5 are shown in this demo).

<a href='http://www.youtube.com/watch?feature=player_embedded&v=THaH9kBve2c' target='_blank'><img src='http://img.youtube.com/vi/THaH9kBve2c/0.jpg' width='425' height=344 /></a>

<a href='Hidden comment: 
==Source Code==

The source code for the planning methods is in [http://code.google.com/p/opennero/source/browse/branches/towers3/mods/BlocksTower/agent.py agent.py]. The goal-stack planner is implemented in the TowerAgent3 class, state-space search in the TowerAgent2 class, and problem reduction planner in the TowerAgent class. See SystemOverview for a general description of how agents work.

The planning methods can also be run manually. To run the problem reduction algorithm, use python recursive_solver.py. To run the goal reduction technique, use python strips.py towers2_strips.txt (you can also specify one of the other STRIPS problems available in the mod, but be careful: the current implementation gets into an infinite loop for towers3! To run the state-space search technique, use python state_space_search.py.

The planning problem is defined formally for both the goal reduction and the state space search technique. The goal reduction implementation uses specially formatted text files such as [http://code.google.com/p/opennero/source/browse/branches/towers3/mods/BlocksTower/towers2_strips.txt towers2_strips.txt]. The state space search uses Python objects and functions to define the planning problem (see for example [http://code.google.com/p/opennero/source/browse/branches/towers3/mods/BlocksTower/towers3.py towers3.py]).

'></a>

## Next Steps ##

While the problem-reduction planner is effective in this domain, it is
implemented in a rather domain-dependent way. A more general
formulation would incorporate means-ends analysis of the problem,
making it possible to identify possible decompositions automatically.

The state-space search is a very general method, but it wastes a lot
of time blindly considering alternatives. The current implementation
also produces suboptimal plans, i.e. those with unnecessary actions in
them. One way to solve this problem is to use iterative deepening of
the cutoff.

While goal reintroduction works in the two-disk case, it fails with
three disks: the planner gets into an infinite loop. The problem is
that the planner commits to an order of actions before it knows that
it will work. The way to solve it is to make the planner nonlinear,
i.e. avoid ordering of actions until necessary.

See [Planning Exercise page](PlanningExercise.md) for a more concrete
assignments of these extensions.