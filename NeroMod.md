# NERO Environment #

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-nero.png' height='300' width='400></img'>

The player first enters the NERO-Training environment, where s/he<br>
develops a team and saves it. The player then enters the NERO-Battle<br>
environment, where s/he loads two competing teams that then battle<br>
each other.<br>
<br>
The agents are simulated "Steve" robots seen also in the <a href='MazeMod.md'>Maze</a> and<br>
<a href='BlocksWorldMod.md'>BlocksWorld</a> environments. In NERO they have egocentric sensors:<br>
<ul><li>laser range finders that sense distance to the nearest object (wall or a tree) in 5 directions around it: front, 45 degrees to each side, 90 degrees to each side. These sensors have a range of 100. They return 1 if they don't intersect with any walls, and a value between 0 and 1 if the intersection interrupts the ray a fraction of its length.<br>
</li><li>radar sensors that return the distance to the flag in 5 overlapping sectors: -3..18, 12..90, -18..3, -12..-90, and -90..90 (behind the agent). These sensors have a range limited to 300 (about half of the field width). Flags can be used to train agents e.g. to move around walls.<br>
</li><li>radar sensors that trigger when enemies are detected within a sector of the space around the robot. The more enemies are detected or the closer they are, the higher the value of the corresponding radar sensor will be.<br>
</li><li>two sensors that depend on the distance and direction to the center of mass of the teammates. These sensors can allow agents to stick together or spread out.<br>
</li><li>a sensor that indicates whether the agent is facing an enemy within sensor range within 2 degrees. This sensor is useful to train agents that shoot well.</li></ul>

Their effectors are<br>
<ul><li>Forward/backward speed: -1..1 of maximum. The agents move at a rate of up to 1 unit per frame (MAX_MOVEMENT_SPEED in <a href='http://code.google.com/p/opennero/source/browse/trunk/mods/NERO/constants.py'>constants.py</a>.<br>
</li><li>Turning speed: -1..1 of maximum. The agents can turn at a rate of up to 0.2 radians per frame (MAX_TURNING_RATE in <a href='http://code.google.com/p/opennero/source/browse/trunk/mods/NERO/constants.py'>constants.py</a>).</li></ul>

Because the robot can shoot only forward, both directions of running are useful (forward for attacking, backward for retreating). If the robot tries to run into an object, it is simply stopped. However, in the current version the robots run through each other (to speed up the simulation).<br>
<br>
Note that there is no output for taking a shot. Instead, the agents<br>
shoot probabilistically. First, they have to be oriented within 2<br>
degrees of the target; outside of that angle they never<br>
shoot. Similarly, if they are further than 600 lenth units away<br>
(roughly the width of the standard field), they never shoot. Between<br>
600 and 300, their likelyhood of shooting increases linearly, and<br>
within 300, they always shoot. Within the 2 degrees, their accuracy<br>
increases linearly so that at 2 degrees they have a 50% chance of<br>
hitting, and iff they are facing the center of the target exactly,<br>
they'll always hit. It is therefore possible to train agents to become<br>
better at shooting by getting closer and orienting more accurately<br>
towards the enemy.<br>
<br>
Their weapon is a laser gun that shoots a single instantaneous ray; it<br>
is blocked by walls and trees, but it has no effect on teammates. The red team shoots red rays and the blue team blue rays; if either ray hits a wall or a tree, it turns green. Each shot that hits an agent decreases the agent's hitpoints by one; once the hitpoints run out, the agent dies and is removed from<br>
the field. In battle, it is gone for good; during training, it is respawned with hitpoints and lifetime reset.<br>
<br>
The standard NERO environment consists of an enclosed field with a<br>
wall in the middle and a couple of trees around, behind which the<br>
robots can take cover. Most of the environment can be manipulated<br>
during training, and to create interesting new battlefields if so<br>
desired.<br>
<br>
The player can look around the environment as usual using the keyboard<br>
and mouse controls:<br>
<br>
<ul><li><b>W</b> - Up (also up arrow)<br>
</li><li><b>A</b> - Left (also left arrow)<br>
</li><li><b>S</b> - Down (also down arrow)<br>
</li><li><b>D</b> - Right (also right arrow)<br>
</li><li><b>Q</b> - Rotate Left<br>
</li><li><b>E</b> - Rotate Right<br>
</li><li><b>R</b> - Rotate Up<br>
</li><li><b>Z</b> or <b>scroll up</b> - Zoom in<br>
</li><li><b>C</b> or <b>scroll down</b> - Zoom out<br>
</li><li><b>F1</b> - Go to this page for help<br>
</li><li><b>F2</b> - Cycle through useful stats that can be displayed above each agent, including (in order) the time their brain has spent on the field, remaining hitpoints (percentage indicated by 5 dots), rtNEAT genome ID (indicating how recently the agent was introduced), rtNEAT species number (indicating how recently the species was introduced), and the current population champion. When you do that, the title bar in the NERO window which data are currently shown.</li></ul>

Near the top left of the screen there's a single number that indicates the current frame rate of the display. It should be 24 or higher in a visually appealing animation, but may fall to 12 or lower on a slow machine (as well as currently on MacOSX due to Irrlicht rendering issue).