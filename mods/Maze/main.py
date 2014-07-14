# OpenNero will execute ModMain when this mod is loaded
from Maze.client import ClientMain

def ModMain(mode = ""):
    ClientMain(mode)

def StartMe():
    from Maze.module import getMod
    getMod().set_speedup(1.0) # full speed ahead
    getMod().start_sarsa() # start an algorithm for headless mode
