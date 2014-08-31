from OpenNero import *

from module import delMod

def switchToHub():
    delMod()
    switchMod('hub', '', 'hub:common')

def createInputMapping():

    # create an io map
    ioMap = PyIOMap()

    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onHold", switchToHub )
    from common.module import openWiki
    ioMap.BindKey( "KEY_F1", "onPress", openWiki('RoombaMod') )
    return ioMap
