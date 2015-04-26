# Tower of Hanoi Environment #

Blocks World is a classical test problem for traditional AI techniques such as symbolic planning, natural language understanding, and even robotics. The idea is to manipulate simple objects (blocks) that can be moved around and stacked on top of each other. While it is a relatively simple domain, it hides some surprising challenges and provides a good illustration for some fundamental principles of AI.

Currently the "Towers of Hanoi" problem is implemented in the Blocks
World, and it is used to demonstrate three different planning methods.

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-blocks.png' height='300' width='400></img'>

<h2>What the display means</h2>

In the Planning demo:<br>
<ul><li>The nearest pole - starting location for the disks<br>
</li><li>The furthest pole - goal location for the disks<br>
</li><li>The middle pole - temporary storage for the disks<br>
</li><li>Size of disks - indicates which disk can be placed on which other disks (smaller on larger)</li></ul>

The text window<br>
<ul><li>text - log output of the planning process, with final plan in the end<br>
</li><li>Next Stop - advance the planning algorithm to its next logical step<br>
</li><li>Skip Rest - skip the rest of the pauses and run the algorithm to completion<br>
</li><li>Close Window - close the window to get back to OpenNERO and execute the resulting plan</li></ul>

<h2>Controls</h2>

The keyboard controls allow moving around in the environment:<br>
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

There are three agent type buttons that start the demonstration of the three planning algorithms:<br>
<ul><li><b>Problem reduction</b> - Hierarchical planning through problem decomposition<br>
</li><li><b>Goal Stacking</b> - STRIPS-like linear search (depth first, with goal reintroduction)<br>
</li><li><b>State-space search</b> - Systematic forward search (depth first)