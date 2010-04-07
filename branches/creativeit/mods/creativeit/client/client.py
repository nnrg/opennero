from OpenNero import *
from random import seed, randint
from math import *
from inputConfig import *
from common.client import *
from creativeit import *

# globals - remove me?
clientContext = None
client = None

# called from GUI
def switchToHub():
    switchMod('hub', 'hub:common')

# control for keyboard agent
def key_left():
    global clientContext
    clientContext.server.control_key('left')
def key_right():
    global clientContext
    clientContext.server.control_key('right')
def key_forward():
    global clientContext
    clientContext.server.control_key('fwd')
def key_back():
    global clientContext
    clientContext.server.control_key('back')

# start/stop tracing agent
def toggle_tracing():
    global clientContext
    guiMan = clientContext.guiManager
    if clientContext.tracing:
        clientContext.server.stop_tracing()
        guiMan.openFileChooserDialog("Save Trace File", True, lambda filename: clientContext.server.save_trace(filename))
        clientContext.tracing = False
    else:
        clientContext.server.start_tracing()
        clientContext.tracing = True


# enable simulation display
def enable_simdisplay():
    global clientContext
    clientContext.server.enable_simdisplay()


# disable simulation display
def disable_simdisplay():
    global clientContext
    clientContext.server.disable_simdisplay()


def show_context_menu():
    global clientContext
    guiMan = clientContext.guiManager

    if len(clientContext.modify_object_id) > 0 or guiMan.isOpenFileChooserDialog():
        return

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    # find the screen position at which to open the context menu
    cursor = clientContext.sim_context.getMousePosition()
    location = clientContext.sim_context.getClickPosition(cursor.x, cursor.y)
    selected_object_id = clientContext.sim_context.getEntityIdUnderMouse(cursor.x, cursor.y)

    contextMenu = gui.create_context_menu(guiMan, 'context', cursor)

    # specify the path of the selected object
    def specify_object_path():
        # reads path specified as python code from given filename
        def read_path(filename):
            with open(filename, 'r') as file:
                code = compile(file.read(), filename, 'eval')
            clientContext.server.setObjectPath(code, selected_object_id)

        guiMan.openFileChooserDialog("Open Object Path File", True, read_path)

    # activate the selected object to be moved
    def move_object():
        clientContext.modify_object_id['pos'] = selected_object_id

    # activate the selected object to be rotated
    def rotate_object():
        clientContext.modify_object_id['rot'] = selected_object_id

    # activate the selected object to be scaled
    def scale_object():
        clientContext.modify_object_id['scale'] = selected_object_id

    # activate the selected object to be colored
    def color_object():
        clientContext.modify_object_id['color'] = selected_object_id

    # remove the selected object from the world
    def remove_object():
        clientContext.object_ids.remove(selected_object_id)
        clientContext.server.removeObject(selected_object_id)

    # add a wall at the current cursor location
    def add_wall():
        clientContext.object_ids.add(clientContext.sim_context.getNextFreeId())
        clientContext.server.addWall(Vector3f(location.x, location.y, clientContext.object_z))

    # add a cube at the current cursor location
    def add_cube():
        cube_id = clientContext.sim_context.getNextFreeId()
        clientContext.object_ids.add(cube_id)
        clientContext.server.addCube(Vector3f(location.x, location.y, clientContext.object_z))

    # give advice to agents.
    def give_advice():
        # reads advice from given filename
        def read_advice(filename):
            with open(filename, 'r') as file:
                clientContext.server.setAdvice(file.read())

        guiMan.openFileChooserDialog("Open Advice File", True, read_advice)
        #adviceBox = gui.create_edit_box(guiMan, 'advice', Pos2i(0,0), Pos2i(200,20), 'hello')
        #clientContext.server.setAdvice('{if v6 >= 0 then {output v8 0.99 v9 0.20}}')

    # load previously stored trace.
    def load_trace():
        guiMan.openFileChooserDialog("Load Trace File", True, lambda filename: clientContext.server.load_trace(filename))

    # if the user clicked on an object they added previously, show the options
    # to modify it; otherwise show general options.
    if selected_object_id in clientContext.object_ids:
        pathButton = gui.create_button(guiMan, 'path', Pos2i(0,0), Pos2i(0,0), '')
        pathButton.OnMouseLeftClick = lambda: specify_object_path()
        contextMenu.addItem('Specify object path', pathButton)

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
        rtneatButton.OnMouseLeftClick = lambda: clientContext.server.start_rtneat()
        contextMenu.addItem('Run rtNEAT', rtneatButton)

        scriptedButton = gui.create_button(guiMan, 'scripted', Pos2i(0,0), Pos2i(0,0), '')
        scriptedButton.OnMouseLeftClick = lambda: clientContext.server.start_scripted()
        contextMenu.addItem('Run scripted', scriptedButton)

        keyboardButton = gui.create_button(guiMan, 'keyboard', Pos2i(0,0), Pos2i(0,0), '')
        keyboardButton.OnMouseLeftClick = lambda: clientContext.server.start_keyboard()
        contextMenu.addItem('Run keyboard', keyboardButton)

        adviceButton = gui.create_button(guiMan, 'advice', Pos2i(0,0), Pos2i(0,0), '')
        adviceButton.OnMouseLeftClick = lambda: give_advice()
        contextMenu.addItem('Give advice', adviceButton)

        loadTraceButton = gui.create_button(guiMan, 'load trace', Pos2i(0,0), Pos2i(0,0), '')
        loadTraceButton.OnMouseLeftClick = lambda: load_trace()
        contextMenu.addItem('Load trace', loadTraceButton)

        unloadTraceButton = gui.create_button(guiMan, 'unload trace', Pos2i(0,0), Pos2i(0,0), '')
        unloadTraceButton.OnMouseLeftClick = lambda: clientContext.server.unload_trace()
        contextMenu.addItem('Unload trace', unloadTraceButton)

        enableBackpropButton = gui.create_button(guiMan, 'enable backprop', Pos2i(0,0), Pos2i(0,0), '')
        enableBackpropButton.OnMouseLeftClick = lambda: clientContext.server.enable_backprop()
        contextMenu.addItem('Enable Backprop', enableBackpropButton)

        disableBackpropButton = gui.create_button(guiMan, 'disable backprop', Pos2i(0,0), Pos2i(0,0), '')
        disableBackpropButton.OnMouseLeftClick = lambda: clientContext.server.disable_backprop()
        contextMenu.addItem('Disable Backprop', disableBackpropButton)


def mouse_action():
    global clientContext
    if len(clientContext.modify_object_id) == 0:
        return

    cursor = clientContext.sim_context.getMousePosition()
    location = clientContext.sim_context.getClickPosition(cursor.x, cursor.y)

    if 'pos' in clientContext.modify_object_id:
        clientContext.server.setObjectPosition(clientContext.modify_object_id['pos'], Vector3f(location.x, location.y, clientContext.object_z))

    elif 'rot' in clientContext.modify_object_id:
        position = clientContext.sim_context.getObjectPosition(clientContext.modify_object_id['rot'])
        angle = atan2(location.x-position.x, location.y-position.y)
        clientContext.server.setObjectRotation(clientContext.modify_object_id['rot'], Vector3f(0, 0, -degrees(angle)))       

    elif 'scale' in clientContext.modify_object_id:
        position = clientContext.sim_context.getObjectPosition(clientContext.modify_object_id['scale'])
        rotation = clientContext.sim_context.getObjectRotation(clientContext.modify_object_id['scale'])
        theta = radians(rotation.z)

        # calculate mouse location in the frame of reference of the object
        localx = (location.x-position.x)*cos(theta) + (location.y-position.y)*sin(theta)
        localy = -(location.x-position.x)*sin(theta) + (location.y-position.y)*cos(theta)

        # scale < 1 if local coordinate is -ve and scale > 1 otherwise
        scalex = 1 + fabs(localx)/clientContext.scale_const
        scaley = 1 + fabs(localy)/clientContext.scale_const
        if localx < 0: scalex = 1/scalex
        if localy < 0: scaley = 1/scaley

        clientContext.server.setObjectScale(clientContext.modify_object_id['scale'], Vector3f(scalex, scaley, 1))       

    elif 'color' in clientContext.modify_object_id:
        position = clientContext.sim_context.getObjectPosition(clientContext.modify_object_id['color'])
        rotation = clientContext.sim_context.getObjectRotation(clientContext.modify_object_id['color'])
        theta = radians(rotation.z)

        localx = position.x - location.x
        color_value = fabs(localx)*clientContext.color_const
        if color_value > clientContext.color_value_max: color_value = clientContext.color_value_max
        color = int(color_value*255/clientContext.color_value_max)
        if localx < 0: scolor = SColor(0, color, 0, 0)
        else: scolor = SColor(0, 0, 0, color)
        
        clientContext.server.setObjectColor(clientContext.modify_object_id['color'], scolor)       


def reset_mouse_action():
    global clientContext
    clientContext.modify_object_id.clear()


#########################################################

# the client context specialized for the creativeit mod
class CreativeITClient(ClientContext):
    def __init__(self):
        ClientContext.__init__(self)
        self.object_ids = set()         # Ids of objects added to the world by the user
        self.object_z = 2               # Z cooridinate of objects added by the user
        self.modify_object_id = dict()  # Id of object whose pos/rot/scale/color is to be modified            
        self.scale_const = 10           # Constant used for scaling objects by moving mouse
        self.color_value_max = 100      # Maximum of color values associated with objects
        self.color_const = 4            # Constant used for adjusting color of objects by moving mouse
        self.tracing = False            # Flag indicating if agent is being traced


def ClientMain():
    # register our client with the application
    global clientContext
    global client
    global cam
    clientContext = CreativeITClient()
    client = Client(clientContext)
    RegisterClient(client)
    # create the io map
    getSimContext().setInputMapping(createInputMapping())
    # create fog effect
    getSimContext().setFog()
    # don't show physics
    disable_physics()
    setup_world()
    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 3000
    camZoomSpeed   = 300
    cam = clientContext.sim_context.addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    recenter()
    clientContext.sim_context.addLightSource(Vector3f(1100, -500, 200), 2000)


def recenter():
    global cam
    cam.setPosition(Vector3f(1000, -120, 100))
    cam.setTarget(Vector3f(100, 100, 10))


def setup_world():
    global clientContext
    server = clientContext.server

    #server.addObject("data/terrain/SurroundTerrain.xml", Vector3f(-260, -750, -17), Vector3f(90,0,0))
    server.addObject("data/terrain/Sea.xml", Vector3f(-3000,-3000,-20))
    server.addObject("data/terrain/IslandTerrain.xml", Vector3f(-1100, -2400, -17), Vector3f(0,0,1))

    #server.addAxes()
    server.addSkyBox("data/sky/irrlicht2")

    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(940, -90, clientContext.object_z))
    # 
    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(950, -5, clientContext.object_z))

    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(870, -100, clientContext.object_z))
    # 
    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(930, -100, clientContext.object_z))
    # 
    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(870, -15, clientContext.object_z))
    # 
    #cube_id = clientContext.sim_context.getNextFreeId()
    #clientContext.object_ids.add(cube_id)
    #clientContext.server.addCube(Vector3f(930, -15, clientContext.object_z))
