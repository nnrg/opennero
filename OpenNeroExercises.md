This page will list exercises for using OpenNERO to learn more about Artificial Intelligence, as well as to learn about how to modify OpenNERO itself. Some of these were introduced and tested in the [Computational Intelligence in Game Design](http://nn.cs.utexas.edu/classes/cig) stream of the [Freshman Research Initiative](http://cns.utexas.edu/fri) at UT Austin, and others in the [Artificial Intelligence class](http://www.cs.utexas.edu/users/risto/cs343) at UT Austin. Suggestions for further exercises are listed at the end of each demo page as well.

Extending OpenNERO
  * AddingStuffExercise - The point of OpenNERO is that you can make new games and new experiments with it. In this exercise you will add a landscape, an object, and an agent to OpenNERO.
  * CreateRoombaAgentExercise - Once we add an agent, we have to program it. Traditionally this is done by writing a script that describes what the agent does. In this exercise, you will write a script for a virtual vacuum cleaner that cleans the floor of a messy computer lab.
  * MazeGeneratorExercise - Games are all about puzzles. In this exercise, you are asked to come up with a bunch of puzzles for our agents to solve. See how hard you can make them!
  * MazeSolverExercise - Now that you have all these new puzzles, can you write an agent that always finds the goal?

AI Exercises
  * HeuristicSearchExercise - Modify existing DFS and `A*` algorithms in the Maze environment to implement Iterative-Deepening `A*` (`IDA`).
  * PlanningExercise - Modify the existing planning methods to make them more effective.
  * [NLPExercise](NLPExercise.md) - Extend the existing NLP grammar.
  * [QLearningExercise](QLearningExercise.md) - Modify the existing Q-learning agent to utilize a simple function approximator, in order to scale it to a more continuous state space.
  * MazeLearnerExercise - Modern AI is all about _machine learning_ - algorithms that improve with experience. This exercise will introduce you to ways of running, comparing, and writing learning algorithms in OpenNERO.
  * AdvancedMazeExercise - Write a creative learning algorithm to produce maze solutions.
  * NeroTournamentExercise - Train intelligent agents in the NERO game through neuroevolution and/or Q-learning.
  * ObjectRecognitionExercise - Build a classifier to recognize objects in the NERO environment.