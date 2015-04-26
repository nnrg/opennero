# The Maze Environment for Search #

The Maze Environment is a 2-D grid world embedded in a 3-D simulation. Grid locations
correspond to potential intersections in the maze; the maze is
randomly generated and contains a single shortest path to the goal, no
loops. Various search and learning agents are implemented in the Maze
environment, as well as a first-person search setting.

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-maze.png' height='300' width='400></img'>

<h2>What the display means</h2>

<ul><li>Red cube - goal position (opposite of the starting position)<br>
</li><li>Yellow marker - next location the agent is going to<br>
</li><li>Blue marker - past locations the agent has already expanded, i.e. whose successors have already been generated<br>
</li><li>Green markers - generated (but not yet expanded) locations the agent may return to<br>
</li><li>White markers - found path</li></ul>

<h2>Controls</h2>

The controls can be easily redefined, but in general, the following keys should work:<br>
<br>
<ul><li><b>F1</b> - help (opens the browser to show this page)<br>
</li><li><b>A</b> - move camera left<br>
</li><li><b>D</b> - move camera right<br>
</li><li><b>W</b> - move camera forward<br>
</li><li><b>S</b> - move camera back<br>
</li><li><b>Q</b> - pan camera left<br>
</li><li><b>E</b> - pan camera right<br>
</li><li><b>R</b> - tilt camera up<br>
</li><li><b>F</b> - tilt camera down<br>
</li><li><b>space bar</b> - recenter camera to origin<br>
</li><li><b>ESC</b> - exit the currently running mod<br>
</li><li><b>Mouse Scroll</b> - zoom in or zoom out<br>
</li><li><b>Z</b> - zoom in<br>
</li><li><b>C</b> - zoom out</li></ul>

<h2>User Interface</h2>

The pull-down menu lists the different types of agents available for the Maze:<br>
<ul><li><b>Depth First Search</b> - starts the depth first search agent<br>
</li><li><b>Breadth First Search</b> - starts the breadth first search agent<br>
</li><li><b><code>A*</code> search</b> with three different types of visualizations<br>
<ul><li>Single Agent <code>A*</code> Search - the agent has to navigate through the maze both to make progress and to back-track to move on<br>
</li><li>Teleporting <code>A*</code> Search - the agent can search for solutions faster by teleporting to the next open node instead of having to backtrack<br>
</li><li>Front <code>A*</code> Search - the agent now has the ability to produce several new agents when faced with different alternatives. These agents are marking the front of the search.<br>
</li></ul></li><li><b>First Person Control - use the arrow keys to try to solve the maze yourself!</li></ul></b>

The control panel includes additional controls:<br>
<ul><li><b>The Exploit/Explore Slider</b> - this is applicable only to the learning methods such as Sarsa and Q-Learning. Because these methods start out knowing nothing about the best actions and learn from experience, they face an <i>exploration-exploitation trade-off</i> during learning, where they have to decide how much of the time to do the best thing they know how to do (exploit) and how much of the time to try to seek new experience (explore). The exploit/explore slider lets you make this decision for them - side it to the right to encourage exploration and slide it to the left to see what the best learned policy so far looks like. The slider will appear as soon as you start running Q-learning.<br>
</li><li><b>The Speedup Slider</b> - this slider controls another tradeoff: one between displaying the simulation slowly enough to see robot animations and movements from cell to cell, and as quickly as the computer running OpenNERO can handle it. The speedup slider is again particularly useful when running the learning agents, because they may require a large amount of experience before finding the optimal path through the maze. To progress through the learning faster, slide the Speedup slider to the right.<br>
</li><li><b>The Starting Offset Slider</b> - By default, the search agent starts at the lower left corner of the maze.  Using this slider, you can have the agent start at a location that is closer to the target.  For example, if you set the starting offset to 8, the agent will start at a random cell whose Manhattan distance from the lower left corner is equal to 8.<br>
</li><li><b>Generate New Maze Button</b> - this button allows you to mix things up by generating a new random maze. Some mazes take longer than others, and some are more suited to particular search techniques.<br>
</li><li><b>Pause/Continue</b> - Pause will temporarily suspend the execution of the algorithm; the button changes to <b>Continue</b> and hitting it will resume execution.<br>
</li><li><b>Start/Reset</b> - Start will begin running the selected algorithm; the button changes to Reset and hitting it will terminate the algorithm.<br>
</li><li><b>Help</b> - will get you to this page.