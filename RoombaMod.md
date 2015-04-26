# Roomba Mod #

The Roomba environment is a virtual computer lab with crumbs distributed on the floor. The agents collect the crumbs and receive reward for doing so.

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-roomba.png' height='300' width='400></img'>

<h2>Controls</h2>

<ul><li><b>Esc</b> - exit the Maze and return to the Hub<br>
</li><li><b>F1</b> - get help by opening the web browser and navigating to this page<br>
</li><li><b>A</b> - move camera left<br>
</li><li><b>D</b> - move camera right<br>
</li><li><b>W</b> - move camera forward<br>
</li><li><b>S</b> - move camera back<br>
</li><li><b>Q</b> - pan camera left<br>
</li><li><b>E</b> - pan camera right<br>
</li><li><b>space bar</b> - recenter camera to origin</li></ul>

<h2>What the display means</h2>

<ul><li>Blue cubes: bread crumbs (or pieces of trash) on the floor, which the vacuum cleaners must pick up<br>
</li><li>Grey cylinders: vacuum cleaner agents<br>
</li><li>Walls: the limits of the environment<br>
</li><li>Tables, Chairs, Computers: at the moment these are only decorations</li></ul>

<h2>User Interface</h2>

The control panel allows you to select the number of agents to run, as well as the policy used to control them. You can select a scripted agent that greedily selects the closest crumb and moves to collect it, or an evolving agent that improves with time.<br>
<br>
<h2>Some functionality provided by the scripting language</h2>
Changing Blue cube distribution:<br>
<ul><li><a href='http://code.google.com/p/opennero/source/browse/trunk/mods/Roomba/world_config.txt'>world_config.txt</a> <b>can</b> hold all information needed to distribute the Blue cubes. You can think of this file as a very rudimentary "map" or "scenario," as the term is used in strategy games.<br>
<ul><li>The first few lines under Dimensions list the sizes of the room. However, these are only filler at the moment.<br>
</li><li>Each line under "Pellets" hold information about one Blue cube. A typical line may look like this:<br>
<code>1	cluster	86.8597775308	20.5101799601	7.31971084367	7.31971084367</code>
</li><li>The first number ("1") is the amount of reward the robot gets for picking up this particular Blue cube<br>
</li><li>"cluster" specify the kind of distribution that this Blue cube belong to. In the case of "cluster", this cube will be placed randomly but close to a certain point. If we have bunch of cubes with identical information, they will "cluster" together, thus the name. The other options for this field are "manual" and "random" (uniform randomization).<br>
</li><li>The numbers that follow are other pieces of information used to generate the position of this Blue cube. Here, from left to right, we have the x and y coordinate of the cluster center, and about 1/3 of the x and y spread, respectively.</li></ul></li></ul>

<h2>Scripted Roomba</h2>

In RoombaMod, one simple strategy is the greedy vacuum cleaner that detects and drives to the nearest piece of litter. In order to start this demonstration, first select the Roomba mod from the main menu and then <b>Switch to Script</b> and <b>Add Bots</b>.<br>
<br>
There are a number of strategies that a student can use to write a script for the Roomba robot. These range from simple greedy solutions that always work to scripts that use some knowledge about e.g. the shape of the room or the distribution of the dirt in it. They of course also include learning methods such as RL or rtNEAT, but the difference is that those methods can <i>improve with experience</i>.