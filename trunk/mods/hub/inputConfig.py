from OpenNero import *

def createInputMapping():
    ioMap = PyIOMap()
    ioMap.ClearMappings()
    # bind our keys
    from common.module import openWiki
    ioMap.BindKey("KEY_F1", "onPress", openWiki('RunningOpenNero'))
    ioMap.BindKey("KEY_ESCAPE", "onHold", lambda: getSimContext().killGame())
    return ioMap
