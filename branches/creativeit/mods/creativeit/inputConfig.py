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

# enable simulation display
def enable_simdisplay():
    getMod().enable_simdisplay()

# disable simulation display
def disable_simdisplay():
    getMod().disable_simdisplay()

def set_circle_object_radius(radius, setpos = True):
    from functools import partial
    if getMod().task == 'circle' and getMod().key_object_id.has_key('1'):
        oid = getMod().key_object_id['1']
        getMod().current_radius = radius
        getMod().setObjectPath(oid, partial(getMod().circle_function, getMod().current_radius, getMod().current_speed))
        print "radius of object (id: %d) set to %f" % (oid, getMod().current_radius)

        # if an agent control method is not running yet, manually set the position of the object.
        if getMod().control == None and setpos:
            getSimContext().setObjectPosition(oid, Vector3f(900 + 80*radius, -60, getMod().object_z))

def set_circle_object_speed(speed):
    from functools import partial
    if getMod().task == 'circle' and getMod().key_object_id.has_key('1'):
        oid = getMod().key_object_id['1']
        getMod().current_speed = speed
        getMod().setObjectPath(oid, partial(getMod().circle_function, getMod().current_radius, getMod().current_speed))
        print "speed of object (id: %d) set to %f" % (oid, getMod().current_speed)

# place predefined objects using keyboard
def place_object(key, obj, pos, rot = Vector3f(0,0,0), vel = Vector3f(0,0,0), scl = Vector3f(1,1,1)):
    if key in getMod().key_object_id:
        getSimContext().setObjectPosition(getMod().key_object_id[key], getMod().key_orig_props[key]['pos'])
        getSimContext().setObjectRotation(getMod().key_object_id[key], getMod().key_orig_props[key]['rot'])
        getSimContext().setObjectScale(getMod().key_object_id[key], getMod().key_orig_props[key]['scl'])
        set_circle_object_radius(1)
        set_circle_object_speed(1)
        print "reset object (id: %d) for key %s" % (getMod().key_object_id[key], key)
    else:
        getMod().key_orig_props[key] = {'pos': pos, 'rot': rot, 'vel': vel, 'scl': scl}
        if obj == 'cube':
            getMod().addCube(pos, rot, scl)
        else:
            getMod().addWall(pos, rot, vel, scl)
        print "added object (id: %d) for key %s" % (getMod().key_object_id[key], key)
        if getMod().task == 'circle':
            pathfile = "creativeit/data/ai/circle.py"
            with open(pathfile, 'r') as file:
                code = compile(file.read(), pathfile, 'eval')
                getMod().circle_function = eval(code)
                set_circle_object_radius(1, False)  # don't set position of just-created object to avoid crash
                set_circle_object_speed(1)

def place_object1():
    if getMod().task == 'combo':
        place_object('1', 'cube', Vector3f(835, -160, getMod().object_z))
    elif getMod().task == 'around':
        place_object('1', 'cube', Vector3f(800, -70, getMod().object_z))
    elif getMod().task == 'circle':
        place_object('1', 'cube', Vector3f(980, -60, getMod().object_z))
 
def place_object2():
    if getMod().task == 'combo':
        place_object('2', 'cube', Vector3f(830, -100, getMod().object_z))
    elif getMod().task == 'around':
        place_object('2', 'wall', Vector3f(860, -60, getMod().object_z), Vector3f(0, 0, 100), Vector3f(0, 0, 0), Vector3f(10, 1, 1))
 
def place_object3():
    if getMod().task == 'combo':
        place_object('3', 'cube', Vector3f(835, -70, getMod().object_z))
 
def place_object4():
    if getMod().task == 'combo':
        place_object('4', 'cube', Vector3f(835, 20, getMod().object_z))
 
def place_object5():
    if getMod().task == 'combo':
        place_object('5', 'wall', Vector3f(862, -102, getMod().object_z), Vector3f(0, 0, 65), Vector3f(0, 0, 0), Vector3f(4.9, 9, 1))

def make_control_menu():
    guiMan = getGuiManager()
 
    def load_script():
        def read_script(filename):
            with open(filename, 'r') as file:
                getMod().setAdvice(file.read())
        guiMan.openFileChooserDialog("Open Script File", True, read_script)
 
    def start_evolution():
        getMod().control = 'evolution'
        getMod().start_rtneat(getMod().task)
        guiMan.removeAll()
        print "started evolution of agents"
 
    def start_scripted():
        getMod().control = 'scripted'
        getMod().start_scripted()
        guiMan.removeAll()
        load_script()
        print "started scripted agent"
 
    def start_keyboard():
        getMod().control = 'keyboard'
        getMod().unload_trace()
        getMod().start_keyboard()
        guiMan.removeAll()
        print "started keyboard agent"
 
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    controlMenu = gui.create_window(guiMan, 'control', Pos2i(0,50), Pos2i(100,110), 'Control Agent')
    controlMenu.setVisibleCloseButton(False)
    #controlMenu.setDraggable(False)
 
    if getMod().approach != 'scripting':
        evolutionButton = gui.create_button(guiMan, 'evolution', Pos2i(0,0), Pos2i(100,30), '')
        evolutionButton.OnMouseLeftClick = lambda: start_evolution()
        evolutionButton.text = 'Start evolution'
        controlMenu.addChild(evolutionButton)
 
    if getMod().approach == 'scripting':
        scriptedButton = gui.create_button(guiMan, 'scripted', Pos2i(0,30), Pos2i(100,30), '')
        scriptedButton.OnMouseLeftClick = lambda: start_scripted()
        scriptedButton.text = 'Start scripted'
        controlMenu.addChild(scriptedButton)
 
    if getMod().approach == 'example':
        keyboardButton = gui.create_button(guiMan, 'keyboard', Pos2i(0,60), Pos2i(100,30), '')
        keyboardButton.OnMouseLeftClick = lambda: start_keyboard()
        keyboardButton.text = 'Start keyboard'
        controlMenu.addChild(keyboardButton)

def reset_objects():
    # first, remove all objects not defined by the task
    for oid in getMod().object_ids:
        if oid not in getMod().key_object_id.values():
            getSimContext().removeObject(oid)
    getMod().object_ids.clear()
    for oid in getMod().key_object_id.values():
        getMod().object_ids.add(oid)

    place_object1()
    place_object2()
    place_object3()
    place_object4()
    place_object5()

def show_context_menu():
    guiMan = getGuiManager()

    if len(getMod().modify_object_id) > 0 or guiMan.isOpenFileChooserDialog():
        return

    if getMod().task == None or getMod().approach == None or getMod().approach == 'evolution':
        return

    if getMod().control == None and (getMod().approach == 'advice' or getMod().approach == 'scripting'):
        return

    if getMod().control == 'scripted':
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
            from functools import partial
            with open(filename, 'r') as file:
                code = compile(file.read(), filename, 'eval')
            getMod().setObjectPath(selected_object_id, partial(eval(code), 1.0))
            print "path of object (id: %d) set from file %s" % (selected_object_id, filename)
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
        getSimContext().removeObject(selected_object_id)
        for key, oid in getMod().key_object_id.items():
            if oid == selected_object_id:
                del getMod().key_object_id[key]
                del getMod().key_orig_props[key]
        print "object (id: %d) removed" % (selected_object_id)

    # add a wall at the current cursor location
    def add_wall():
         getMod().addWall(Vector3f(location.x, location.y, getMod().object_z))
         print "wall (id: %d) added at location (%f, %f, %f)" % (oid, location.x, location.y, getMod().object_z)

    # add a cube at the current cursor location
    def add_cube():
         getMod().addCube(Vector3f(location.x, location.y, getMod().object_z))
         print "cube (id: %d) added at location (%f, %f, %f)" % (oid, location.x, location.y, getMod().object_z)

    # give advice to agents.
    def give_advice():
        # reads advice from given filename
        def read_advice(filename):
            with open(filename, 'r') as file:
                getMod().setAdvice(file.read())

        guiMan.openFileChooserDialog("Open Advice File", True, read_advice)
        #adviceBox = gui.create_edit_box(guiMan, 'advice', Pos2i(0,0), Pos2i(200,20), 'hello')
        #getMod().setAdvice('{if v6 >= 0 then {output v8 0.99 v9 0.20}}')

    # save trace of agent
    def save_trace():
        guiMan.openFileChooserDialog("Save Example", True, lambda filename: getMod().save_trace(filename))
        print "saved trace in file"

    # load previously stored trace.
    def load_trace():
        guiMan.openFileChooserDialog("Load Example", True, lambda filename: getMod().load_trace(filename))
        print "loaded trace from file"
        
    # unload previously loaded trace.
    def unload_trace():
        getMod().unload_trace()
        print "unloaded trace"

    # activate changing the radius of circle in which cube moves
    def change_radius():
        getMod().modify_object_id['radius'] = getMod().key_object_id['1']
        getMod().saved_cursor = getMod().sim_context.getMousePosition()

    # activate changing the speed of cube moving in a circle
    def change_speed():
        getMod().modify_object_id['speed'] = getMod().key_object_id['1']
        getMod().saved_cursor = getMod().sim_context.getMousePosition()

    # if we are using the shaping approach and the user clicked on an object, then
    # show options to modify it.
    if getMod().approach == 'shaping' and selected_object_id in getMod().object_ids:
        #pathButton = gui.create_button(guiMan, 'path', Pos2i(0,0), Pos2i(0,0), '')
        #pathButton.OnMouseLeftClick = lambda: specify_object_path()
        #contextMenu.addItem('Specify object path', pathButton)

        moveButton = gui.create_button(guiMan, 'move', Pos2i(0,0), Pos2i(0,0), '')
        moveButton.OnMouseLeftClick = move_object
        contextMenu.addItem('Move object', moveButton)

        rotateButton = gui.create_button(guiMan, 'rotate', Pos2i(0,0), Pos2i(0,0), '')
        rotateButton.OnMouseLeftClick = rotate_object
        contextMenu.addItem('Rotate object', rotateButton)

        scaleButton = gui.create_button(guiMan, 'scale', Pos2i(0,0), Pos2i(0,0), '')
        scaleButton.OnMouseLeftClick = scale_object
        contextMenu.addItem('Scale object', scaleButton)

        #colorButton = gui.create_button(guiMan, 'color', Pos2i(0,0), Pos2i(0,0), '')
        #colorButton.OnMouseLeftClick = lambda: color_object()
        #contextMenu.addItem('Color object', colorButton)

        removeButton = gui.create_button(guiMan, 'remove', Pos2i(0,0), Pos2i(0,0), '')
        removeButton.OnMouseLeftClick = remove_object
        contextMenu.addItem('Remove object', removeButton)

    elif getMod().approach == 'shaping':
        wallButton = gui.create_button(guiMan, 'wall', Pos2i(0,0), Pos2i(0,0), '')
        wallButton.OnMouseLeftClick = add_wall
        contextMenu.addItem('Add wall', wallButton)

        cubeButton = gui.create_button(guiMan, 'cube', Pos2i(0,0), Pos2i(0,0), '')
        cubeButton.OnMouseLeftClick = add_cube
        contextMenu.addItem('Add cube', cubeButton)

        resetButton = gui.create_button(guiMan, 'reset', Pos2i(0,0), Pos2i(0,0), '')
        resetButton.OnMouseLeftClick = reset_objects
        contextMenu.addItem('Reset objects', resetButton)

        if getMod().task == 'circle':
            radiusButton = gui.create_button(guiMan, 'radius', Pos2i(0,0), Pos2i(0,0), '')
            radiusButton.OnMouseLeftClick = lambda: change_radius()
            contextMenu.addItem('Change radius', radiusButton)

            speedButton = gui.create_button(guiMan, 'speed', Pos2i(0,0), Pos2i(0,0), '')
            speedButton.OnMouseLeftClick = lambda: change_speed()
            contextMenu.addItem('Change speed', speedButton)

    elif getMod().approach == 'advice':
        adviceButton = gui.create_button(guiMan, 'advice', Pos2i(0,0), Pos2i(0,0), '')
        adviceButton.OnMouseLeftClick = give_advice
        contextMenu.addItem('Load advice', adviceButton)

    #elif getMod().approach == 'scripting':
    #    scriptingButton = gui.create_button(guiMan, 'scripting', Pos2i(0,0), Pos2i(0,0), '')
    #    scriptingButton.OnMouseLeftClick = lambda: give_advice()
    #    contextMenu.addItem('Load script', scriptingButton)

    elif getMod().approach == 'example':
        if getMod().control == 'keyboard':
            saveTraceButton = gui.create_button(guiMan, 'save trace', Pos2i(0,0), Pos2i(0,0), '')
            saveTraceButton.OnMouseLeftClick = lambda: save_trace()
            contextMenu.addItem('Save example', saveTraceButton)
        else:
            loadTraceButton = gui.create_button(guiMan, 'load trace', Pos2i(0,0), Pos2i(0,0), '')
            loadTraceButton.OnMouseLeftClick = lambda: load_trace()
            contextMenu.addItem('Load example', loadTraceButton)

            unloadTraceButton = gui.create_button(guiMan, 'unload trace', Pos2i(0,0), Pos2i(0,0), '')
            unloadTraceButton.OnMouseLeftClick = lambda: unload_trace()
            contextMenu.addItem('Unload example', unloadTraceButton)

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

    elif 'radius' in getMod().modify_object_id:
        mousex = cursor.x - getMod().saved_cursor.x
        radius = getMod().current_radius + mousex*getMod().radius_const
        if radius < 0.0: radius = 0.0
        if radius > 1.0: radius = 1.0
        set_circle_object_radius(radius)
        getMod().saved_cursor = cursor
        label = str(getMod().current_radius*100) + "%"
        getSimContext().setObjectLabel(getMod().modify_object_id['radius'], label);
        
    elif 'speed' in getMod().modify_object_id:
        mousex = cursor.x - getMod().saved_cursor.x
        speed = getMod().current_speed + mousex*getMod().speed_const
        if speed < 0.0: speed = 0.0
        if speed > 1.0: speed = 1.0
        set_circle_object_speed(speed)
        getMod().saved_cursor = cursor
        label = str(getMod().current_speed*100) + "%"
        getSimContext().setObjectLabel(getMod().modify_object_id['speed'], label);
        


def reset_mouse_action():
    getMod().modify_object_id.clear()
    
def createInputMapping():

    # create an io map
    ioMap = PyIOMap()

    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub )
    ioMap.BindKey( "KEY_F8", "onPress", enable_simdisplay )
    ioMap.BindKey( "KEY_F9", "onPress", disable_simdisplay )
    ioMap.BindMouseAction( "moveX", mouse_action )
    ioMap.BindMouseAction( "moveY", mouse_action )
    ioMap.BindMouseButton( "left", "onPress", reset_mouse_action )
    ioMap.BindMouseButton( "right", "onPress", show_context_menu )

    # place predefined objects using keyboard
    ioMap.BindKey( "KEY_KEY_1", "onPress", place_object1 )
    ioMap.BindKey( "KEY_KEY_2", "onPress", place_object2 )
    ioMap.BindKey( "KEY_KEY_3", "onPress", place_object3 )
    ioMap.BindKey( "KEY_KEY_4", "onPress", place_object4 )
    ioMap.BindKey( "KEY_KEY_5", "onPress", place_object5 )

    # FPS control
    ioMap.BindKey( "KEY_KEY_A", "onHold", first_person_control('left') )
    ioMap.BindKey( "KEY_KEY_D", "onHold", first_person_control('right') )
    ioMap.BindKey( "KEY_KEY_W", "onHold", first_person_control('fwd') )
    ioMap.BindKey( "KEY_KEY_S", "onHold", first_person_control('back') )
    ioMap.BindKey( "KEY_KEY_Q", "onHold", first_person_control('Lturn') )
    ioMap.BindKey( "KEY_KEY_E", "onHold", first_person_control('Rturn') )

    return ioMap
