from OpenNero import *

# an empty function
def blank():
    pass
    
def createInputMapping():

    # create an io map
    ioMap = PyIOMap()

    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", "switchToHub()" )
    ioMap.BindKey( "KEY_CONTROL+KEY_KEY_A", "onPress", "blank()" )
    ioMap.BindKey( "KEY_KEY_A", "onPress", "blank()" )
    ioMap.BindKey( "KEY_SPACE", "onPress", "recenter()" )
    ioMap.BindKey( "KEY_F5", "onPress", "toggle_tracing()" )
    ioMap.BindKey( "KEY_F8", "onPress", "enable_simdisplay()" )
    ioMap.BindKey( "KEY_F9", "onPress", "disable_simdisplay()" )
    ioMap.BindMouseAction( "moveX", "mouse_action()" )
    ioMap.BindMouseAction( "moveY", "mouse_action()" )
    ioMap.BindMouseButton( "left", "onPress", "reset_mouse_action()" )
    ioMap.BindMouseButton( "left", "onRelease", "blank()" )
    ioMap.BindMouseButton( "middle", "onPress", "blank()" )
    ioMap.BindMouseButton( "middle", "onRelease", "blank()" )
    ioMap.BindMouseButton( "right", "onPress", "show_context_menu()" )
    ioMap.BindMouseButton( "right", "onRelease", "blank()" )
    ioMap.BindMouseAction( "scroll", "blank()" )

    # FPS control
    #ioMap.BindKey( "KEY_KEY_A", "onHold", "fps_strafe_left()" )
    #ioMap.BindKey( "KEY_KEY_D", "onHold", "fps_strafe_right()" )
    #ioMap.BindKey( "KEY_KEY_S", "onHold", "fps_back()" )
    #ioMap.BindKey( "KEY_KEY_W", "onHold", "fps_forward()" )
    #ioMap.BindKey( "KEY_KEY_Q", "onHold", "fps_turn_left()" )
    #ioMap.BindKey( "KEY_KEY_E", "onHold", "fps_turn_right()" )

    # Keyboard control
    ioMap.BindKey( "KEY_RIGHT", "onHold", "key_right()" )
    ioMap.BindKey( "KEY_LEFT", "onHold", "key_left()" )
    ioMap.BindKey( "KEY_UP", "onHold", "key_forward()" )
    ioMap.BindKey( "KEY_DOWN", "onHold", "key_back()" )
    
    return ioMap
