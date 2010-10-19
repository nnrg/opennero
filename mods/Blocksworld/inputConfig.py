from OpenNero import *

from common.module import openWiki
from module import delMod

def switchToHub():
    delMod()
    switchMod('hub', 'hub:common')

def blank():
    pass

def createInputMapping():
    from client import show_context_menu, mouse_action, reset_mouse_action
    # create an io map
    ioMap = PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub)
    ioMap.BindKey( "KEY_F1", "onPress", openWiki('BlocksworldMod') )
    
    ioMap.BindMouseAction( "moveX", mouse_action)
    ioMap.BindMouseAction( "moveY", mouse_action)
    ioMap.BindMouseButton( "left"  , "onPress"    , reset_mouse_action)
    ioMap.BindMouseButton( "right" , "onRelease"  , blank)
    ioMap.BindMouseButton( "middle", "onPress"    , blank)
    ioMap.BindMouseButton( "middle", "onRelease"  , blank)
    ioMap.BindMouseButton( "right" , "onPress"    , show_context_menu)
    ioMap.BindMouseButton( "right" , "onRelease"  , blank)

    ioMap.BindMouseAction( "scroll", blank)

    return ioMap
