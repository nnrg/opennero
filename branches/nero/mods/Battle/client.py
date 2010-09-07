from OpenNero import *
from module import getMod, delMod
from NeroEnvironment import Fitness
from common import *
import common.gui as gui
from inputConfig import *
from module import getReader

def toggle_ai_callback():
    global toggleAiButton
    toggle_ai()
    if toggleAiButton.text == 'Begin Battle':
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

    x = getReader() 

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")  
    guiWindow = gui.create_window( guiMan, 'window', Pos2i(20,20),Pos2i(80,100), 'Nero Controls' )

    toggleAiButton = gui.create_button( guiMan, 'toggle_ai', Pos2i(0,0),Pos2i(60,80), '' )
    toggleAiButton.text = 'Begin Battle'
    toggleAiButton.OnMouseLeftClick = toggle_ai_callback
    
    """
    saveAiButton = gui.create_button(guiMan, 'save_ai', Pos2i(0,80),Pos2i(30,80), '')
    saveAiButton.text = 'Save'
    saveAiButton.OnMouseLeftClick = save_ai_call

    loadAiButton = gui.create_button(guiMan, 'load_ai', Pos2i(30,80),Pos2i(30,80),'')
    loadAiButton.text = 'Load'
    loadAiButton.OnMouseLeftClick = load_ai_call


    #Stand Ground Scroll Data
    sgText = gui.create_text(guiMan, 'sgText', Pos2i(65,-2), Pos2i(85,15), Fitness.STAND_GROUND)
    sgValue = gui.create_text(guiMan, 'sgValue', Pos2i(310,-2), Pos2i(20,15), '0')
    sgScroll = gui.create_scroll_bar(guiMan, 'sgScroll', Pos2i(150, 0), Pos2i(150,10), True)
    sgScroll.setMax(200)
    sgScroll.setLargeStep(10)
    sgScroll.setSmallStep(1)
    sgScroll.setPos(100)
    sgScroll.OnScrollBarChange = weight_adjusted(sgScroll, Fitness.STAND_GROUND, sgValue)

    #Stick Together Scroll Data
    stText = gui.create_text(guiMan, 'stText', Pos2i(65,12), Pos2i(85,15), Fitness.STICK_TOGETHER)
    stValue = gui.create_text(guiMan, 'stValue', Pos2i(310,12), Pos2i(20,15), '0')
    stScroll = gui.create_scroll_bar(guiMan, 'stScroll', Pos2i(150, 15), Pos2i(150,10), True)
    stScroll.setMax(200)
    stScroll.setLargeStep(10)
    stScroll.setSmallStep(1)
    stScroll.setPos(100)
    stScroll.OnScrollBarChange = weight_adjusted(stScroll, Fitness.STICK_TOGETHER, stValue)

    dtaText = gui.create_text(guiMan, 'dtaText', Pos2i(350,12), Pos2i(50,15),"Distance")
    dtaValue = gui.create_text(guiMan, 'dtaValue', Pos2i(560,12), Pos2i(20,15), '50')
    dtaScroll = gui.create_scroll_bar(guiMan, 'dtaScroll', Pos2i(400, 15), Pos2i(150,10), True)
    dtaScroll.setMax(100)
    dtaScroll.setLargeStep(10)
    dtaScroll.setSmallStep(1)
    dtaScroll.setPos(50)
    dtaScroll.OnScrollBarChange = dta_adjusted(dtaScroll, dtaValue)

    #Approach Enemy Scroll Data
    aeText = gui.create_text(guiMan, 'aeText', Pos2i(65,27), Pos2i(85,15), Fitness.APPROACH_ENEMY)
    aeValue = gui.create_text(guiMan, 'aeValue', Pos2i(310,27), Pos2i(20,15), '0')
    aeScroll = gui.create_scroll_bar(guiMan, 'aeScroll', Pos2i(150, 30), Pos2i(150,10), True)
    aeScroll.setMax(200)
    aeScroll.setLargeStep(10)
    aeScroll.setSmallStep(1)
    aeScroll.setPos(100)
    aeScroll.OnScrollBarChange = weight_adjusted(aeScroll, Fitness.APPROACH_ENEMY, aeValue)


    dtbText = gui.create_text(guiMan, 'dtbText', Pos2i(350,27), Pos2i(50,15),"Distance")
    dtbValue = gui.create_text(guiMan, 'dtbValue', Pos2i(560,27), Pos2i(20,15), '50')
    dtbScroll = gui.create_scroll_bar(guiMan, 'dtbScroll', Pos2i(400, 30), Pos2i(150,10), True)
    dtbScroll.setMax(100)
    dtbScroll.setLargeStep(10)
    dtbScroll.setSmallStep(1)
    dtbScroll.setPos(50)
    dtbScroll.OnScrollBarChange = dtb_adjusted(dtbScroll, dtbValue)

    #Approach Flag Scroll Data
    afText = gui.create_text(guiMan, 'afText', Pos2i(65,42), Pos2i(85,15), Fitness.APPROACH_FLAG)
    afValue = gui.create_text(guiMan, 'afValue', Pos2i(310,42), Pos2i(20,15), '0')
    afScroll = gui.create_scroll_bar(guiMan, 'afScroll', Pos2i(150, 45), Pos2i(150,10), True)
    afScroll.setMax(200)
    afScroll.setLargeStep(10)
    afScroll.setSmallStep(1)
    afScroll.setPos(200)
    afScroll.OnScrollBarChange = weight_adjusted(afScroll, Fitness.APPROACH_FLAG, afValue)


    dtcText = gui.create_text(guiMan, 'dtcText', Pos2i(350,42), Pos2i(50,15),"Distance")
    dtcValue = gui.create_text(guiMan, 'dtcValue', Pos2i(560,42), Pos2i(20,15), '50')
    dtcScroll = gui.create_scroll_bar(guiMan, 'dtcScroll', Pos2i(400, 45), Pos2i(150,10), True)
    dtcScroll.setMax(100)
    dtcScroll.setLargeStep(10)
    dtcScroll.setSmallStep(1)
    dtcScroll.setPos(50)
    dtcScroll.OnScrollBarChange = dtc_adjusted(dtcScroll, dtcValue)

    #Hit Target Scroll Data
    htText = gui.create_text(guiMan, 'htText', Pos2i(65,57), Pos2i(85,15), Fitness.HIT_TARGET)
    htValue = gui.create_text(guiMan, 'htValue', Pos2i(310,57), Pos2i(20,15), '0')
    htScroll = gui.create_scroll_bar(guiMan, 'htScroll', Pos2i(150, 60), Pos2i(150,10), True)
    htScroll.setMax(200)
    htScroll.setLargeStep(10)
    htScroll.setSmallStep(1)
    htScroll.setPos(100)
    htScroll.OnScrollBarChange = weight_adjusted(htScroll, Fitness.HIT_TARGET, htValue)

    #Avoid Fire Scroll Data
    vfText = gui.create_text(guiMan, 'vfText', Pos2i(65,72), Pos2i(85,15), Fitness.AVOID_FIRE)
    vfValue = gui.create_text(guiMan, 'vfValue', Pos2i(310,72), Pos2i(20,15), '0')
    vfScroll = gui.create_scroll_bar(guiMan, 'vfScroll', Pos2i(150, 75), Pos2i(150,10), True)
    vfScroll.setMax(200)
    vfScroll.setLargeStep(10)
    vfScroll.setSmallStep(1)
    vfScroll.setPos(100)
    vfScroll.OnScrollBarChange = weight_adjusted(vfScroll, Fitness.AVOID_FIRE, vfValue)

    #Lifetime Scroll Data
    ltText = gui.create_text(guiMan, 'ltText', Pos2i(65,92), Pos2i(85,15), "Lifetime")
    ltValue = gui.create_text(guiMan, 'ltValue', Pos2i(310,92), Pos2i(20,15), '10')
    ltScroll = gui.create_scroll_bar(guiMan, 'ltScroll', Pos2i(150, 95), Pos2i(150,10), True)
    ltScroll.setMax(100)
    ltScroll.setLargeStep(10)
    ltScroll.setSmallStep(1)
    ltScroll.setPos(10)
    ltScroll.OnScrollBarChange = lt_adjusted(ltScroll, ltValue)

    #Friendly Fire Scroll Data
    ffText   = gui.create_text(guiMan, 'ffText',Pos2i(65,107),Pos2i(85,15),'Friendly Fire')
    ffValue = gui.create_text(guiMan, 'ffValue', Pos2i(310,107), Pos2i(20,15), '0')
    ffScroll = gui.create_scroll_bar(guiMan, 'ffScroll', Pos2i(150, 110), Pos2i(150,10), True)
    ffScroll.setMax(200)
    ffScroll.setLargeStep(10)
    ffScroll.setSmallStep(1)
    ffScroll.setPos(100)
    ffScroll.OnScrollBarChange = ff_adjusted(ffScroll, ffValue)

    #Explore or Exploit Scroll Data
    eeText = gui.create_text(guiMan, 'eeText', Pos2i(65,122), Pos2i(85,15),'Explore/Exploit')
    eeValue = gui.create_text(guiMan, 'eeValue', Pos2i(310,122), Pos2i(20,15), '50')
    eeScroll = gui.create_scroll_bar(guiMan, 'eeScroll', Pos2i(150, 125), Pos2i(150,10), True)
    eeScroll.setMax(100)
    eeScroll.setLargeStep(10)
    eeScroll.setSmallStep(1)
    eeScroll.setPos(50)
    eeScroll.OnScrollBarChange = ee_adjusted(eeScroll, eeValue)

    #Hitpoint Scroll Data
    hpText = gui.create_text(guiMan, 'hpText', Pos2i(65,137), Pos2i(85,15),'Hitpoints')
    hpValue = gui.create_text(guiMan, 'hpValue', Pos2i(310,137), Pos2i(20,15), '50')
    hpScroll = gui.create_scroll_bar(guiMan, 'hpScroll', Pos2i(150, 140), Pos2i(150,10), True)
    hpScroll.setMax(100)
    hpScroll.setLargeStep(10)
    hpScroll.setSmallStep(1)
    hpScroll.setPos(5)
    hpScroll.OnScrollBarChange = hp_adjusted(hpScroll, hpValue)

    #Speedup Scroll Data
    spText = gui.create_text(guiMan, 'spText', Pos2i(65,152), Pos2i(85,15), 'Speedup')
    spValue = gui.create_text(guiMan, 'spValue', Pos2i(310,152),Pos2i(20,15), '0') 
    spScroll = gui.create_scroll_bar(guiMan, 'spScroll', Pos2i(150,155), Pos2i(150,10), True)
    spScroll.setMax(100)
    spScroll.setLargeStep(10)
    spScroll.setSmallStep(1)
    spScroll.setPos(0)
    spScroll.OnScrollBarChange = sp_adjusted(spScroll,spValue)

    global stat1Value, stat2Value, stat1Label, stat2Label

    #stat1Label = gui.create_text(guiMan, 'stat1Label', Pos2i(350,90),Pos2i(110,15),'Team 1: Avg Fitness: ')
    #stat1Value = gui.create_text(guiMan, 'stat1Value', Pos2i(470,90),Pos2i(85,15),'n/a')

    #stat2Label = gui.create_text(guiMan, 'stat2Label', Pos2i(350,105),Pos2i(110,15),'Team 2: Avg Fitness: ')
    #stat2Value = gui.create_text(guiMan, 'stat2Label', Pos2i(470,105),Pos2i(85,15),'n/a')

    trainText = gui.create_text(guiMan, 'trainText', Pos2i(350,130), Pos2i(40,30), "Train")
    fightText = gui.create_text(guiMan, 'fightText', Pos2i(520,130), Pos2i(100,30), "Fight")
    
    modeScroll = gui.create_scroll_bar(guiMan, 'modeScroll', Pos2i (390,130), Pos2i(120,30), True)    
    modeScroll.setMax(1)
    modeScroll.setPos(0)
    modeScroll.OnScrollBarChange = mode_adjusted(modeScroll)


    guiWindow.addChild(saveAiButton)
    guiWindow.addChild(loadAiButton)
    guiWindow.addChild(sgScroll)
    guiWindow.addChild(stScroll)
    guiWindow.addChild(aeScroll)
    guiWindow.addChild(afScroll)
    guiWindow.addChild(htScroll)
    guiWindow.addChild(vfScroll)
    guiWindow.addChild(ltScroll)
    guiWindow.addChild(dtaScroll)
    guiWindow.addChild(dtbScroll)
    guiWindow.addChild(dtcScroll)
    guiWindow.addChild(ffScroll)
    guiWindow.addChild(eeScroll)
    guiWindow.addChild(hpScroll)
    guiWindow.addChild(spScroll)
    guiWindow.addChild(sgText)
    guiWindow.addChild(stText)
    guiWindow.addChild(aeText)
    guiWindow.addChild(afText)
    guiWindow.addChild(htText)
    guiWindow.addChild(vfText)
    guiWindow.addChild(ltText)
    guiWindow.addChild(dtaText)
    guiWindow.addChild(dtbText)
    guiWindow.addChild(dtcText)
    guiWindow.addChild(ffText)
    guiWindow.addChild(eeText)
    guiWindow.addChild(hpText)
    guiWindow.addChild(spText)
    guiWindow.addChild(sgValue)
    guiWindow.addChild(stValue)
    guiWindow.addChild(aeValue)
    guiWindow.addChild(afValue)
    guiWindow.addChild(htValue)
    guiWindow.addChild(vfValue)
    guiWindow.addChild(ltValue)
    guiWindow.addChild(dtaValue)
    guiWindow.addChild(dtbValue)
    guiWindow.addChild(dtcValue)
    guiWindow.addChild(ffValue)
    guiWindow.addChild(eeValue)
    guiWindow.addChild(hpValue)
    guiWindow.addChild(spValue)
    #guiWindow.addChild(stat1Label)
    #guiWindow.addChild(stat1Value)
    #guiWindow.addChild(stat2Label)
    #guiWindow.addChild(stat2Value)
    guiWindow.addChild(modeScroll)
    guiWindow.addChild(fightText)
    guiWindow.addChild(trainText)
    """
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

    
