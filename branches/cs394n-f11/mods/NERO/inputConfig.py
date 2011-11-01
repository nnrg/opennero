import OpenNero
import common
import module
import constants

def switchToHub():
    module.delMod()
    OpenNero.switchMod('hub', 'hub:common')

def blank():
    pass

def toggleDisplayHint():
    constants.nextDisplayHint()

def createInputMapping():
    from client import show_context_menu, mouse_action, reset_mouse_action
    # create an io map
    ioMap = OpenNero.PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub)
    ioMap.BindKey( "KEY_F1", "onPress", common.openWiki('NeroMod') )
    ioMap.BindKey( "KEY_F2", "onPress", toggleDisplayHint )

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
