from OpenNero import *

from common.module import openWiki
from module import delMod

def switchToHub():
    delMod()
    switchMod('hub', 'hub:common')

def createInputMapping():
    # create an io map
    ioMap = PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub)
    ioMap.BindKey( "KEY_F1", "onPress", openWiki('NeroMod') )
    return ioMap
