## Introduction ##

The different search methods are demonstrated in the maze environment,
where the goal is to find a path through the maze to the goal
indicated by the red cube. The mazes are generated randomly; they
contain no loops, i.e. there's a unique shortest path to the goal.
You can try new mazes by pressing the "Generate New Maze" button.

## First-Person Search ##

<a href='http://www.youtube.com/watch?feature=player_embedded&v=ZktDYB94vFo' target='_blank'><img src='http://img.youtube.com/vi/ZktDYB94vFo/0.jpg' width='425' height=344 /></a>

Before exploring the different search methods, you may want to try to
solve the maze yourself, by pressing the "First Person" button. You
can move in the maze using the keyboard controls:

  * w = forward
  * a = turn left
  * d = turn right
  * s = move back

If you keep pressing continuously, you'll move continuously. You can
also move faster by moving the speedup slider to the right.

By trying to solve the maze yourself, you can gain appreciation of how
hard this task actually is. Many locations in the maze look the same,
and you have to try to remember where you've been and where you are
going. Try solving the maze first after taking a look at the maze from
the air first; then try solving it without such global knowledge.  You
can sometimes see the mountains and even the red cube from a distance,
but notice that the search methods do not get any such clues. They
have to simply systematically search through alternatives. And they
are really good at it!

After you are familiar with the search task, try
[Brute Force methods](BruteForceSearch.md)
and [Heuristic methods](HeuristicSearch.md) on it.
Later on, you may also want to try
[methods for learning to run the maze](LearningDemo.md).