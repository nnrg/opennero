from ext_agent import *

r = ext_agent("data/ai/neat_params.dat", 1, 4, 20)
allagents = []

while True:
    agent = r.getAgent()
    act = r.action([1],agent)
    act2 = r.action([-1],agent)
    r0 = act[0] + act[1] - act[2] - act[3]
    #r0 = 0
    r1 = 0
    #r1 = act2[3] + act2[2] - act2[1] - act2[0]
    r2 = r0 + r1
    if r2 > 0:
        act3 = r.action([4],agent)
        r2 += act3[3] + act3[2] + act3[1] + act3[0]
    else:
        act3 = r.action([-4],agent)
        r2 += act3[3] - act3[2] + act3[1] - act3[0]
    #print act
    print r2
    r.reward(r2,agent)
    allagents.append(agent)
    #print allagents
    r.done(agent)
