[Sarsa](http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node64.html) is a type of temporal difference [Reinforcement Learning](http://webdocs.cs.ualberta.ca/~sutton/book/the-book.html) (section 21.2.3 of [AIMA](http://aima.cs.berkeley.edu/)) that uses a _value function_ to pick the best action.

In MazeMod, a Sarsa reinforcement agent leans to navigate the maze from experience. To start this demonstration, start OpenNERO and click on the Maze button. Once the maze loads, click the Sarsa button. You can adjust the fraction of the actions taken greedily vs actions taken to explore the environment using the explore-exploit slider.

Time-difference reinforcement learning agents can optionally use a _function approximator_ such as an ANN. In the simple case of the maze, the methods do not use an approximator and store a value for each state-action pair (tabular case).

Implementation of this method can be found in [Sarsa.h](http://code.google.com/p/opennero/source/browse/trunk/source/ai/rl/Sarsa.h) and [Sarsa.cpp](http://code.google.com/p/opennero/source/browse/trunk/source/ai/rl/Sarsa.cpp).


Additional parameters can be set in [SydneySarsa.xml](http://code.google.com/p/opennero/source/browse/trunk/mods/Maze/data/shapes/character/SydneySarsa.xml) by changing the AI section:

```
  <AI>
    <Python agent="SarsaBrain(0.9, 0.05, 0.3, 0.4)" />
  </AI>
```

The parameters passed to the SarsaBrain constructor are:

  * γ (gamma) - reward discount factor (between 0 and 1)
  * α (alpha) - learning rate (between 0 and 1)
  * ε (epsilon) - parameter for the epsilon-greedy policy (between 0 and 1)
  * λ (lambda) - parameter for the SARSA(lambda) learning algorith