## Introduction ##

One of the main reasons to use games for AI research is that they are fun for people to play. Since the grand goal of AI is to achieve human-level computational intelligence, it helps to be able to interact with, and learn from, people. OpenNERO has a growing number of facilities and examples for doing just that.

## First Person Control ##

First person games (including action, adventure, puzzle and so on) are a large and popular genre of modern video and computer games. The basic paradigm is looking at the world through the eyes of a character (or an AI agent). Such "walking in someone else's shoes" can give a surprising amount of perspective about the tasks that need to be solved by an AI algorithm.

In MazeMod, the user can select the First Person button to try to solve the maze themselves.

To control the agent, use A,W,S,D keys or the arrow keys on the keyboard.

<a href='http://opennero.googlecode.com/svn/wiki/OpenNERO-maze-fp.png'><img width='400' alt='' height='300' src='http://opennero.googlecode.com/svn/wiki/OpenNERO-maze-fp.png' /></a>

Even though the picture is in some ways more complicated (and more informative) than the ray sensors used by the learning agents, it is pretty easy to see that solving the maze in this way is not as easy as it would be looking down on it. The player has to keep in mind a map (which they don't have to start with!) and where on this map they are. This is also called _Simultaneous Localization And Mapping_ problem, or _SLAM_. It can get pretty tricky.

## Scripted Roomba ##

In RoombaMod, one simple strategy is the greedy vacuum cleaner that detects and drives to the nearest piece of litter. In order to start this demonstration, first select the Roomba mod from the main menu and then **Switch to Script** and **Add Bots**.

There are a number of strategies that a student can use to write a script for the Roomba robot. These range from simple greedy solutions that always work to scripts that use some knowledge about e.g. the shape of the room or the distribution of the dirt in it. They of course also include learning methods such as RL or rtNEAT, but the difference is that those methods can _improve with experience_.