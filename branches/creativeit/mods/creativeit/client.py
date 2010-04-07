from OpenNero import *
from random import seed, randint

from math import *

from inputConfig import createInputMapping
from common import *
import common.gui as gui
from creativeit.module import getMod, delMod
from creativeit.constants import *

#########################################################

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
