from OpenNero import *

from module import delMod

def switchToHub():
    delMod()
    switchMod('hub', 'hub:common')

def createInputMapping():
    # create an io map
    ioMap = PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress",  switchToHub)
    return ioMap
