from OpenNero import *
from random import seed

# add the key and mouse bindings
from inputConfig import *

# add network utils
from common import *

from module import getMod, delMod
    
### called from gui elements ############################

def toggle_ai_callback(pauseButton):
    """ pause and resume all AI agents """
    toggle_ai()
    if pauseButton.text == 'Pause!':
        disable_ai()
        pauseButton.text = 'Resume!'
    else:
        pauseButton.text = 'Pause!'
        reset_ai()

def toggle_bot_type(changeBotButton, botTypeBox):
    if botTypeBox.text.lower().find('script') >= 0:
        botTypeBox.text = 'rtNEAT'
        changeBotButton.text = 'Switch to Script'
    else:
        botTypeBox.text = 'Script'
        changeBotButton.text = 'Switch to rtNEAT'

def toggle_rtneat():
    " XXX "
    return True
    
#########################################################

### CREATIVEIT: context menu ############################

def show_context_menu():
    global clientContext
    if len(getSimContext().modify_object_id) > 0:
        return

    guiMan = getSimContext().getGuiManager()
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    # find the screen position at which to open the context menu
    cursor = getSimContext().getMousePosition()
    location = getSimContext().getClickPosition(cursor.x, cursor.y)
    selected_object_id = getSimContext().getEntityNetIdUnderMouse(cursor.x, cursor.y)

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    # activate the selected object to be moved
    def move_object():
        getSimContext().modify_object_id['pos'] = selected_object_id

    # activate the selected object to be rotated
    def rotate_object():
        getSimContext().modify_object_id['rot'] = selected_object_id

    # activate the selected object to be scaled
    def scale_object():
        getSimContext().modify_object_id['scale'] = selected_object_id

    # activate the selected object to be colored
    def color_object():
        getSimContext().modify_object_id['color'] = selected_object_id

    # remove the selected object from the world
    def remove_object():
        getSimContext().object_ids.remove(selected_object_id)
        getSimContext().getMod().removeObject(selected_object_id)

    # add a wall at the current cursor location
    def add_wall():
        getSimContext().object_ids.add(getSimContext().getNextFreeNetId())
        getSimContext().getMod().addWall(Vector3f(location.x, location.y, getSimContext().object_z))

    # add a cube at the current cursor location
    def add_cube():
        cube_id = getSimContext().getNextFreeNetId()
        getSimContext().object_ids.add(cube_id)
        getSimContext().getMod().addCube(Vector3f(location.x, location.y, getSimContext().object_z))

    # if the user clicked on an object they added previously, show the options
    # to modify it; otherwise show options to add new objects
    if selected_object_id in getSimContext().object_ids:
        moveButton = gui.create_button(guiMan, 'move', Pos2i(0,0), Pos2i(0,0), '')
        moveButton.OnMouseLeftClick = lambda: move_object()
        contextMenu.addItem('Move object', moveButton)

        rotateButton = gui.create_button(guiMan, 'rotate', Pos2i(0,0), Pos2i(0,0), '')
        rotateButton.OnMouseLeftClick = lambda: rotate_object()
        contextMenu.addItem('Rotate object', rotateButton)

        scaleButton = gui.create_button(guiMan, 'scale', Pos2i(0,0), Pos2i(0,0), '')
        scaleButton.OnMouseLeftClick = lambda: scale_object()
        contextMenu.addItem('Scale object', scaleButton)

        colorButton = gui.create_button(guiMan, 'color', Pos2i(0,0), Pos2i(0,0), '')
        colorButton.OnMouseLeftClick = lambda: color_object()
        contextMenu.addItem('Color object', colorButton)

        removeButton = gui.create_button(guiMan, 'remove', Pos2i(0,0), Pos2i(0,0), '')
        removeButton.OnMouseLeftClick = lambda: remove_object()
        contextMenu.addItem('Remove object', removeButton)

    else:
        wallButton = gui.create_button(guiMan, 'wall', Pos2i(0,0), Pos2i(0,0), '')
        wallButton.OnMouseLeftClick = lambda: add_wall()
        contextMenu.addItem('Add wall', wallButton)

        cubeButton = gui.create_button(guiMan, 'cube', Pos2i(0,0), Pos2i(0,0), '')
        cubeButton.OnMouseLeftClick = lambda: add_cube()
        contextMenu.addItem('Add cube', cubeButton)

        rtneatButton = gui.create_button(guiMan, 'rtneat', Pos2i(0,0), Pos2i(0,0), '')
        rtneatButton.OnMouseLeftClick = lambda: getSimContext().getMod().start_rtneat()
        contextMenu.addItem('Run rtNEAT', rtneatButton)

#########################################################

### CREATIVEIT: mouse actions ###########################

def mouse_action():
    global clientContext
    if len(getSimContext().modify_object_id) == 0:
        return

    cursor = getSimContext().getMousePosition()
    location = getSimContext().getClickPosition(cursor.x, cursor.y)

    if 'pos' in getSimContext().modify_object_id:
        getSimContext().getMod().setObjectPosition(getSimContext().modify_object_id['pos'], Vector3f(location.x, location.y, getSimContext().object_z))

    elif 'rot' in getSimContext().modify_object_id:
        position = getSimContext().getObjectPosition(getSimContext().modify_object_id['rot'])
        angle = atan2(location.x-position.x, location.y-position.y)
        getSimContext().getMod().setObjectRotation(getSimContext().modify_object_id['rot'], Vector3f(0, 0, -degrees(angle)))       

    elif 'scale' in getSimContext().modify_object_id:
        position = getSimContext().getObjectPosition(getSimContext().modify_object_id['scale'])
        rotation = getSimContext().getObjectRotation(getSimContext().modify_object_id['scale'])
        theta = radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x-position.x)*cos(theta) + (location.y-position.y)*sin(theta)
        localy = -(location.x-position.x)*sin(theta) + (location.y-position.y)*cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + fabs(localx)/getSimContext().scale_const
        scaley = 1 + fabs(localy)/getSimContext().scale_const
        if localx < 0: scalex = 1/scalex
        if localy < 0: scaley = 1/scaley

        getSimContext().getMod().setObjectScale(getSimContext().modify_object_id['scale'], Vector3f(scalex, scaley, 1))       

    elif 'color' in getSimContext().modify_object_id:
        position = getSimContext().getObjectPosition(getSimContext().modify_object_id['color'])
        rotation = getSimContext().getObjectRotation(getSimContext().modify_object_id['color'])
        theta = radians(rotation.z)

        localx = position.x - location.x
        color_value = fabs(localx)*getSimContext().color_const
        if color_value > getSimContext().color_value_max: color_value = getSimContext().color_value_max
        color = int(color_value*255/getSimContext().color_value_max)
        if localx < 0: scolor = SColor(0, color, 0, 0)
        else: scolor = SColor(0, 0, 0, color)
        
        getSimContext().getMod().setObjectColor(getSimContext().modify_object_id['color'], scolor)       


def reset_mouse_action():
    global clientContext
    getSimContext().modify_object_id.clear()

#########################################################

def CreateGui(guiMan): 
    global addBotButton
    global pauseButton
    global cleanBotButton
   
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    
    botTypeBox = gui.create_edit_box(guiMan, 'botType', Pos2i(10,10), Pos2i(90,30), 'rtNEAT')

    numBotBox = gui.create_edit_box(guiMan, 'numBot', Pos2i(110,10), Pos2i(30,30), '5')
    
    changeBotButton = gui.create_button(guiMan, 'changeBot', Pos2i(10,50), Pos2i(130,30), '')
    changeBotButton.text = "Switch to Script"
    changeBotButton.OnMouseLeftClick = lambda:toggle_bot_type(changeBotButton, botTypeBox)

    addBotButton = gui.create_button(guiMan, 'addBot', Pos2i(150,10), Pos2i(60,70), '')
    addBotButton.text = "Add Bots!"
    addBotButton.OnMouseLeftClick = lambda:getMod().add_bot(botTypeBox.text, numBotBox.text)

    pauseButton = gui.create_button( guiMan, 'pause', Pos2i(10,90), Pos2i(95,30), '' )
    pauseButton.text = 'Pause!'
    pauseButton.OnMouseLeftClick = lambda:toggle_ai_callback(pauseButton)
    
    cleanBotButton = gui.create_button(guiMan, 'cleanBot', Pos2i(115,90), Pos2i(95,30), '')
    cleanBotButton.text = "Clean Bots!"
    cleanBotButton.OnMouseLeftClick = lambda:getMod().clean_bots()

    AiWindow = gui.create_window( guiMan, 'AiWindow', Pos2i(560,20), Pos2i(220,150), 'AI Controls' )
    AiWindow.addChild(botTypeBox)
    AiWindow.addChild(numBotBox)
    AiWindow.addChild(changeBotButton)
    AiWindow.addChild(addBotButton)
    AiWindow.addChild(pauseButton)
    AiWindow.addChild(cleanBotButton)

### CREATIVEIT: ClientContext ###########################
class CreativeITContext:
    def __init__(self):
        ClientContext.__init__(self)
        self.object_ids = set()         # Net Ids of objects added to the world by the user
        self.object_z = 2               # Z cooridinate of objects added by the user
        self.modify_object_id = dict()  # Net Id of object whose pos/rot/scale/color is to be modified            
        self.scale_const = 10           # Constant used for scaling objects by moving mouse
        self.color_value_max = 100      # Maximum of color values associated with objects
        self.color_const = 4            # Constant used for adjusting color of objects by moving mouse

#########################################################

def ClientMain():    
    # disable physics and AI updates at first
    # disable_physics()
    disable_ai()
    
    # initialize random number generator with current time
    seed()
    
    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 1500
    camZoomSpeed   = 100
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setPosition(Vector3f(100, 100, 50))
    cam.setTarget(Vector3f(1, 1, 1))
    cam.setFarPlane(1000)
    cam.setEdgeScroll(False)
    
    getMod().setup_sandbox()
    
    # add a light source
    getSimContext().addLightSource(Vector3f(500,-500,1000), 1500)

    # create the io map
    getSimContext().setInputMapping(createInputMapping())

    # setup the gui
    CreateGui(getSimContext().getGuiManager())
