import math

import OpenNero
import common
import common.gui as gui
import constants
import module
import agent

guiMan = None
modify_object_id = {}
object_ids = {}

def showDisplayHint():
    hint = constants.nextDisplayHint()
    if hint is None:
        OpenNero.setWindowCaption("")
    else:
        OpenNero.setWindowCaption("Displaying: " + hint)

def switchToHub():
    module.delMod()
    OpenNero.switchMod('hub', '', 'hub:common')

# control for keyboard agent
def key(key):
    agent.KeyboardAgent.keys.add(key) # tell the keyboard agent that a key has been hit this frame
    
def blank():
    pass

def createInputMapping():
    from client import show_context_menu, mouse_action, reset_mouse_action
    # create an io map
    ioMap = OpenNero.PyIOMap()
    # bind our keys
    ioMap.ClearMappings()
    ioMap.BindKey( "KEY_ESCAPE", "onPress", switchToHub)
    ioMap.BindKey( "KEY_F1", "onPress", common.openWiki('NeroMod') )
    ioMap.BindKey( "KEY_F2", "onPress", showDisplayHint )

    # Keyboard control
    ioMap.BindKey( "KEY_RIGHT", "onHold", lambda: key('right') )
    ioMap.BindKey( "KEY_LEFT", "onHold", lambda: key('left') )
    ioMap.BindKey( "KEY_UP", "onHold", lambda: key('fwd') )
    ioMap.BindKey( "KEY_DOWN", "onHold", lambda: lambda: key('back') )
    
    ioMap.BindMouseAction( "moveX", mouse_action)
    ioMap.BindMouseAction( "moveY", mouse_action)
    ioMap.BindMouseButton( "left"  , "onPress"    , reset_mouse_action)
    ioMap.BindMouseButton( "left"  , "onRelease"  , blank)
    ioMap.BindMouseButton( "middle", "onPress"    , blank)
    ioMap.BindMouseButton( "middle", "onRelease"  , blank)
    ioMap.BindMouseButton( "right" , "onPress"    , show_context_menu)
    ioMap.BindMouseButton( "right" , "onRelease"  , blank)

    ioMap.BindMouseAction( "scroll", blank)

    return ioMap

def show_context_menu():
    global modify_object_id
    global object_ids

    if len(modify_object_id) > 0:
        return

    guiMan.setTransparency(1.0)

    sim_context = OpenNero.getSimContext()

    # find the screen position at which to open the context menu
    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)
    selected_object_id = sim_context.getClickedEntityId(cursor.x, cursor.y)

    if selected_object_id not in object_ids and abs(location.z) > 10: return

    print "location:", location

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    def add_wall():
        obj_id = common.addObject(
            "data/shapes/cube/Cube.xml",
            OpenNero.Vector3f(location.x, location.y, constants.HEIGHT + constants.OFFSET),
            OpenNero.Vector3f(0, 0, 90),
            scale=OpenNero.Vector3f(5, 30, constants.HEIGHT*2),
            type=1)
        object_ids[obj_id] = set(['rotate', 'move', 'scale', 'remove'])

    def rotate_object():
        modify_object_id['rot'] = selected_object_id

    def scale_object():
        modify_object_id['scale'] = selected_object_id

    def move_object():
        modify_object_id['move'] = selected_object_id

    def remove_flag():
        module.getMod().remove_flag()

    def place_flag():
        module.getMod().change_flag([location.x, location.y, 0])

    def place_basic_turret():
        obj_id = module.getMod().place_basic_turret([location.x, location.y, 0])
        object_ids[obj_id] = set(['move', 'remove'])

    def set_spawn_1():
        module.getMod().set_spawn(location.x, location.y, constants.OBJECT_TYPE_TEAM_0)

    def set_spawn_2():
        module.getMod().set_spawn(location.x, location.y, constants.OBJECT_TYPE_TEAM_1)

    def remove_wall():
        common.removeObject(selected_object_id)

    if selected_object_id in object_ids:
        operations = object_ids[selected_object_id]

        if 'rotate' in operations:
            rotateButton = gui.create_button(guiMan, 'rotate', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
            rotateButton.OnMouseLeftClick = lambda: rotate_object()
            contextMenu.addItem('Rotate Object', rotateButton)

        if 'scale' in operations:
            scaleButton = gui.create_button(guiMan, 'scale', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
            scaleButton.OnMouseLeftClick = lambda: scale_object()
            contextMenu.addItem('Scale Object', scaleButton)

        if 'move' in operations:
            moveButton = gui.create_button(guiMan, 'move', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
            moveButton.OnMouseLeftClick = lambda: move_object()
            contextMenu.addItem('Move Object', moveButton)

        if 'remove' in operations:
            removeButton = gui.create_button(guiMan, 'remove', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
            removeButton.OnMouseLeftClick = lambda: remove_wall()
            contextMenu.addItem('Remove Object', removeButton)

    else:
        wallButton = gui.create_button(guiMan, 'wall', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        wallButton.OnMouseLeftClick = lambda: add_wall()
        contextMenu.addItem('Add wall', wallButton)

        rmFlagButton = gui.create_button(guiMan, 'flag', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        rmFlagButton.OnMouseLeftClick = lambda: remove_flag()
        contextMenu.addItem('Remove Flag', rmFlagButton)

        flagButton = gui.create_button(guiMan, 'flag', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        flagButton.OnMouseLeftClick = lambda: place_flag()
        contextMenu.addItem('Place Flag', flagButton)

        turretButton = gui.create_button(guiMan, 'b_turret', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        turretButton.OnMouseLeftClick = lambda: place_basic_turret()
        contextMenu.addItem('Place Basic Turret', turretButton)

        spawn1Button = gui.create_button(guiMan, 'blue spawn', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        spawn1Button.OnMouseLeftClick = lambda: set_spawn_1()
        contextMenu.addItem('Set Blue Spawn Location', spawn1Button)

        spawn2Button = gui.create_button(guiMan, 'red spawn', OpenNero.Pos2i(0, 0), OpenNero.Pos2i(0, 0), '')
        spawn2Button.OnMouseLeftClick = lambda: set_spawn_2()
        contextMenu.addItem('Set Red Spawn Location', spawn2Button)


def reset_mouse_action():
    global modify_object_id
    modify_object_id = {}


def mouse_action():
    global modify_object_id
    global object_ids

    if len(modify_object_id) == 0:
        return

    sim_context = OpenNero.getSimContext()

    cursor = sim_context.getMousePosition()
    location = sim_context.getClickedPosition(cursor.x, cursor.y)

    if 'move' in modify_object_id:
        pos = sim_context.getObjectPosition(modify_object_id['move'])
        pos.x = location.x
        pos.y = location.y
        sim_context.setObjectPosition(modify_object_id['move'], pos)

    if 'rot' in modify_object_id:
        position = sim_context.getObjectPosition(modify_object_id['rot'])
        angle = math.atan2(location.x - position.x, location.y - position.y)
        sim_context.setObjectRotation(modify_object_id['rot'], OpenNero.Vector3f(0, 0, -math.degrees(angle)))

    if 'scale' in modify_object_id:
        position = sim_context.getObjectPosition(modify_object_id['scale'])
        rotation = sim_context.getObjectRotation(modify_object_id['scale'])
        theta = math.radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x - position.x) * math.cos(theta) + (location.y - position.y) * math.sin(theta)
        localy = -(location.x - position.x) * math.sin(theta) + (location.y - position.y) * math.cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + math.fabs(localx)
        scaley = 1 + math.fabs(localy)

        if localx < 0: scalex = 1 / scalex
        if localy < 0: scaley = 1 / scaley

        prev_scale = sim_context.getObjectScale(modify_object_id['scale'])
        sim_context.setObjectScale(modify_object_id['scale'], OpenNero.Vector3f(scalex, scaley, prev_scale.z))


def ClientMain():
    global modify_object_id
    global object_ids
    global guiMan

    OpenNero.disable_ai()

    if not module.getMod().setup_map():
        switchToHub()
        return

    # add a light source
    OpenNero.getSimContext().addLightSource(OpenNero.Vector3f(500, -500, 1000), 1500)

    common.addSkyBox("data/sky/irrlicht2")

    # setup the gui
    guiMan = common.getGuiManager()
    object_ids = {}
    modify_object_id = {}

    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 15000
    camZoomSpeed   = 200
    cam = OpenNero.getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(40000)
    cam.setEdgeScroll(False)

    def recenter(cam):
        def closure():
            cam.setPosition(OpenNero.Vector3f(0, 0, 100))
            cam.setTarget(OpenNero.Vector3f(100, 100, 0))
        return closure

    recenter_cam = recenter(cam)
    recenter_cam()

    # create the io map
    ioMap = createInputMapping()
    ioMap.BindKey("KEY_SPACE", "onPress", recenter_cam)
    OpenNero.getSimContext().setInputMapping(ioMap)
