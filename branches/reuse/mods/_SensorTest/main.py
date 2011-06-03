# when the mod is loaded, OpenNERO will import this module and call ModMain

# generic imports
from OpenNero import *
from common import *
from random import uniform
from math import *

# module specific imports
from SensorTest.environment import SensorTestEnvironment
from SensorTest.constants import *
from SensorTest import agent

def ModMain():
    """
    Main function called by OpenNERO C++ code

    This particular one sets up an environment, an agent and some test objects
    in order to test the built-in sensors.
    """

    # stop the AI loop if it is running
    disable_ai()

    # add a camera
    camRotateSpeed = 500
    camMoveSpeed   = 1500
    camZoomSpeed   = 100
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setPosition(Vector3f(100, 100, 50))
    cam.setTarget(Vector3f(1, 1, 1))
    cam.setFarPlane(1000)
    cam.setEdgeScroll(False)
    
    # set the environment
    set_environment(SensorTestEnvironment())
    
    # add the agent
    addObject("agent.xml", Vector3f(0,0,0), type = OBJECT_TYPE_AGENT)
    
    # TESTING: add the sensed objects
    for i in xrange(N_OBJECTS):
        a = radians(360.0 * i / N_OBJECTS)
        xyz = Vector3f(RADIUS * cos(a), RADIUS * sin(a), 0)
        addObject("data/shapes/cube/RedCube.xml", xyz, type = OBJECT_TYPE_SENSED)

    # TESTING: an object in front of the agent
    addObject("data/shapes/cube/GreenCube.xml", Vector3f(0.5 * RADIUS, 0, 0), type = OBJECT_TYPE_FORWARD)
    
    # TESTING: add other random objects
    for i in xrange(N_OBJECTS * 5):
        xyz = Vector3f(uniform(-RADIUS, RADIUS), uniform(-RADIUS, RADIUS), 0)
        addObject("data/shapes/cube/BlueCube.xml", xyz, type = OBJECT_TYPE_OTHER)
    
    # start the ai loop
    enable_ai()
