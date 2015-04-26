## Introduction ##

Brute-force (or uninformed) search methods try out alternatives
systematically in a fixed order, in order to find a solution. In the
case of the maze, it means finding a path to the goal. Watch the
videos below to see how they work, or run OpenNERO yourself to try
them out interactively.

## Running the Demo ##

To run the demo, first:

  1. [Start OpenNERO](RunningOpenNero.md)
  1. Start the Search Experiment
  1. Click on the Depth First Search button to run DFS
  1. Click on the Breadth First Search button to run BFS

The details of each method are described below.

## Depth First Search ##

[Depth-First Search](http://en.wikipedia.org/wiki/Depth-first_search) (see section 3.4.3 of [AIMA](http://aima.cs.berkeley.edu/)) is a technique for searching a graph or a tree-like structure. The Maze environment includes an agent that searches for the target (<font color='red'>RED</font> cube) by expanding the graph representing the maze in a depth-first manner. Click on Depth First Search and observe the resulting behavior. Note:

  * The maze can be abstracted as a graph of cells connected by edges where there are no walls
  * <font color='blue'>BLUE</font> square markers represent expanded nodes
  * When the agent runs into a dead-end, it has to retrace its steps to get to the next branch
  * When the agent reaches to goal, it displays the path to it with white square markers

<a href='http://www.youtube.com/watch?feature=player_embedded&v=3EG2OmjZ4wg' target='_blank'><img src='http://img.youtube.com/vi/3EG2OmjZ4wg/0.jpg' width='425' height=344 /></a>

## Breadth First Search ##

[Breadth-First Search](http://en.wikipedia.org/wiki/Breadth-first_search) (see section 3.4.1 of [AIMA](http://aima.cs.berkeley.edu/)) is a technique for searching a graph or a tree-like structure by exploring all leaves of the same depth first. MazeMod includes an agent that searches for the target (<font color='red'>RED</font> cube) by expanding the graph representing the maze in a breadth-first manner. Click on Breadth First Search and observe the resulting behavior. Note:

  * The maze can be abstracted as a graph of cells connected by edges where there are no walls
  * <font color='blue'>BLUE</font> square markers represent expanded nodes
  * <font color='green'>GREEN</font> squares mark the nodes that have been generated but not yet expanded (i.e. their successors have not been generated)
  * <font color='yellow'>YELLOW</font> square marks the node where the agent needs to go next
  * Since there is a lot more retracing the agent would need to do for a Breadth First exploration, we allow it to "Teleport" from its current position to the yellow square.
  * When the agent reaches to goal, it displays the path to it with white square markers

<a href='http://www.youtube.com/watch?feature=player_embedded&v=SzAj7s4P2zo' target='_blank'><img src='http://img.youtube.com/vi/SzAj7s4P2zo/0.jpg' width='425' height=344 /></a>

## Source Code ##

The source code for this and other Maze agents is in [agent.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/agent.py).

The Breadth First Search is implemented in the `BFSSearchAgent` class, which actually extends `GenericSearchAlgorithm` along with `AStarSearch`. Depth First search is implemented in `DFSSearchAgent`. See SystemOverview for a general description of how agents work.

## Next Steps ##

There are other varieties of search. For example, [AI: A Modern Approach](http://aima.cs.berkeley.edu/) describes the following uninformed search strategies:

  * 3.4. Uninformed Search Strategies ... 81
    * 3.4.1. Breadth-first search ... 81
    * 3.4.2. Uniform-cost search ... 83
    * 3.4.3. Depth-first search ... 85
    * 3.4.4. Depth-limited search ... 87
    * 3.4.5. Iterative deepening depth-first search ... 88
    * 3.4.6. Bidirectional search ... 90

We encourage you to implement one of these in OpenNERO as a programming assignment - you really understand something when you build it!

After you understand all there is to know about these uninformed search agents, you can check out the HeuristicSearch demo page.