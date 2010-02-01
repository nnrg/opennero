
class ext_agent:

   def  __init__(self, params, ins, outs, popsize):
       from rtneat2 import *
       self.rtneat = RTNEAT(params, ins, outs, popsize, 1)
       self.chart = {}
       self.id = 0
       print

   def action(self, sensors,A):
       print "action"
       print sensors
       print dir(A)
       print A 
       print type(A)
       print type(sensors)
       A.net.load_sensors(sensors)
       print "sensors loaded!"
       A.net.activate()
       action = A.net.get_outputs()
       return action

   def reward(self, value,agent):
       agent.time_alive += 10
       x = agent.net.num_inputs
       agent.fitness += value

   def done(self, agent):
       agent.net.flush()

   def getAgent(self):
       agent = self.rtneat.next_organism()
       self.id += 1
       print type(agent)
       return agent
