from OpenNero import *
from random import seed, randint

from math import *

from inputConfig import *
from common import *
import common.gui as gui
from creativeit.module import getMod, delMod
from creativeit.constants import *

def make_approach_menu():
    guiMan = getGuiManager()

    def start_approach(approach):
        getMod().approach = approach
        guiMan.removeAll()
        make_control_menu()
        print "selected approach: ", approach

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    approachMenu = gui.create_window(guiMan, 'approaches', Pos2i(0,50), Pos2i(100,170), 'Select Approach')
    approachMenu.setVisibleCloseButton(False)
    #approachMenu.setDraggable(False)

    evolutionButton = gui.create_button(guiMan, 'evolution', Pos2i(0,0), Pos2i(100,30), '')
    evolutionButton.OnMouseLeftClick = lambda: start_approach('evolution')
    evolutionButton.text = 'Evolution'
    approachMenu.addChild(evolutionButton)

    shapingButton = gui.create_button(guiMan, 'shaping', Pos2i(0,30), Pos2i(100,30), '')
    shapingButton.OnMouseLeftClick = lambda: start_approach('shaping')
    shapingButton.text = 'Shaping'
    approachMenu.addChild(shapingButton)

    adviceButton = gui.create_button(guiMan, 'advice', Pos2i(0,60), Pos2i(100,30), '')
    adviceButton.OnMouseLeftClick = lambda: start_approach('advice')
    adviceButton.text = 'Advice'
    approachMenu.addChild(adviceButton)

    exampleButton = gui.create_button(guiMan, 'example', Pos2i(0,90), Pos2i(100,30), '')
    exampleButton.OnMouseLeftClick = lambda: start_approach('example')
    exampleButton.text = 'Example'
    approachMenu.addChild(exampleButton)

    scriptingButton = gui.create_button(guiMan, 'scripting', Pos2i(0,120), Pos2i(100,30), '')
    scriptingButton.OnMouseLeftClick = lambda: start_approach('scripting')
    scriptingButton.text = 'Scripting'
    approachMenu.addChild(scriptingButton)

def make_task_menu():
    guiMan = getGuiManager()

    def start_task(task):
        def closure():
            getMod().task = task
            guiMan.removeAll()
            make_approach_menu()
            reset_objects()
            print "selected task: ", task
        return closure

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    taskMenu = gui.create_window(guiMan, 'tasks', Pos2i(0,50), Pos2i(100,110), 'Select Task')
    taskMenu.setVisibleCloseButton(False)
    #taskMenu.setDraggable(False)

    comboButton = gui.create_button(guiMan, 'combo', Pos2i(0,0), Pos2i(100,30), '')
    comboButton.OnMouseLeftClick = start_task('combo')
    comboButton.text = 'Cube/Wall Combo'
    taskMenu.addChild(comboButton)

    aroundButton = gui.create_button(guiMan, 'around', Pos2i(0,30), Pos2i(100,30), '')
    aroundButton.OnMouseLeftClick = start_task('around')
    aroundButton.text = 'Around the Wall'
    taskMenu.addChild(aroundButton)

    circleButton = gui.create_button(guiMan, 'circle', Pos2i(0,60), Pos2i(100,30), '')
    circleButton.OnMouseLeftClick = start_task('circle')
    circleButton.text = 'Circle Chase'
    taskMenu.addChild(circleButton)

def ClientMain():
    # create fog effect
    getSimContext().setFog()
    
    setup_world()
    
    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 3000
    camZoomSpeed   = 100
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    recenter_cam = recenter(cam) # create a closure to avoid having a global variable
    recenter_cam() # call the recenter function
    getSimContext().addLightSource(Vector3f(1100, -500, 200), 2000)
    make_task_menu()
    # create the key binding
    ioMap = createInputMapping()
    ioMap.BindKey( "KEY_SPACE", "onPress", recenter_cam )
    getSimContext().setInputMapping(ioMap)

def recenter(cam):
    def closure():
        cam.setPosition(Vector3f(1000, -120, 100))
        cam.setTarget(Vector3f(100, 100, 10))
    return closure

def setup_world():
    addObject("data/terrain/Sea.xml", Vector3f(-3000,-3000,-20))
    addObject("data/terrain/IslandTerrain.xml", Vector3f(-1100, -2400, -17), Vector3f(0,0,1))
    addSkyBox("data/sky/irrlicht2")
