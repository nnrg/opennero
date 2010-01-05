from OpenNero import *

def createInputMapping():
    ioMap = PyIOMap()
    # bind our keys
    from common.module import openWiki
    ioMap.BindKey( "KEY_F1", "onPress", openWiki('RunningOpenNero') )
    return ioMap
