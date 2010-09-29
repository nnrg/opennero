from OpenNero import *
from module import getMod, delMod
from common import *
import common.gui as gui
from inputConfig import *

def toggle_ai_callback():
    global toggleAiButton
    toggle_ai()
    if toggleAiButton.text == 'Begin Battle':
     from Battle.module import parseInput
     data = script_server.read_data()
     while data:
         parseInput(data.strip())
         data = script_server.read_data()
     getMod().start_rtneat()
     reset_ai()
     toggleAiButton.text = 'Toggle AI'

def save_ai_call():
    getMod().save_rtneat()

def load_ai_call():
    getMod().load_rtneat()

def recenter(cam):
    def closure():
        cam.setPosition(Vector3f(0, 0, 100))
        cam.setTarget(Vector3f(100,100,0))
    return closure

#########################################################

def CreateGui(guiMan): 
    global toggleAiButton
    global mode
    mode = 0

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")  
    guiWindow = gui.create_window( guiMan, 'window', Pos2i(20,20),Pos2i(80,100), 'Nero Controls' )

    toggleAiButton = gui.create_button( guiMan, 'toggle_ai', Pos2i(0,0),Pos2i(60,80), '' )
    toggleAiButton.text = 'Begin Battle'
    toggleAiButton.OnMouseLeftClick = toggle_ai_callback
    
    guiWindow.addChild(toggleAiButton)

def weight_adjusted(scroll, key, value):
    result = scroll.getPos() - 100
    value.text = str(result)
    getMod().set_weight(key, float(result)/100)
    def closure():
       result = scroll.getPos() - 100
       value.text = str(result)
       getMod().set_weight(key, float(result)/100)
    return closure
    return 0

def lt_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    value.text = str(scroll.getPos())
    getMod().ltChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().ltChange(float(scroll.getPos()))
    return closure

def dta_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    value.text = str(scroll.getPos())
    getMod().dtaChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().dtaChange(float(scroll.getPos()))
    return closure

def dtb_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    value.text = str(scroll.getPos())
    getMod().dtbChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().dtbChange(float(scroll.getPos()))
    return closure

def dtc_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    value.text = str(scroll.getPos())
    getMod().dtcChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().dtcChange(float(scroll.getPos()))
    return closure

def ff_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    result = scroll.getPos() - 100
    value.text = str(result)
    getMod().ffChange(float(result)/100)
    def closure():
        result = scroll.getPos() - 100
        value.text = str(result)
        getMod().ffChange(float(result)/100)
    return closure

def ee_adjusted(scroll, value):
    # this returns a callback function for reacting to the changing epsilon value
    value.text = str(scroll.getPos())
    getMod().eeChange(float(scroll.getPos())/100)
    def closure():
        value.text = str(scroll.getPos())
        getMod().eeChange(float(scroll.getPos())/100)
    return closure

def hp_adjusted(scroll, value):
    value.text = str(scroll.getPos())
    getMod().hpChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().hpChange(scroll.getPos())
    return closure

def sp_adjusted(scroll, value):
    value.text = str(scroll.getPos())
    getMod().hpChange(float(scroll.getPos()))
    def closure():
        value.text = str(scroll.getPos())
        getMod().set_speedup(scroll.getPos())
    return closure

def mode_adjusted(scroll):
    def closure():
     global mode
     cmode = scroll.getPos()
     if cmode == 0:
       print "Training Mode"
       global stat1Label, stat2Label, stat1Value, stat2Value
       #stat1Label.text = "Team 1 Avg Fitness:"
       #stat2Label.text = "Team 2 Avg Fitness:"
       #stat1Value.text = "n/a"
       #stat2Value.text = "n/a"
       mode = 0
     else:
        mode = 1
        print "Battle Mode"
        print mode
        getMod().hpChange(1)
        getMod().eeChange(1.0)
        getMod().ltChange(0)
        global value1, value2, stat1Label, stat2Label
        #stat1Label.text = "Team 1 Score:"
        #stat2Label.text = "Team 2 Score:"
        value1 = 6
        value2 = 6
        set_stat(1,1)
        set_stat(1,2)
    return closure    

def set_stat(value, team):
    global stat1Value, stat2Value
    global mode
    global value1,value2
    print "set_stat mode: " + str(mode)
    """
    if mode == 0:
     if team == 1:
      stat1Value.text = str(value)
     else:
      stat2Value.text = str(value)
    if mode == 1:
      if team == 1:
          print value1
          stat1Value.text = str(value1 - 1)
      else:
          print value2
          stat2Value.text = str(value2 - 1)
    if mode == 2:
        stat1Value.text = "n/a"
        stat2Value.text = "n/a"
    """

def ClientMain():
    # physics off, ai off by default
    #disable_physics()
    disable_ai()

    getMod().setup_map()

    # add a light source
    getSimContext().addLightSource(Vector3f(500,-500,1000), 1500)

    addSkyBox("data/sky/irrlicht2")

    # setup the gui
    CreateGui(getGuiManager())

    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 15000
    camZoomSpeed   = 200
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(40000)
    cam.setEdgeScroll(False)
    recenter_cam = recenter(cam)
    recenter_cam()

    # create the io map
    ioMap = createInputMapping()
    ioMap.BindKey( "KEY_SPACE", "onPress", recenter_cam )
    getSimContext().setInputMapping(ioMap)
