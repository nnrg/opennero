# Iterative Deepening `A*` Search #

In this exercise, you will be implementing the Iterative Deepening
`A*` (`IDA*`) search algorithm in the OpenNERO platform. You'll use
the existing implementations of Depth First Search (DFS) and `A*`
search as starting points, and modify and extend them to implement
`IDA*`. `IDA*` search is an informed search algorithm that uses the
same heuristic-function strategy as `A*`, but follows a DFS approach
and only keeps track of the most recent path, thus reducing the memory
complexity of the algorithm.


Before your implement your agent, it's a good idea to look at the demos for [Brute Force](BruteForceSearch.md) (uninformed) and [Heuristic](HeuristicSearch.md) (informed) search to familiarize yourself with the behavior of the implemented search agents.

## Downloading Exercise Files ##

To create your `IDA*` agent, start with downloading the [skeleton files for the search exercise](http://opennero.googlecode.com/svn/wiki/Hw1.tar.gz). Copy the folder from the archive to your OpenNERO installation folder. At this point, a new mod should show up in OpenNERO whenever you run it.

## Implementing Your Agent ##

Open the agent.py file inside the exercise folder. Modify the code in the IdaStarSearchAgent class to implement the `IDA*` search algorithm.  The starting implementation for this agent is listed below.  The starting implementation returns random actions on every request.

You may want to study the implementation of other search agents in the Maze environment in the [Maze/agent.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/agent.py) file (located in trunk/mods/Maze). Notice that there are several agents implemented here, including a DFS agent (DFSSearchAgent) and an `A*` agent (AStarSearchAgent).

### Agent Class ###

```
class IdaStarSearchAgent(SearchAgent):
    """
    IDA* algorithm
    """
    def __init__(self):
        SearchAgent.__init__(self)

    def reset(self):
        """
        Reset the agent
        """
        pass

    def initialize(self, init_info):
        """
        Initializes the agent upon reset
        """
        self.action_info = init_info.actions
        return True

    def start(self, time, observations):
        """
        Called on the first move
        """
        return self.action_info.random()
    
    def act(self, time, observations, reward):
        """
        Called every time the agent needs to take an action
        """
        return self.action_info.random()

    def end(self, time, reward):
        """
        at the end of an episode, the environment tells us the final reward
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        self.reset()
        return True

    def destroy(self):
        """
        After one or more episodes, this agent can be disposed of
        """
        return True
```

Your agent is operating in a 2-D maze and is capable of moving from cell to cell. In the `start` and `act` methods, your agent receives a collection of `observations` about the world:

  * `observations[0]` -- The current row position of your agent.
  * `observations[1]` -- The current column position of your agent.
  * `observations[2]` -- 1 if there is an obstacle in the row above your position, otherwise 0.
  * `observations[3]` -- 1 if there is an obstacle in the row below your position, otherwise 0.
  * `observations[4]` -- 1 if there is an obstacle in the column to the right of your position, otherwise 0.
  * `observations[5]` -- 1 if there is an obstacle in the column to the left of your position, otherwise 0.

In these methods, you are required to return an `action` -- an integer corresponding to the desired move:

  * 0 -- Move to the row above. `(row, column) → (row + 1, column)`
  * 1 -- Move to the row below. `(row, column) → (row - 1, column)`
  * 2 -- Move to the column to the right. `(row, column) → (row, column + 1)`
  * 3 -- Move to the column to the left. `(row, column) → (row, column - 1)`
  * 4 -- Do nothing.

The class above is an empty skeleton. Refer to the other agent implementations in `agent.py` for guidance on how to implement in your agent.

## Frequently Asked Questions ##

You can check out the [FAQ](http://www.cs.utexas.edu/users/risto/cs343/private/hw1/hw1.html) accumulated from previous classes.

### Debugging ###

If you run into any bugs in your program, you can find the error log file for OpenNERO at one of the following locations:

  * **Linux or Mac:** `~/.opennero/nero_log.txt`
  * **Windows:** `"AppData\Local\OpenNERO\nero_log.txt"` or `"Local Settings\Application Data\OpenNERO\nero_log.txt"` depending on the version of Windows you have.