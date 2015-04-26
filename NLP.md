## Introduction ##

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-nlp.png' height='300' width='400></img'>

Planning means deciding on a course of action before acting. In this<br>
demo, three different planning methods (goal-stacking, forward<br>
search, and problem reduction) are demonstrated in the Towers of Hanoi<br>
task. The planning process is first run in the background,<br>
illustrating its progress in a text window. The Steve robot then<br>
executes the completed plan in the environment.  Watch the videos<br>
below to see how the planning demo works, or run OpenNERO yourself to<br>
try it out interactively.<br>
<br>
<h2>Running the Demo</h2>

To run the demo, first:<br>
<br>
<ol><li><a href='RunningOpenNero.md'>Start OpenNERO</a>
</li><li>Start the NLP experiment<br>
</li><li>Select Semantic Parser from the pull-down menu<br>
</li><li>Click on the Start button to start the demo</li></ol>

<h2>Text input interface</h2>

Once you run the agent, a pop-up dialog box will open up which will take text input. For a start, enter the following commands in the dialog box and see how the agent responds.<br>
<br>
Move Disk1 from Pole1 to pole3<br>
Pick up Disk1 lying on Pole1<br>
Put Disk1 on Pole2<br>
<br>
The current parser can only handle three possible types of English language sentences and the actions corresponding to each of them.<br>
<br>
<table><thead><th> <b>English Sentence</b> </th><th> <b>OpenNERO Command</b> </th></thead><tbody>
<tr><td> Move Disk1 from Pole1 to Pole2 </td><td> Mov Disk1 Pole1 Pole2 </td></tr>
<tr><td> Pick up Disk1 from Pole1 </td><td> Pick Disk1 Pole1 </td></tr>
<tr><td> Put down Disk1 on Pole2 </td><td> Put Disk1 Pole2 </td></tr></tbody></table>

Similarly, variants of the above sentences/commands can be made by using Disk1/Disk2/Disk3 and Pole1/Pole2/Pole3. The command format is always:<br>
<br>
<pre><code>S -&gt; VP1 NP1 NP3 NP4 | VP2 NP1 NP3 | VP3 NP1 NP4<br>
VP1 -&gt; move <br>
VP2 -&gt; pick up <br>
VP3 -&gt; put<br>
NP1 -&gt; disk1 | disk2 | disk3<br>
NP2 -&gt; pole1 | pole2 | pole3<br>
NP3 -&gt; from NP2<br>
NP4 -&gt; to NP2 | on NP2<br>
</code></pre>

The simulator will only execute legal commands. For example, if Disk1 is lying on Pole1, then you cannot pick it from Pole2 or, if Disk1 is lying on Disk2, then you cannot move Disk2. Remember, the starting configuration of 3-disk planner includes Disk3 lying on Pole1, Disk2 lying on Disk3 and Disk1 lying on Disk2.