# OpenNero will execute ModMain when this mod is loaded
from BlocksPlanning.client import ClientMain

def ModMain():
    ClientMain()

def StartMe():
    from BlocksPlanning.module import getMod
    getMod().set_speedup(1.0) # full speed ahead
    getMod().start_sarsa() # start an algorithm for headless mode
