## Purpose ##

In this assignment, you will write a very simple behavior script for a roomba-like "vacuum cleaner" agent in OpenNERO. You should learn how to write Python behaviors in OpenNERO and play with trying to improve the performance of the behavior.

## Instructions ##

  * Check out OpenNERO using Subversion. Use the URL http://opennero.googlecode.com/svn/trunk.
  * Build OpenNERO (see BuildingOpenNero)
  * Run the roomba example OpenNERO (See RoombaMod)
  * Understand the relevant code
    * Look at the following files:
      * [mods/Roomba/data/shapes/roomba/Roomba.xml](http://code.google.com/p/opennero/source/browse/trunk/mods/Roomba/data/shapes/roomba/Roomba.xml): This is the XML file that tells OpenNERO what the bot looks like and what brain it will use
      * [mods/Roomba/roomba.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Roomba/roomba.py): This is a Python file that contains the behavior for a scripted Roomba agent that you will modify
        * Note the `RoombaBrain` class has the following important methods (that it overrides from a general `AgentBrain`)
          * initialize(init\_info) - this tells the brain what the environment is like: how many observations it will receive, how many actions it should provide, and what the reward looks like
          * start(time, observations): action - this method implements the first decision the Brain has to make - given the initial observation, what is the first action?
          * act(time, observations, reward): action - this method is called (potentially many times in a row) to figure out the next action after receiving a reward and seeing a new set of observations
          * end(time, reward) - this method just informs the Brain what the last reward was. Sometimes this is very important - for example when the agent doesn't know how well it did until the end of the episode.
      * [mods/Roomba/module.py](http://code.google.com/p/opennero/source/browse/trunk/mods/Roomba/module.py): This is a Python file that describes the Roomba environment, including its transition and reward functions.
  * Compare the provided script with a random baseline
    * How well do the different kinds of agents work in this environment?
    * Plug in a random agent instead of `RoombaBrain`:
```
class RandomAgent(AgentBrain):
    """
    A uniform random baseline - a very simple agent
    """
    def __init__(self):
        AgentBrain.__init__(self) # have to make this call
    def initialize(self, init_info):
        """
        create a new agent
        """
        self.action_info = init_info.actions
        return True
    def start(self, time, observations):
        """
        return first action given the first observations
        """
        return self.action_info.random()
    def reset(self):
        pass
    def act(self, time, observations, reward):
        """
        return an action given the reward for the previous action and the new observations
        """
        return self.action_info.random()
    def end(self, time, reward):
        """
        receive the reward for the last observation
        """
        print  "Final reward: %f, cumulative: %f" % (reward[0], self.fitness[0])
        return True
    def destroy(self):
        """
        called when the agent is done
        """
        return True
```
    * Run a single vacuum with this behavior a number of times to get a good idea of how well it does on average
    * Use **plot\_server.py** (see PlottingOpenNeroResults) to look at the fitness of the agent recorded in **nero\_log.txt** (see OpenNeroLogs) (it's a good idea to save this file after a long run).
```
pythonw plot_server.py path/to/your/nero_log.txt
```
    * Now compare your results to how `RoombaBrain` does. Can you see a qualitative difference in the graph?
  * Create your own scripted behavior
    * Can you make a better version of `RoombaBrain`? Can you make it faster (it currently takes some time for each vacuum to figure out where the closest crumbs are)? You can start by modifying a copy of `RoombaBrain` or `RandomAgent` depending on what you have in mind.
    * Your behavior does not need to be better than the one implemented by `RoombaBrain`, but it needs to be different in some rational or useful way: you should be able to justify it by saying something like "I tried to do this because...".
    * Compare your behavior with the random baseline and with the `RoombaBrain`. How well does it do?
  * Bonus: add Sarsa or Q-Learning RL agent into Roomba
    * The Maze mod has Sarsa and Q-Learning reinforcement learning agents that work well in this environment.
    * Can you add a similar agent to Roomba?
    * How well does it work? If everything is working, we should be able to see some improvement over time in the graphs.

## Questions ##

  1. What are the method(s) of `RandomAgent` that define what it does (its actions)?
  1. What are the method(s) of `RoombaEnvironment` that define what the vacuum "feels"?
  1. What are the method(s) of the `RoombaEnvironment` that interpret the vacuum "actions"?
  1. What are the things that Roomba "feels"?
  1. What are the actions that are available to Roomba?
  1. What is the average score you got from 10 random runs of Roomba? (NOTE: the reward gets printed after each episode to the console and to the nero\_log.txt file)
  1. What was your improved strategy?
  1. How well did it do?