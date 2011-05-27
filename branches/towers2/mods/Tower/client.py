from OpenNero import *
from random import seed, randint

# add the key and mouse bindings
from inputConfig import createInputMapping

from common import *
import common.gui as gui

from Tower.module import getMod, delMod
from Tower.constants import *

#########################################################

def CreateGui(guiMan):
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")  
    
    towerButton3 = gui.create_button(guiMan, 'tower3', Pos2i(0,0), Pos2i(150,30), '')
    towerButton3.text = '3 Layer Towers of Hanoi'
    towerButton3.OnMouseLeftClick = lambda: getMod().start_tower(3)

    towerButton4 = gui.create_button(guiMan, 'tower4', Pos2i(0,30), Pos2i(150,30), '')
    towerButton4.text = '4 Layer Towers of Hanoi'
    towerButton4.OnMouseLeftClick = lambda: getMod().start_tower(4)
    
    towerButton5 = gui.create_button(guiMan, 'tower5', Pos2i(0,60), Pos2i(150,30), '')
    towerButton5.text = '5 Layer Towers of Hanoi'
    towerButton5.OnMouseLeftClick = lambda: getMod().start_tower(5)
    
    agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(150, 120), 'Agent')
    agentWindow.addChild(towerButton3)
    agentWindow.addChild(towerButton4)
    agentWindow.addChild(towerButton5)

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
    #addObject("data/terrain/Sea.xml", Vector3f(-3000 + NUDGE_X,-3000 + NUDGE_Y,-20))
    addObject("data/terrain/FlatTerrain.xml", Vector3f(-500,-500,0), Vector3f(0,0,0)) #Vector3f(-1100 + NUDGE_X, -2400 + NUDGE_Y, -17), Vector3f(0,0,-45))
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
