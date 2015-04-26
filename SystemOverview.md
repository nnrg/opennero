OpenNERO is a federation of several _mods_ - nearly independent
collections of resources and scripts stored in individual directories.
For example, there are the [Maze mod](MazeMod.md), the [NERO mod](NeroMod.md)
and the [Roomba mod](RoombaMod.md) - see [Running OpenNERO](RunningOpenNero.md)
for more details.

Since OpenNERO is all about designing intelligent agents, it has an
API for adding new _agents_ and new _environments_ to test them in.
An agent can interact with an environment by receiving _observations_
from it, sending it _actions_, and receiving _rewards_ for those
actions. Agents in OpenNERO extend the class `AgentBrain`.
Environments extend the `Environment` class. Both agents and
environments can be written in Python or in C++.

<img src='http://opennero.googlecode.com/svn/wiki/agent-environment-loop.png' height='300' width='400></img'>

<h2>Agent-Environment interaction diagram</h2>

<img src='http://opennero.googlecode.com/svn/wiki/agent-environment-sequence.png' height='864' width='553></img'>

<h2>Agent-Environment interface classes</h2>

The main interfaces (which are defined in C++ and exported into Python) look like this:<br>
<br>
<h3><a href='http://code.google.com/p/opennero/source/browse/trunk/source/ai/Environment.h'>Environment</a></h3>
<ul><li><b>sense</b>(agent): <code>Observations</code> <font color='green'>// produce the observations that the agent 'sees'</font>
</li><li><b>step</b>(agent, action): <code>Reward</code> <font color='green'>// perform the action requested by the agent and return the reward</font>
</li><li><b>get_agent_info</b>(agent): <code>AgentInitInfo</code> <font color='green'>// return the initialization information describing the state and action spaces for the agent</font>
</li><li><b>is_episode_over</b>(agent): boolean <font color='green'>// return true iff the episode is over for the agent</font>
</li><li><b>cleanup</b>(agent) <font color='green'>// clean up the environment before exiting</font>
</li><li><b>reset</b>(agent) <font color='green'>// return the environment to its initial state</font></li></ul>

<h3><a href='http://code.google.com/p/opennero/source/browse/trunk/source/ai/AgentBrain.h'>AgentBrain</a></h3>
<ul><li><b>initialize</b>(agent_init_info) <font color='green'>// initialize the agent with the <code>AgentInitInfo</code> from the environment</font>
</li><li><b>start</b>(time, observations): <code>Actions</code> <font color='green'>// determine the first action the agent should take given its initial observations</font>
</li><li><b>act</b>(time, observations, reward): <code>Actions</code> <font color='green'>// determine the consequent actions the agent should take given the observations</font>
</li><li><b>end</b>(time, observations, reward) <font color='green'>// get information about the final observation and reward from the environment</font>
</li><li><b>destroy</b>() <font color='green'>// destroy the agent and release its resources</font></li></ul>

<h3><code>AgentInitInfo</code>, <code>Actions</code> and <code>Observations</code></h3>

The basic types OpenNERO uses to pass around information between <code>Environment</code> and <code>Agent</code> are defined in <a href='http://code.google.com/p/opennero/source/browse/trunk/source/ai/AI.h'>AI.h</a>. Both <code>Actions</code> and <code>Observations</code> are actually a <code>FeatureVector</code> - an array of one or more double values. Conceptually, a <code>FeatureVector</code> carries discrete or continuous values within a certain range. In order to describe the possible values for a particular <code>FeatureVector</code>, a <code>FeatureVectorInfo</code> is used:<br>
<br>
<h3><code>AgentInitInfo</code></h3>
<ul><li><b>sensors</b>: <code>FeatureVectorInfo</code> <font color='green'>// description of the state (observation) space</font>
</li><li><b>actions</b>: <code>FeatureVectorInfo</code> <font color='green'>// description of the action space</font>
</li><li><b>rewards</b>: <code>FeatureVectorInfo</code> <font color='green'>// description of the rewards</font></li></ul>

<h3><code>FeatureVectorInfo</code></h3>
<ul><li><b>size</b>(): integer <font color='green'>// number of elements in the vector</font>
</li><li><b>addDiscrete</b>(min, max): integer <font color='green'> // add a discrete feature</font>
</li><li><b>addContinuous</b>(min, max): integer <font color='green'> // add a continuous feature</font>
</li><li><b>getBounds</b>(i): <code>Bounds</code> <font color='green'> // get the bounds of the ith feature</font>
</li><li><b>getMin</b>(i): double <font color='green'>// min of ith element</font>
</li><li><b>getMax</b>(i): double <font color='green'>// max of ith element</font>
</li><li><b>isDiscrete</b>(i): boolean <font color='green'>// return true iff the ith feature is discrete</font>
</li><li><b>isContinuous</b>(i): boolean <font color='green'>// return true iff the ith feature is continous</font>
</li><li><b>validate</b>(feature_vector): boolean <font color='green'>// check that feature_vector is in compliance with these bounds</font>
</li><li><b>getInstance</b>(): <code>FeatureVector</code> <font color='green'>// get a zero instance of this feature vector</font>
</li><li><b>getRandom</b>(): <code>FeatureVector</code> <font color='green'>// get a random instance of this feature vector</font></li></ul>

<h2>Components</h2>

OpenNERO is built using open source components, including:<br>
<ul><li><a href='http://irrlicht.sourceforge.net/'>Irrlicht 3D Engine</a> - released under the <a href='http://irrlicht.sourceforge.net/license.html'>Irrlicht Engine License</a>
</li><li><a href='http://www.boost.org/'>Boost C++ libraries</a> - governed by the <a href='http://www.boost.org/users/license.html'>Boost Software License</a>
</li><li><a href='http://www.python.org/'>Python scripting language</a> - governed by the <a href='http://www.python.org/psf/license/'>Python License</a>
</li><li><a href='http://nn.cs.utexas.edu/?rtNEAT'>rtNEAT algorithm</a> - created by <a href='http://www.cs.ucf.edu/~kstanley/'>Ken Stanley</a> and <a href='http://www.cs.utexas.edu/~risto/'>Risto Miikkulainen</a> at UT Austin.