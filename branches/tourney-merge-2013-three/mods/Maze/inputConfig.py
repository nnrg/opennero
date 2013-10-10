from OpenNero import *

from common.module import openWiki
from Maze.module import getMod, delMod

# functions bound to key/mouse events
def switchToHub():
    delMod()
    switchMod('hub', 'hub:common')

def first_person_control(command):
    # get the controller action
    def closure():
        # controller action
        getMod().control_fps(command)
    return closure

# create the binding
def createInputMapping():
    # create an io map
    ioMap = PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub )
    # FPS control
    ioMap.BindKey( "KEY_KEY_A", "onPress", first_person_control('CCW') )
    ioMap.BindKey( "KEY_KEY_D", "onPress", first_person_control('CW') )
    ioMap.BindKey( "KEY_KEY_W", "onPress", first_person_control('FWD') )
    ioMap.BindKey( "KEY_KEY_S", "onPress", first_person_control('BCK') )
    ioMap.BindKey( "KEY_LEFT", "onPress", first_person_control('CCW') )
    ioMap.BindKey( "KEY_RIGHT", "onPress", first_person_control('CW') )
    ioMap.BindKey( "KEY_UP", "onPress", first_person_control('FWD') )
    ioMap.BindKey( "KEY_DOWN", "onPress", first_person_control('BCK') )
    ioMap.BindKey( "KEY_F1", "onPress", openWiki('MazeMod') )
    ioMap.BindKey( "KEY_KEY_H", "onPress", openWiki('MazeMod') )
    return ioMap
