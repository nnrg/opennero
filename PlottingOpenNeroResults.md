# Introduction #

OpenNERO comes with its own tool for plotting the progress of an AI algorithm (the reward and fitness it receives from the environment). This tool can be configured to run in parallel with the program, or it can be used after an experiment has finished to display the results as a graph.

# Prerequisites #

In order to run the built-in plot viewer, you will need the following installed on your operating system:

  * [Python](http://www.python.org/)
  * [matplotlib](http://matplotlib.sourceforge.net/)
  * [wxPython](http://www.wxpython.org/)

# Details #

  * Locate the logging output produced by OpenNERO (see OpenNeroLogs)
  * Run
```
python plot_server.py YOUR_NERO_LOG
```