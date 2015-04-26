## Overview ##

In this exercise, you will implement a maze generation algorithm from a class of [perfect maze generation algorithms](http://www.astrolog.org/labyrnth/algrithm.htm#perfect). Pick an algorithm for generating a maze from this page: http://www.astrolog.org/labyrnth/algrithm.htm#perfect and implement it.

We are aiming for a variety of different algorithms, so once  you decide on something you think you want to do, feel free to send us your solution to be included in the code.

Think about other possible maze generation strategies - will your maze have loops in it? Will it be static or dynamic?

## Instructions ##

Check out the trunk of OpenNERO as usual and build it.

Please look in [[mods/Maze/agent.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/agent.py)] for the search agents and in [[mods/Maze/mazer.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/mazer.py)] and understand how they work.

Note that you can run [[mods/Maze/mazer.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/mazer.py)] by itself using python to quickly test it.

Add an alternative maze generation algorithm to mazer.py and test it on the command line and within OpenNERO. Do the current maze solvers available solve the maze you generated?