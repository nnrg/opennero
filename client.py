from OpenNero import *
from random import seed, randint

# add the key and mouse bindings
from inputConfig import createInputMapping

from common import *
import common.gui as gui

from TeamAdapt.module import getMod, delMod
from TeamAdapt.constants import *

#########################################################

def CreateGui(guiMan):
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")  
    
#    dfsButton = gui.create_button(guiMan, 'dfs', Pos2i(0, 0), Pos2i(100, 30), '')
#    dfsButton.text = 'Depth First Search'
#    dfsButton.OnMouseLeftClick = lambda: getMod().start_dfs()
#
#    aStarButton = gui.create_button(guiMan, 'astar', Pos2i(0, 30), Pos2i(100, 30), '')
#    aStarButton.text = 'Single Agent A*'
#    aStarButton.OnMouseLeftClick = lambda: getMod().start_astar()
#
#    aStarButton2 = gui.create_button(guiMan, 'astar2', Pos2i(0, 60), Pos2i(100, 30), '')
#    aStarButton2.text = 'Teleporting A*'
#    aStarButton2.OnMouseLeftClick = lambda: getMod().start_astar2()
#
#    aStarButton3 = gui.create_button(guiMan, 'astar3', Pos2i(0, 90), Pos2i(100, 30), '')
#    aStarButton3.text = 'Front A*'
#    aStarButton3.OnMouseLeftClick = lambda: getMod().start_astar3()
#
#    fpsButton = gui.create_button(guiMan, 'fps', Pos2i(0,120), Pos2i(100,30), '')
#    fpsButton.text = 'First Person Control'
#    fpsButton.OnMouseLeftClick = lambda: getMod().start_fps()

    randomButton = gui.create_button(guiMan, 'random', Pos2i(0,0), Pos2i(100,30), '')
    randomButton.text = 'Random Baseline'
    randomButton.OnMouseLeftClick = lambda: getMod().start_random()

    rtneatButton = gui.create_button(guiMan, 'rtneat', Pos2i(0,30), Pos2i(100,30), '')
    rtneatButton.text = 'Neuroevolution'
    rtneatButton.OnMouseLeftClick = lambda: getMod().start_rtneat()

#    sarsaButton = gui.create_button(guiMan, 'sarsa', Pos2i(0,210), Pos2i(100,30), '')
#    sarsaButton.text = 'Sarsa'
#    sarsaButton.OnMouseLeftClick = lambda: getMod().start_sarsa()
#
#    qlearningButton = gui.create_button(guiMan, 'qlearning', Pos2i(0,240), Pos2i(100,30), '')
#    qlearningButton.text = 'Q-Learning'
#    qlearningButton.OnMouseLeftClick = lambda: getMod().start_qlearning()

    agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(100, 400), 'Agent')
#    agentWindow.addChild(dfsButton)
#    agentWindow.addChild(aStarButton)
#    agentWindow.addChild(aStarButton2)
#    agentWindow.addChild(aStarButton3)
#    agentWindow.addChild(fpsButton)
    agentWindow.addChild(randomButton)
    agentWindow.addChild(rtneatButton)
#    agentWindow.addChild(sarsaButton)
#    agentWindow.addChild(qlearningButton)

    epsilon_percent = int(INITIAL_EPSILON * 100)
    epsilonValue = gui.create_text(guiMan, 'epsilonEditBox', Pos2i(260,0), Pos2i(100,30), str(epsilon_percent))

    epsilonLabel = gui.create_text(guiMan, 'epsilonLabel', Pos2i(10,0), Pos2i(100,30), 'Exploit-Explore:')

    # this can be used to adjust the exploration-exploitation tradeoff (fraction of 
    # champion organisms in the case of rt-NEAT and fraction of greedy actions in the case
    # of the epsilon-greedy RL methods like Sarsa and Q-learning)
    epsilonScroll = gui.create_scroll_bar(guiMan, 'epsilonScroll', Pos2i(100,0), Pos2i(150,20), True)
    epsilonScroll.setMax(100)
    epsilonScroll.setLargeStep(10)
    epsilonScroll.setSmallStep(1)
    epsilonScroll.setPos(epsilon_percent)
    getMod().set_epsilon(INITIAL_EPSILON)
    epsilonScroll.OnScrollBarChange = epsilon_adjusted(epsilonScroll, epsilonValue)
    
    speedupValue = gui.create_text(guiMan, 'speedupEditBox', Pos2i(260, 30), Pos2i(100, 30), str(0))
    
    speedupLabel = gui.create_text(guiMan, 'speedupLabel', Pos2i(10, 30), Pos2i(100, 30), 'Speedup:')
    
    speedupScroll = gui.create_scroll_bar(guiMan, 'speedupScroll', Pos2i(100, 30), Pos2i(150,20), True)
    speedupScroll.setMax(100)
    speedupScroll.setLargeStep(10)
    speedupScroll.setSmallStep(1)
    speedupScroll.setPos(0)
    getMod().set_speedup(0)
    speedupScroll.OnScrollBarChange = speedup_adjusted(speedupScroll, speedupValue)
    
    paramWindow = gui.create_window(guiMan, 'paramWindow', Pos2i(20, 500), Pos2i(300,100), 'Parameters')
    paramWindow.addChild(epsilonLabel)
    paramWindow.addChild(epsilonScroll)
    paramWindow.addChild(epsilonValue)
    paramWindow.addChild(speedupLabel)
    paramWindow.addChild(speedupScroll)
    paramWindow.addChild(speedupValue)

def epsilon_adjusted(scroll, value):
    # generate a closure that will be called whenever the epsilon slider is adjusted
    value.text = str(scroll.getPos())
    getMod().set_epsilon(float(scroll.getPos())/100)
    def closure():
        value.text = str(scroll.getPos())
        getMod().set_epsilon(float(scroll.getPos())/100)
    return closure

def speedup_adjusted(scroll, value):
    # generate a closure that will be called whenever the speedup slider is adjusted
    value.text = str(scroll.getPos())
    getMod().set_speedup(float(scroll.getPos())/100)
    def closure():
        value.text = str(scroll.getPos())
        getMod().set_speedup(float(scroll.getPos())/100)
    return closure

def recenter(cam):
    def closure():
        cam.setPosition(Vector3f(NUDGE_X, NUDGE_Y, 30))
        cam.setTarget(Vector3f(NUDGE_X + GRID_DX * ROWS / 2, NUDGE_Y + GRID_DY * COLS / 2, 5))
    return closure

def ClientMain():
    # create fog effect
    getSimContext().setFog()

    # don't show physics
    # disable_physics()

    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 3000
    camZoomSpeed   = 100
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    recenter_cam = recenter(cam) # create a closure to avoid having a global variable
    recenter_cam() # call the recenter function

    # load the background
    #addObject("data/terrain/Terrain.xml", Vector3f(-3000 + NUDGE_X,-3000 + NUDGE_Y,-20))
    addObject("data/terrain/FlatTerrain.xml", Vector3f(-100 + NUDGE_X, -100 + NUDGE_Y, -5), Vector3f(0,0,0))
    addSkyBox("data/sky/irrlicht2")

    # load the maze
    getSimContext().addLightSource(Vector3f(-500,-500,1000), 1500)
    getMod().add_maze()

    # load the GUI
    CreateGui(getGuiManager())

    # create the key binding
    ioMap = createInputMapping()
    ioMap.BindKey( "KEY_SPACE", "onPress", recenter_cam )
    getSimContext().setInputMapping(ioMap)
