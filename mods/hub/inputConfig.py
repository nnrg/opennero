from OpenNero import *

# an empty function
def blank():
    pass
    
def createInputMapping():
    ioMap = PyIOMap()

    # bind our keys
    ioMap.BindKey( "KEY_CONTROL+KEY_KEY_A", "onPress", blank )
    ioMap.BindKey( "KEY_KEY_A", "onPress", blank )
    ioMap.BindMouseAction( "moveX", blank )
    ioMap.BindMouseAction( "moveY", blank )
    ioMap.BindMouseButton( "left", "onPress", blank )
    ioMap.BindMouseButton( "left", "onRelease", blank )
    ioMap.BindMouseButton( "middle", "onPress", blank )
    ioMap.BindMouseButton( "middle", "onRelease", blank )
    ioMap.BindMouseButton( "right", "onPress", blank )
    ioMap.BindMouseButton( "right", "onRelease", blank )
    ioMap.BindMouseAction( "scroll", blank )
    
    return ioMap
