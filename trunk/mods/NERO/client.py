from OpenNero import *
from module import getMod, delMod
from NeroEnvironment import Fitness
from common import *
import common.gui as gui
from inputConfig import *

def toggle_ai_callback():
    global toggleAiButton
    toggle_ai()
    if toggleAiButton.text == 'Deploy':
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

def show_context_menu():
    global modify_object_id
    global object_ids

    if len(modify_object_id) > 0:
        return

    x = getScriptOutput('NERO/menu.py') 

    guiMan.setTransparency(1.0)
    
    sim_context = getSimContext()

    # find the screen position at which to open the context menu
    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)
    selected_object_id = sim_context.getClickedEntityId(cursor.x, cursor.y)

    print "location:", location

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    def add_wall():
        object_ids.append(addObject("data/shapes/cube/Cube.xml", Vector3f(location.x,location.y,0), Vector3f(0, 0, 90), scale=Vector3f(1,30,5), label="World Wall0", type = 1 ))

    def rotate_object():
        modify_object_id['rot'] = selected_object_id

    def scale_object():
        modify_object_id['scale'] = selected_object_id

    def move_object():
        modify_object_id['move'] = selected_object_id

    def place_flag():
        from module import getMod
        getMod().change_flag([location.x,location.y,0])

    def remove_wall():
        removeObject(selected_object_id)

    if selected_object_id in object_ids:
        rotateButton = gui.create_button(guiMan, 'rotate', Pos2i(0,0), Pos2i(0,0), '')
        rotateButton.OnMouseLeftClick = lambda: rotate_object()
        contextMenu.addItem('Rotate Object', rotateButton)

        scaleButton = gui.create_button(guiMan, 'scale', Pos2i(0,0), Pos2i(0,0), '')
        scaleButton.OnMouseLeftClick = lambda: scale_object()
        contextMenu.addItem('Scale Object', scaleButton)
        
        moveButton = gui.create_button(guiMan, 'move', Pos2i(0,0), Pos2i(0,0), '')
        moveButton.OnMouseLeftClick = lambda: move_object()
        contextMenu.addItem('Move Object', moveButton)

        removeButton = gui.create_button(guiMan, 'remove', Pos2i(0,0), Pos2i(0,0), '')
        removeButton.OnMouseLeftClick = lambda: remove_wall()
        contextMenu.addItem('Remove Object', removeButton)

    else:
        wallButton = gui.create_button(guiMan, 'wall', Pos2i(0,0), Pos2i(0,0), '')
        wallButton.OnMouseLeftClick = lambda: add_wall()
        contextMenu.addItem('Add wall', wallButton)

        flagButton = gui.create_button(guiMan, 'flag', Pos2i(0,0), Pos2i(0,0), '')
        flagButton.OnMouseLeftClick = lambda: place_flag()
        contextMenu.addItem('Place Flag', flagButton)

def reset_mouse_action():
    global modify_object_id
    modify_object_id = {}

def mouse_action():
    import math
    global modify_object_id
    global object_ids

    if len(modify_object_id) == 0:
        return

    sim_context = getSimContext()

    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)


    if 'move' in modify_object_id:
       sim_context.setObjectPosition(modify_object_id['move'], Vector3f(location.x, location.y, 0))

    if 'rot' in modify_object_id:
        position = sim_context.getObjectPosition(modify_object_id['rot'])
        angle = math.atan2(location.x-position.x, location.y-position.y)
        sim_context.setObjectRotation(modify_object_id['rot'], Vector3f(0,0,-math.degrees(angle)))

    if 'scale' in modify_object_id:
        position = sim_context.getObjectPosition(modify_object_id['scale'])
        rotation = sim_context.getObjectRotation(modify_object_id['scale'])
        theta = math.radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x-position.x)*math.cos(theta) + (location.y-position.y)*math.sin(theta)
        localy = -(location.x-position.x)*math.sin(theta) + (location.y-position.y)*math.cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + math.fabs(localx)/10
        scaley = 1 + math.fabs(localy)/10

        if localx < 0: scalex = 1/scalex
        if localy < 0: scaley = 1/scaley

        sim_context.setObjectScale(modify_object_id['scale'],Vector3f(scalex,scaley,20))

#########################################################

def CreateGui(guim): 
    global toggleAiButton
    global mode
    global modify_object_id
    global object_ids
    global guiMan
    mode = 0

    guiMan = guim
    object_ids = []
    modify_object_id = {}

    #x = getReader() 

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")  
    guiWindow = gui.create_window( guiMan, 'window', Pos2i(20,20),Pos2i(80,100), 'Nero Controls' )

    toggleAiButton = gui.create_button( guiMan, 'toggle_ai', Pos2i(0,0),Pos2i(60,80), '' )
    toggleAiButton.text = 'Deploy'
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

    if not getMod().setup_map():
        switchToHub()
        return

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

    
