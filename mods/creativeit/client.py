import OpenNero
from math import *
import random
import common
import common.gui as gui
import constants
import module

# globals - remove me?
client = None

# called from GUI
def switchToHub():
    module.delMod()
    OpenNero.switchMod('hub', 'hub:common')

# control for keyboard agent
def key_left():
    module.getMod().control_key('left')
def key_right():
    module.getMod().control_key('right')
def key_forward():
    module.getMod().control_key('fwd')
def key_back():
    module.getMod().control_key('back')

# enable simulation display
def enable_simdisplay():
    module.getMod().enable_simdisplay()

# disable simulation display
def disable_simdisplay():
    module.getMod().disable_simdisplay()

def set_circle_object_radius(radius, setpos = True):
    from functools import partial
    if module.getMod().task == 'circle' and module.getMod().key_object_id.has_key('1'):
        oid = module.getMod().key_object_id['1']
        module.getMod().current_radius = radius
        module.getMod().setObjectPath(oid, partial(module.getMod().circle_function, module.getMod().current_radius, module.getMod().current_speed))
        print "radius of object (id: %d) set to %f" % (oid, module.getMod().current_radius)

        # if an agent control method is not running yet, manually set the position of the object.
        if module.getMod().control == None and setpos:
            module.getMod().setObjectPosition(oid, OpenNero.Vector3f(900 + 80*radius, -60, module.getMod().object_z))

def set_circle_object_speed(speed):
    from functools import partial
    if module.getMod().task == 'circle' and module.getMod().key_object_id.has_key('1'):
        oid = module.getMod().key_object_id['1']
        module.getMod().current_speed = speed
        module.getMod().setObjectPath(oid, partial(module.getMod().circle_function, module.getMod().current_radius, module.getMod().current_speed))
        print "speed of object (id: %d) set to %f" % (oid, module.getMod().current_speed)

# place predefined objects using keyboard
def place_object(key, obj, pos, rot = OpenNero.Vector3f(0,0,0), vel =
                 OpenNero.Vector3f(0,0,0), scl = OpenNero.Vector3f(1,1,1)):
    sim_context = OpenNero.getSimContext()
    if key in module.getMod().key_object_id:
        module.getMod().setObjectPosition(module.getMod().key_object_id[key], module.getMod().key_orig_props[key]['pos'])
        module.getMod().setObjectRotation(module.getMod().key_object_id[key], module.getMod().key_orig_props[key]['rot'])
        module.getMod().setObjectScale(module.getMod().key_object_id[key], module.getMod().key_orig_props[key]['scl'])
        set_circle_object_radius(1)
        set_circle_object_speed(1)
        print "reset object (id: %d) for key %s" % (module.getMod().key_object_id[key], key)
    else:
        module.getMod().key_orig_props[key] = {'pos': pos, 'rot': rot, 'vel': vel, 'scl': scl}
        if obj == 'cube':
            id = module.getMod().addCube(pos, rot, vel, scl)
        else:
            id = module.getMod().addWall(pos, rot, vel, scl)
        module.getMod().key_object_id[key] = id
        module.getMod().object_ids.add(id)
        print "added object (id: %d) for key %s" % (id, key)

        if module.getMod().task == 'circle':
            pathfile = "creativeit/data/ai/circle.py"
            with open(pathfile, 'r') as file:
                code = compile(file.read(), pathfile, 'eval')
                module.getMod().circle_function = eval(code)
                set_circle_object_radius(1, False)  # don't set position of just-created object to avoid crash
                set_circle_object_speed(1)


def place_object1():
    if module.getMod().task == 'combo':
        place_object('1', 'cube', OpenNero.Vector3f(835, -160, module.getMod().object_z))
    elif module.getMod().task == 'around':
        place_object('1', 'cube', OpenNero.Vector3f(800, -70, module.getMod().object_z))
    elif module.getMod().task == 'circle':
        place_object('1', 'cube', OpenNero.Vector3f(980, -60, module.getMod().object_z))

def place_object2():
    if module.getMod().task == 'combo':
        place_object('2', 'cube', OpenNero.Vector3f(830, -100, module.getMod().object_z))
    elif module.getMod().task == 'around':
        place_object('2', 'wall', OpenNero.Vector3f(860, -60, module.getMod().object_z), OpenNero.Vector3f(0, 0, 100), OpenNero.Vector3f(0, 0, 0), OpenNero.Vector3f(10, 1, 1))

def place_object3():
    if module.getMod().task == 'combo':
        place_object('3', 'cube', OpenNero.Vector3f(835, -70, module.getMod().object_z))

def place_object4():
    if module.getMod().task == 'combo':
        place_object('4', 'cube', OpenNero.Vector3f(835, 20, module.getMod().object_z))

def place_object5():
    if module.getMod().task == 'combo':
        place_object('5', 'wall', OpenNero.Vector3f(862, -102, module.getMod().object_z), OpenNero.Vector3f(0, 0, 65), OpenNero.Vector3f(0, 0, 0), OpenNero.Vector3f(4.9, 9, 1))

def make_control_menu():
    guiMan = common.getGuiManager()

    def load_script():
        def read_script(filename):
            with open(filename, 'r') as file:
                module.getMod().setAdvice(file.read())
        guiMan.openFileChooserDialog("Open Script File", True, read_script)

    def start_evolution():
        module.getMod().control = 'evolution'
        module.getMod().start_rtneat(module.getMod().task)
        guiMan.removeAll()
        print "started evolution of agents"

    def start_scripted():
        module.getMod().control = 'scripted'
        module.getMod().start_scripted()
        guiMan.removeAll()
        load_script()
        print "started scripted agent"

    def start_keyboard():
        module.getMod().control = 'keyboard'
        module.getMod().unload_trace()
        module.getMod().start_keyboard()
        guiMan.removeAll()
        print "started keyboard agent"

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    controlMenu = gui.create_window(guiMan, 'control', OpenNero.Pos2i(0,50), OpenNero.Pos2i(100,110), 'Control Agent')
    controlMenu.setVisibleCloseButton(False)
    #controlMenu.setDraggable(False)

    if module.getMod().approach != 'scripting':
        evolutionButton = gui.create_button(guiMan, 'evolution', OpenNero.Pos2i(0,0), OpenNero.Pos2i(100,30), '')
        evolutionButton.OnMouseLeftClick = lambda: start_evolution()
        evolutionButton.text = 'Start evolution'
        controlMenu.addChild(evolutionButton)

    if module.getMod().approach == 'scripting':
        scriptedButton = gui.create_button(guiMan, 'scripted', OpenNero.Pos2i(0,30), OpenNero.Pos2i(100,30), '')
        scriptedButton.OnMouseLeftClick = lambda: start_scripted()
        scriptedButton.text = 'Start scripted'
        controlMenu.addChild(scriptedButton)

    if module.getMod().approach == 'example':
        keyboardButton = gui.create_button(guiMan, 'keyboard', OpenNero.Pos2i(0,60), OpenNero.Pos2i(100,30), '')
        keyboardButton.OnMouseLeftClick = lambda: start_keyboard()
        keyboardButton.text = 'Start keyboard'
        controlMenu.addChild(keyboardButton)

def make_approach_menu():
    guiMan = common.getGuiManager()

    def start_approach(approach):
        module.getMod().approach = approach
        guiMan.removeAll()
        make_control_menu()
        print "selected approach: ", approach

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    approachMenu = gui.create_window(guiMan, 'approaches', OpenNero.Pos2i(0,50), OpenNero.Pos2i(100,170), 'Select Approach')
    approachMenu.setVisibleCloseButton(False)
    #approachMenu.setDraggable(False)

    evolutionButton = gui.create_button(guiMan, 'evolution', OpenNero.Pos2i(0,0), OpenNero.Pos2i(100,30), '')
    evolutionButton.OnMouseLeftClick = lambda: start_approach('evolution')
    evolutionButton.text = 'Evolution'
    approachMenu.addChild(evolutionButton)

    shapingButton = gui.create_button(guiMan, 'shaping', OpenNero.Pos2i(0,30), OpenNero.Pos2i(100,30), '')
    shapingButton.OnMouseLeftClick = lambda: start_approach('shaping')
    shapingButton.text = 'Shaping'
    approachMenu.addChild(shapingButton)

    adviceButton = gui.create_button(guiMan, 'advice', OpenNero.Pos2i(0,60), OpenNero.Pos2i(100,30), '')
    adviceButton.OnMouseLeftClick = lambda: start_approach('advice')
    adviceButton.text = 'Advice'
    approachMenu.addChild(adviceButton)

    exampleButton = gui.create_button(guiMan, 'example', OpenNero.Pos2i(0,90), OpenNero.Pos2i(100,30), '')
    exampleButton.OnMouseLeftClick = lambda: start_approach('example')
    exampleButton.text = 'Example'
    approachMenu.addChild(exampleButton)

    scriptingButton = gui.create_button(guiMan, 'scripting', OpenNero.Pos2i(0,120), OpenNero.Pos2i(100,30), '')
    scriptingButton.OnMouseLeftClick = lambda: start_approach('scripting')
    scriptingButton.text = 'Scripting'
    approachMenu.addChild(scriptingButton)

def make_task_menu():
    guiMan = common.getGuiManager()

    def start_task(task):
        module.getMod().task = task
        guiMan.removeAll()
        make_approach_menu()
        reset_objects()
        print "selected task: ", task

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    taskMenu = gui.create_window(guiMan, 'tasks', OpenNero.Pos2i(0,50), OpenNero.Pos2i(100,110), 'Select Task')
    taskMenu.setVisibleCloseButton(False)
    #taskMenu.setDraggable(False)

    comboButton = gui.create_button(guiMan, 'combo', OpenNero.Pos2i(0,0), OpenNero.Pos2i(100,30), '')
    comboButton.OnMouseLeftClick = lambda: start_task('combo')
    comboButton.text = 'Cube/Wall Combo'
    taskMenu.addChild(comboButton)

    aroundButton = gui.create_button(guiMan, 'around', OpenNero.Pos2i(0,30), OpenNero.Pos2i(100,30), '')
    aroundButton.OnMouseLeftClick = lambda: start_task('around')
    aroundButton.text = 'Around the Wall'
    taskMenu.addChild(aroundButton)

    circleButton = gui.create_button(guiMan, 'circle', OpenNero.Pos2i(0,60), OpenNero.Pos2i(100,30), '')
    circleButton.OnMouseLeftClick = lambda: start_task('circle')
    circleButton.text = 'Circle Chase'
    taskMenu.addChild(circleButton)

def reset_objects():
    # first, remove all objects not defined by the task
    for oid in module.getMod().object_ids:
        if oid not in module.getMod().key_object_id.values():
            module.getMod().removeObject(oid)
    module.getMod().object_ids.clear()
    for oid in module.getMod().key_object_id.values():
        module.getMod().object_ids.add(oid)

    place_object1()
    place_object2()
    place_object3()
    place_object4()
    place_object5()

def show_context_menu():
    guiMan = common.getGuiManager()

    if len(module.getMod().modify_object_id) > 0 or guiMan.isOpenFileChooserDialog():
        return

    if module.getMod().task == None or module.getMod().approach == None or module.getMod().approach == 'evolution':
        return

    if module.getMod().control == None and (module.getMod().approach == 'advice' or module.getMod().approach == 'scripting'):
        return

    if module.getMod().control == 'scripted':
        return

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    
    sim_context = OpenNero.getSimContext()

    # find the screen position at which to open the context menu
    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)
    selected_object_id = sim_context.getClickedEntityId(cursor.x, cursor.y)

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    # specify the path of the selected object
    def specify_object_path():
        # reads path specified as python code from given filename
        def read_path(filename):
            from functools import partial
            with open(filename, 'r') as file:
                code = compile(file.read(), filename, 'eval')
            module.getMod().setObjectPath(selected_object_id, partial(eval(code), 1.0))
            print "path of object (id: %d) set from file %s" % (selected_object_id, filename)

        guiMan.openFileChooserDialog("Open Object Path File", True, read_path)

    # activate the selected object to be moved
    def move_object():
        module.getMod().modify_object_id['pos'] = selected_object_id

    # activate the selected object to be rotated
    def rotate_object():
        module.getMod().modify_object_id['rot'] = selected_object_id

    # activate the selected object to be scaled
    def scale_object():
        module.getMod().modify_object_id['scale'] = selected_object_id

    # activate the selected object to be colored
    def color_object():
        module.getMod().modify_object_id['color'] = selected_object_id

    # remove the selected object from the world
    def remove_object():
        module.getMod().object_ids.remove(selected_object_id)
        module.getMod().removeObject(selected_object_id)
        for key, oid in module.getMod().key_object_id.items():
            if oid == selected_object_id:
                del module.getMod().key_object_id[key]
                del module.getMod().key_orig_props[key]
        print "object (id: %d) removed" % (selected_object_id)

    # add a wall at the current cursor location
    def add_wall():
        oid = module.getMod().addWall(OpenNero.Vector3f(location.x, location.y, module.getMod().object_z))
        module.getMod().object_ids.add(oid)
        print "wall (id: %d) added at location (%f, %f, %f)" % (oid, location.x, location.y, module.getMod().object_z)

    # add a cube at the current cursor location
    def add_cube():
        oid = module.getMod().addCube(OpenNero.Vector3f(location.x, location.y, module.getMod().object_z))
        module.getMod().object_ids.add(oid)
        print "cube (id: %d) added at location (%f, %f, %f)" % (oid, location.x, location.y, module.getMod().object_z)

    # give advice to agents.
    def give_advice():
        # reads advice from given filename
        def read_advice(filename):
            with open(filename, 'r') as file:
                module.getMod().setAdvice(file.read())

        guiMan.openFileChooserDialog("Open Advice File", True, read_advice)
        #adviceBox = gui.create_edit_box(guiMan, 'advice', OpenNero.Pos2i(0,0), OpenNero.Pos2i(200,20), 'hello')
        #module.getMod().setAdvice('{if v6 >= 0 then {output v8 0.99 v9 0.20}}')

    # save trace of agent
    def save_trace():
        guiMan.openFileChooserDialog("Save Example", True, lambda filename: module.getMod().save_trace(filename))
        print "saved trace in file"

    # load previously stored trace.
    def load_trace():
        guiMan.openFileChooserDialog("Load Example", True, lambda filename: module.getMod().load_trace(filename))
        print "loaded trace from file"

    # unload previously loaded trace.
    def unload_trace():
        module.getMod().unload_trace()
        print "unloaded trace"

    # activate changing the radius of circle in which cube moves
    def change_radius():
        module.getMod().modify_object_id['radius'] = module.getMod().key_object_id['1']
        module.getMod().saved_cursor = OpenNero.getSimContext().getMousePosition()

    # activate changing the speed of cube moving in a circle
    def change_speed():
        module.getMod().modify_object_id['speed'] = module.getMod().key_object_id['1']
        module.getMod().saved_cursor = OpenNero.getSimContext().getMousePosition()

    # if we are using the shaping approach and the user clicked on an object, then
    # show options to modify it.
    if module.getMod().approach == 'shaping' and selected_object_id in module.getMod().object_ids:
        #pathButton = gui.create_button(guiMan, 'path', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        #pathButton.OnMouseLeftClick = lambda: specify_object_path()
        #contextMenu.addItem('Specify object path', pathButton)

        moveButton = gui.create_button(guiMan, 'move', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        moveButton.OnMouseLeftClick = lambda: move_object()
        contextMenu.addItem('Move object', moveButton)

        rotateButton = gui.create_button(guiMan, 'rotate', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        rotateButton.OnMouseLeftClick = lambda: rotate_object()
        contextMenu.addItem('Rotate object', rotateButton)

        scaleButton = gui.create_button(guiMan, 'scale', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        scaleButton.OnMouseLeftClick = lambda: scale_object()
        contextMenu.addItem('Scale object', scaleButton)

        #colorButton = gui.create_button(guiMan, 'color', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        #colorButton.OnMouseLeftClick = lambda: color_object()
        #contextMenu.addItem('Color object', colorButton)

        removeButton = gui.create_button(guiMan, 'remove', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        removeButton.OnMouseLeftClick = lambda: remove_object()
        contextMenu.addItem('Remove object', removeButton)

    elif module.getMod().approach == 'shaping':
        wallButton = gui.create_button(guiMan, 'wall', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        wallButton.OnMouseLeftClick = lambda: add_wall()
        contextMenu.addItem('Add wall', wallButton)

        cubeButton = gui.create_button(guiMan, 'cube', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        cubeButton.OnMouseLeftClick = lambda: add_cube()
        contextMenu.addItem('Add cube', cubeButton)

        resetButton = gui.create_button(guiMan, 'reset', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        resetButton.OnMouseLeftClick = lambda: reset_objects()
        contextMenu.addItem('Reset objects', resetButton)

        if module.getMod().task == 'circle':
            radiusButton = gui.create_button(guiMan, 'radius', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
            radiusButton.OnMouseLeftClick = lambda: change_radius()
            contextMenu.addItem('Change radius', radiusButton)

            speedButton = gui.create_button(guiMan, 'speed', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
            speedButton.OnMouseLeftClick = lambda: change_speed()
            contextMenu.addItem('Change speed', speedButton)

    elif module.getMod().approach == 'advice':
        adviceButton = gui.create_button(guiMan, 'advice', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        adviceButton.OnMouseLeftClick = lambda: give_advice()
        contextMenu.addItem('Load advice', adviceButton)

    #elif module.getMod().approach == 'scripting':
    #    scriptingButton = gui.create_button(guiMan, 'scripting', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
    #    scriptingButton.OnMouseLeftClick = lambda: give_advice()
    #    contextMenu.addItem('Load script', scriptingButton)

    elif module.getMod().approach == 'example':
        if module.getMod().control == 'keyboard':
            saveTraceButton = gui.create_button(guiMan, 'save trace', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
            saveTraceButton.OnMouseLeftClick = lambda: save_trace()
            contextMenu.addItem('Save example', saveTraceButton)

        else:
            loadTraceButton = gui.create_button(guiMan, 'load trace', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
            loadTraceButton.OnMouseLeftClick = lambda: load_trace()
            contextMenu.addItem('Load example', loadTraceButton)

            unloadTraceButton = gui.create_button(guiMan, 'unload trace', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
            unloadTraceButton.OnMouseLeftClick = lambda: unload_trace()
            contextMenu.addItem('Unload example', unloadTraceButton)

        #enableBackpropButton = gui.create_button(guiMan, 'enable backprop', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        #enableBackpropButton.OnMouseLeftClick = lambda: module.getMod().enable_backprop()
        #contextMenu.addItem('Enable Backprop', enableBackpropButton)
        #
        #disableBackpropButton = gui.create_button(guiMan, 'disable backprop', OpenNero.Pos2i(0,0), OpenNero.Pos2i(0,0), '')
        #disableBackpropButton.OnMouseLeftClick = lambda: module.getMod().disable_backprop()
        #contextMenu.addItem('Disable Backprop', disableBackpropButton)

def mouse_action():
    if len(module.getMod().modify_object_id) == 0:
        return

    sim_context = OpenNero.getSimContext()
    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)

    if 'pos' in module.getMod().modify_object_id:
        module.getMod().setObjectPosition(module.getMod().modify_object_id['pos'], OpenNero.Vector3f(location.x, location.y, module.getMod().object_z))

    elif 'rot' in module.getMod().modify_object_id:
        position = sim_context.getObjectPosition(module.getMod().modify_object_id['rot'])
        angle = atan2(location.x-position.x, location.y-position.y)
        module.getMod().setObjectRotation(module.getMod().modify_object_id['rot'], OpenNero.Vector3f(0, 0, -degrees(angle)))

    elif 'scale' in module.getMod().modify_object_id:
        position = sim_context.getObjectPosition(module.getMod().modify_object_id['scale'])
        rotation = sim_context.getObjectRotation(module.getMod().modify_object_id['scale'])
        theta = radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x-position.x)*cos(theta) + (location.y-position.y)*sin(theta)
        localy = -(location.x-position.x)*sin(theta) + (location.y-position.y)*cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + fabs(localx)/module.getMod().scale_const
        scaley = 1 + fabs(localy)/module.getMod().scale_const
        if localx < 0: scalex = 1/scalex
        if localy < 0: scaley = 1/scaley

        module.getMod().setObjectScale(module.getMod().modify_object_id['scale'], OpenNero.Vector3f(scalex, scaley, 1))

    elif 'color' in module.getMod().modify_object_id:
        position = sim_context.getObjectPosition(module.getMod().modify_object_id['color'])
        rotation = sim_context.getObjectRotation(module.getMod().modify_object_id['color'])
        theta = radians(rotation.z)

        localx = position.x - location.x
        color_value = fabs(localx)*module.getMod().color_const
        if color_value > module.getMod().color_value_max: color_value = module.getMod().color_value_max
        color = int(color_value*255/module.getMod().color_value_max)
        if localx < 0: scolor = SColor(0, color, 0, 0)
        else: scolor = SColor(0, 0, 0, color)

        module.getMod().setObjectColor(module.getMod().modify_object_id['color'], scolor)

    elif 'radius' in module.getMod().modify_object_id:
        mousex = cursor.x - module.getMod().saved_cursor.x
        radius = module.getMod().current_radius + mousex*module.getMod().radius_const
        if radius < 0.0: radius = 0.0
        if radius > 1.0: radius = 1.0
        set_circle_object_radius(radius)
        module.getMod().saved_cursor = cursor
        label = str(module.getMod().current_radius*100) + "%"
        module.getMod().setObjectLabel(module.getMod().modify_object_id['radius'], label);

    elif 'speed' in module.getMod().modify_object_id:
        mousex = cursor.x - module.getMod().saved_cursor.x
        speed = module.getMod().current_speed + mousex*module.getMod().speed_const
        if speed < 0.0: speed = 0.0
        if speed > 1.0: speed = 1.0
        set_circle_object_speed(speed)
        module.getMod().saved_cursor = cursor
        label = str(module.getMod().current_speed*100) + "%"
        module.getMod().setObjectLabel(module.getMod().modify_object_id['speed'], label);

def reset_mouse_action():
    module.getMod().modify_object_id.clear()

# an empty function
def blank():
    pass

def createInputMapping():

    # create an io map
    ioMap = OpenNero.PyIOMap()

    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub )
    ioMap.BindKey( "KEY_CONTROL+KEY_KEY_A", "onPress", blank )
    ioMap.BindKey( "KEY_KEY_A", "onPress", blank )
    ioMap.BindKey( "KEY_F8", "onPress", enable_simdisplay )
    ioMap.BindKey( "KEY_F9", "onPress", disable_simdisplay )
    ioMap.BindMouseAction( "moveX", mouse_action )
    ioMap.BindMouseAction( "moveY", mouse_action )
    ioMap.BindMouseButton( "left", "onPress", reset_mouse_action )
    ioMap.BindMouseButton( "left", "onRelease", blank )
    ioMap.BindMouseButton( "middle", "onPress", blank )
    ioMap.BindMouseButton( "middle", "onRelease", blank )
    ioMap.BindMouseButton( "right", "onPress", show_context_menu )
    ioMap.BindMouseButton( "right", "onRelease", blank )
    ioMap.BindMouseAction( "scroll", blank )

    # place predefined objects using keyboard
    ioMap.BindKey( "KEY_KEY_1", "onPress", place_object1 )
    ioMap.BindKey( "KEY_KEY_2", "onPress", place_object2 )
    ioMap.BindKey( "KEY_KEY_3", "onPress", place_object3 )
    ioMap.BindKey( "KEY_KEY_4", "onPress", place_object4 )
    ioMap.BindKey( "KEY_KEY_5", "onPress", place_object5 )

    # FPS control
    #ioMap.BindKey( "KEY_KEY_A", "onHold", fps_strafe_left )
    #ioMap.BindKey( "KEY_KEY_D", "onHold", fps_strafe_right )
    #ioMap.BindKey( "KEY_KEY_S", "onHold", fps_back )
    #ioMap.BindKey( "KEY_KEY_W", "onHold", fps_forward )
    #ioMap.BindKey( "KEY_KEY_Q", "onHold", fps_turn_left )
    #ioMap.BindKey( "KEY_KEY_E", "onHold", fps_turn_right )

    # Keyboard control
    ioMap.BindKey( "KEY_RIGHT", "onHold", key_right )
    ioMap.BindKey( "KEY_LEFT", "onHold", key_left )
    ioMap.BindKey( "KEY_UP", "onHold", key_forward )
    ioMap.BindKey( "KEY_DOWN", "onHold", key_back )

    return ioMap

def ClientMain():
    global guiMan

    OpenNero.disable_ai()
    
    # create fog effect
    OpenNero.getSimContext().setFog()

    # don't show physics
    module.getMod().setup_world()
    
    guiMan = common.getGuiManager()

    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 3000
    camZoomSpeed   = 300
    cam = OpenNero.getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    
    def recenter(cam):
        def closure():
            cam.setPosition(OpenNero.Vector3f(1000, -120, 100))
            cam.setTarget(OpenNero.Vector3f(100, 100, 10))
        return closure
    
    recenter_cam = recenter(cam)
    recenter_cam()
    
    ioMap = createInputMapping()
    ioMap.BindKey("KEY_SPACE", "onPress", recenter_cam)
    OpenNero.getSimContext().setInputMapping(ioMap)
    
    OpenNero.getSimContext().addLightSource(OpenNero.Vector3f(1100, -500, 200), 2000)
    make_task_menu()
