from OpenNero import *
from random import seed, randint

# add the key and mouse bindings
from inputConfig import createInputMapping

from common import *
import common.gui as gui

from BlocksTower.module import getMod, delMod
from BlocksTower.constants import *

#########################################################

def CreateGui(guiMan):
    print 'CreateGui'
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    towerButton1 = gui.create_button(guiMan, 'tower1', Pos2i(0,0), Pos2i(250,30), '')
    towerButton1.text = 'Towers of Hanoi via recursive solver'
    towerButton1.OnMouseLeftClick = lambda: getMod().start_tower1()

    towerButton2 = gui.create_button(guiMan, 'tower2', Pos2i(0,30), Pos2i(250,30), '')
    towerButton2.text = 'Towers of Hanoi via search'
    towerButton2.OnMouseLeftClick = lambda: getMod().start_tower2()

    towerButton3 = gui.create_button(guiMan, 'tower3', Pos2i(0,60), Pos2i(250,30), '')
    towerButton3.text = 'Towers of Hanoi via STRIPS planning'
    towerButton3.OnMouseLeftClick = lambda: getMod().start_tower3()

    agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(250, 120), 'Agent')
    agentWindow.addChild(towerButton1)
    agentWindow.addChild(towerButton2)
    agentWindow.addChild(towerButton3)

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
