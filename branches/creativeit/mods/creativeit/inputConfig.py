from OpenNero import *

from common import *
from math import *
import common.gui as gui
from common.module import openWiki, getGuiManager
from creativeit.module import getMod, delMod

# functions bound to key/mouse events
def switchToHub():
    delMod()
    switchMod('hub', 'hub:common')

def first_person_control(command):
    # get the controller action
    def closure():
        # controller action
        getMod().control_key(command)
    return closure

# an empty function
def blank():
    pass

# start/stop tracing agent
def toggle_tracing():
    guiMan = getGuiManager()
    if getMod().tracing:
        getMod().stop_tracing()
        guiMan.openFileChooserDialog("Save Trace File", True, lambda filename: getMod().save_trace(filename))
        getMod().tracing = False
    else:
        getMod().start_tracing()
        getMod().tracing = True

# enable simulation display
def enable_simdisplay():
    getMod().enable_simdisplay()

# disable simulation display
def disable_simdisplay():
    getMod().disable_simdisplay()

def show_context_menu():
    guiMan = getGuiManager()

    if len(getMod().modify_object_id) > 0 or guiMan.isOpenFileChooserDialog():
        return

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    # find the screen position at which to open the context menu
    cursor = getSimContext().getMousePosition()
    location = getSimContext().getClickedPosition(cursor.x, cursor.y)
    selected_object_id = getSimContext().getClickedEntityId(cursor.x, cursor.y)

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    # specify the path of the selected object
    def specify_object_path():
        # reads path specified as python code from given filename
        def read_path(filename):
            with open(filename, 'r') as file:
                code = compile(file.read(), filename, 'eval')
            getSimContext().setObjectPath(code, selected_object_id)

        guiMan.openFileChooserDialog("Open Object Path File", True, read_path)

    # activate the selected object to be moved
    def move_object():
        getMod().modify_object_id['pos'] = selected_object_id

    # activate the selected object to be rotated
    def rotate_object():
        getMod().modify_object_id['rot'] = selected_object_id

    # activate the selected object to be scaled
    def scale_object():
        getMod().modify_object_id['scale'] = selected_object_id

    # activate the selected object to be colored
    def color_object():
        getMod().modify_object_id['color'] = selected_object_id

    # remove the selected object from the world
    def remove_object():
        getMod().removeObject(selected_object_id)

    # add a wall at the current cursor location
    def add_wall():
        getMod().addWall(Vector3f(location.x, location.y, getMod().object_z))

    # add a cube at the current cursor location
    def add_cube():
        getMod().addCube(Vector3f(location.x, location.y, getMod().object_z))

    # give advice to agents.
    def give_advice():
        # reads advice from given filename
        def read_advice(filename):
            with open(filename, 'r') as file:
                getMod().setAdvice(file.read())

        guiMan.openFileChooserDialog("Open Advice File", True, read_advice)
        #adviceBox = gui.create_edit_box(guiMan, 'advice', Pos2i(0,0), Pos2i(200,20), 'hello')
        #getMod().setAdvice('{if v6 >= 0 then {output v8 0.99 v9 0.20}}')

    # load previously stored trace.
    def load_trace():
        guiMan.openFileChooserDialog("Load Trace File", True, lambda filename: getMod().load_trace(filename))
        
    # if the user clicked on an object they added previously, show the options
    # to modify it; otherwise show general options.
    if selected_object_id in getMod().object_ids:
        pathButton = gui.create_button(guiMan, 'path', Pos2i(0,0), Pos2i(0,0), '')
        pathButton.OnMouseLeftClick = specify_object_path
        contextMenu.addItem('Specify object path', pathButton)

        moveButton = gui.create_button(guiMan, 'move', Pos2i(0,0), Pos2i(0,0), '')
        moveButton.OnMouseLeftClick = move_object
        contextMenu.addItem('Move object', moveButton)

        rotateButton = gui.create_button(guiMan, 'rotate', Pos2i(0,0), Pos2i(0,0), '')
        rotateButton.OnMouseLeftClick = rotate_object
        contextMenu.addItem('Rotate object', rotateButton)

        scaleButton = gui.create_button(guiMan, 'scale', Pos2i(0,0), Pos2i(0,0), '')
        scaleButton.OnMouseLeftClick = scale_object
        contextMenu.addItem('Scale object', scaleButton)

        colorButton = gui.create_button(guiMan, 'color', Pos2i(0,0), Pos2i(0,0), '')
        colorButton.OnMouseLeftClick = color_object
        contextMenu.addItem('Color object', colorButton)

        removeButton = gui.create_button(guiMan, 'remove', Pos2i(0,0), Pos2i(0,0), '')
        removeButton.OnMouseLeftClick = remove_object
        contextMenu.addItem('Remove object', removeButton)

    else:
        wallButton = gui.create_button(guiMan, 'wall', Pos2i(0,0), Pos2i(0,0), '')
        wallButton.OnMouseLeftClick = add_wall
        contextMenu.addItem('Add wall', wallButton)

        cubeButton = gui.create_button(guiMan, 'cube', Pos2i(0,0), Pos2i(0,0), '')
        cubeButton.OnMouseLeftClick = add_cube
        contextMenu.addItem('Add cube', cubeButton)

        rtneatButton = gui.create_button(guiMan, 'rtneat', Pos2i(0,0), Pos2i(0,0), '')
        rtneatButton.OnMouseLeftClick = lambda: getMod().start_rtneat()
        contextMenu.addItem('Run rtNEAT', rtneatButton)

        scriptedButton = gui.create_button(guiMan, 'scripted', Pos2i(0,0), Pos2i(0,0), '')
        scriptedButton.OnMouseLeftClick = lambda: getMod().start_scripted()
        contextMenu.addItem('Run scripted', scriptedButton)

        keyboardButton = gui.create_button(guiMan, 'keyboard', Pos2i(0,0), Pos2i(0,0), '')
        keyboardButton.OnMouseLeftClick = lambda: getMod().start_keyboard()
        contextMenu.addItem('Run keyboard', keyboardButton)

        adviceButton = gui.create_button(guiMan, 'advice', Pos2i(0,0), Pos2i(0,0), '')
        adviceButton.OnMouseLeftClick = give_advice
        contextMenu.addItem('Give advice', adviceButton)

        loadTraceButton = gui.create_button(guiMan, 'load trace', Pos2i(0,0), Pos2i(0,0), '')
        loadTraceButton.OnMouseLeftClick = load_trace
        contextMenu.addItem('Load trace', loadTraceButton)

        unloadTraceButton = gui.create_button(guiMan, 'unload trace', Pos2i(0,0), Pos2i(0,0), '')
        unloadTraceButton.OnMouseLeftClick = lambda: getMod().unload_trace()
        contextMenu.addItem('Unload trace', unloadTraceButton)

        enableBackpropButton = gui.create_button(guiMan, 'enable backprop', Pos2i(0,0), Pos2i(0,0), '')
        enableBackpropButton.OnMouseLeftClick = lambda: getMod().enable_backprop()
        contextMenu.addItem('Enable Backprop', enableBackpropButton)

        disableBackpropButton = gui.create_button(guiMan, 'disable backprop', Pos2i(0,0), Pos2i(0,0), '')
        disableBackpropButton.OnMouseLeftClick = lambda: getMod().disable_backprop()
        contextMenu.addItem('Disable Backprop', disableBackpropButton)


def mouse_action():
    if len(getMod().modify_object_id) == 0:
        return

    cursor = getSimContext().getMousePosition()
    location = getSimContext().getClickedPosition(cursor.x, cursor.y)

    if 'pos' in getMod().modify_object_id:
        getSimContext().setObjectPosition(getMod().modify_object_id['pos'], Vector3f(location.x, location.y, getMod().object_z))

    elif 'rot' in getMod().modify_object_id:
        position = getSimContext().getObjectPosition(getMod().modify_object_id['rot'])
        angle = atan2(location.x-position.x, location.y-position.y)
        getSimContext().setObjectRotation(getMod().modify_object_id['rot'], Vector3f(0, 0, -degrees(angle)))       

    elif 'scale' in getMod().modify_object_id:
        position = getSimContext().getObjectPosition(getMod().modify_object_id['scale'])
        rotation = getSimContext().getObjectRotation(getMod().modify_object_id['scale'])
        theta = radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x-position.x)*cos(theta) + (location.y-position.y)*sin(theta)
        localy = -(location.x-position.x)*sin(theta) + (location.y-position.y)*cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + fabs(localx)/getMod().scale_const
        scaley = 1 + fabs(localy)/getMod().scale_const
        if localx < 0: scalex = 1/scalex
        if localy < 0: scaley = 1/scaley

        getSimContext().setObjectScale(getMod().modify_object_id['scale'], Vector3f(scalex, scaley, 1))       

    elif 'color' in getMod().modify_object_id:
        position = getSimContext().getObjectPosition(getMod().modify_object_id['color'])
        rotation = getSimContext().getObjectRotation(getMod().modify_object_id['color'])
        theta = radians(rotation.z)

        localx = position.x - location.x
        color_value = fabs(localx)*getMod().color_const
        if color_value > getMod().color_value_max: color_value = getMod().color_value_max
        color = int(color_value*255/getMod().color_value_max)
        if localx < 0: scolor = Color(0, color, 0, 0)
        else: scolor = Color(0, 0, 0, color)
        
        getSimContext().setObjectColor(getMod().modify_object_id['color'], scolor)       


def reset_mouse_action():
    getMod().modify_object_id.clear()
    
def createInputMapping():

    # create an io map
    ioMap = PyIOMap()

    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub )
    ioMap.BindKey( "KEY_F5", "onPress", toggle_tracing )
    ioMap.BindKey( "KEY_F8", "onPress", enable_simdisplay )
    ioMap.BindKey( "KEY_F9", "onPress", disable_simdisplay )
    ioMap.BindMouseAction( "moveX", mouse_action )
    ioMap.BindMouseAction( "moveY", mouse_action )
    ioMap.BindMouseButton( "left", "onPress", reset_mouse_action )
    ioMap.BindMouseButton( "right", "onPress", show_context_menu )

    # FPS control
    ioMap.BindKey( "KEY_KEY_A", "onHold", first_person_control('left') )
    ioMap.BindKey( "KEY_KEY_D", "onHold", first_person_control('right') )
    ioMap.BindKey( "KEY_KEY_W", "onHold", first_person_control('fwd') )
    ioMap.BindKey( "KEY_KEY_S", "onHold", first_person_control('back') )
    ioMap.BindKey( "KEY_KEY_Q", "onHold", first_person_control('Lturn') )
    ioMap.BindKey( "KEY_KEY_E", "onHold", first_person_control('Rturn') )

    return ioMap
