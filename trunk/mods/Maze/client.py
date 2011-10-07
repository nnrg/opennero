from OpenNero import *
from random import seed, randint

# add the key and mouse bindings
from inputConfig import createInputMapping

from common import *
import common.gui as gui
from common.module import openWiki

from Maze.module import getMod, delMod
from Maze.constants import *
from Maze.environment import ContMazeEnvironment

# Agents and the functions that start them

AGENTS = [
    ('Depth First Search', lambda: getMod().start_dfs()),
    ('Breadth First Search', lambda: getMod().start_bfs()),
    ('Single Agent A*', lambda: getMod().start_astar()),
    ('Teleporting A*', lambda: getMod().start_astar2()),
    ('Front A*', lambda: getMod().start_astar3()),
    ('First Person', lambda: getMod().start_fps()),
    ('Random Baseline', lambda: getMod().start_random()),
    ('Sarsa', lambda: getMod().start_sarsa()),
    ('Q-Learning', lambda: getMod().start_qlearning()),
#    ('CustomRL', lambda: getMod().start_customrl()),
#    ('Harder Q-Learning', lambda: getMod().start_qlearning(ContMazeEnvironment)),
#    ('Harder CustomRL', lambda: getMod().start_customrl(ContMazeEnvironment)),
]

def CreateGui(guiMan):
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    window_w = 350 # width
    button_h = 30  # height

    x, y = 5, 3 * button_h + 5

    agentComboBox = gui.create_combo_box(guiMan, "agentComboBox", Pos2i(x, y), Pos2i(window_w-10, button_h - 10))

    for agent_name, agent_function in AGENTS:
        agentComboBox.addItem(agent_name)

    x, y = 5, 2 * button_h

    startAgentButton = gui.create_button(guiMan, 'startAgentButton', Pos2i(x + 0*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    pauseAgentButton = gui.create_button(guiMan, 'pauseAgentButton', Pos2i(x + 1*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    startAgentButton.text = 'Start'
    pauseAgentButton.text = 'Pause'
    pauseAgentButton.enabled = False
    startAgentButton.OnMouseLeftClick = startAgent(startAgentButton, pauseAgentButton, agentComboBox)
    pauseAgentButton.OnMouseLeftClick = pauseAgent(startAgentButton, pauseAgentButton)

    newMazeButton = gui.create_button(guiMan, 'newMazeButton', Pos2i(x + 2*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    newMazeButton.text = 'New Maze'
    newMazeButton.OnMouseLeftClick = lambda: getMod().generate_new_maze()

    helpButton = gui.create_button(guiMan, 'helpButton', Pos2i(x + 3*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    helpButton.text = 'Help'
    helpButton.OnMouseLeftClick = openWiki('MazeMod')

    x, y = 5, 0 * button_h

    # this can be used to adjust the exploration-exploitation tradeoff
    epsilon_percent = int(INITIAL_EPSILON * 100)
    epsilonLabel = gui.create_text(guiMan, 'epsilonLabel', Pos2i(x, y), Pos2i(100,30), 'Exploit-Explore:')
    epsilonScroll = gui.create_scroll_bar(guiMan, 'epsilonScroll', Pos2i(x + 100, y), Pos2i(150,20), True)
    epsilonValue = gui.create_text(guiMan, 'epsilonEditBox', Pos2i(x + window_w - 50, y), Pos2i(50,30), str(epsilon_percent))
    epsilonScroll.setMax(100)
    epsilonScroll.setLargeStep(10)
    epsilonScroll.setSmallStep(1)
    epsilonScroll.setPos(epsilon_percent)
    getMod().set_epsilon(INITIAL_EPSILON)
    epsilonScroll.OnScrollBarChange = epsilon_adjusted(epsilonScroll, epsilonValue)

    x, y = 5, 1 * button_h

    # this can adjust the speed of the simulation
    speedupLabel = gui.create_text(guiMan, 'speedupLabel', Pos2i(x, y), Pos2i(100, 30), 'Speedup:')
    speedupScroll = gui.create_scroll_bar(guiMan, 'speedupScroll', Pos2i(x + 100, y), Pos2i(150,20), True)
    speedupValue = gui.create_text(guiMan, 'speedupEditBox', Pos2i(x + window_w - 50, y), Pos2i(50, 30), str(0))
    speedupScroll.setMax(100)
    speedupScroll.setLargeStep(10)
    speedupScroll.setSmallStep(1)
    speedupScroll.setPos(0)
    getMod().set_speedup(0)
    speedupScroll.OnScrollBarChange = speedup_adjusted(speedupScroll, speedupValue)

    agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(window_w, 4*button_h+25), 'Agent')
    agentWindow.addChild(agentComboBox)
    agentWindow.addChild(newMazeButton)
    agentWindow.addChild(startAgentButton)
    agentWindow.addChild(pauseAgentButton)
    agentWindow.addChild(helpButton)
    agentWindow.addChild(epsilonLabel)
    agentWindow.addChild(epsilonScroll)
    agentWindow.addChild(epsilonValue)
    agentWindow.addChild(speedupLabel)
    agentWindow.addChild(speedupScroll)
    agentWindow.addChild(speedupValue)

def epsilon_adjusted(scroll, value):
    # generate a closure that will be called whenever the epsilon slider is adjusted
    value.text = str(scroll.getPos())
    getMod().set_epsilon(float(scroll.getPos())/100)
    def closure():
        value.text = str(scroll.getPos())
        getMod().set_epsilon(float(scroll.getPos())/100)
    return closure

def speedup_adjusted(scroll, value):
    # generate a closure that will be called whenever the speedup slider is adjusted
    value.text = str(scroll.getPos())
    getMod().set_speedup(float(scroll.getPos())/100)
    def closure():
        value.text = str(scroll.getPos())
        getMod().set_speedup(float(scroll.getPos())/100)
    return closure

def startAgent(starter, pauser, combo_box):
    """ return a function that starts or stops the agent """
    def closure():
        if starter.text == 'Start':
            i = combo_box.getSelected()
            (agent_name, agent_function) = AGENTS[i]
            print 'Starting', agent_name
            agent_function()
            pauser.text = 'Pause'
            pauser.enabled = True
            starter.text = 'Reset'
        else:
            getMod().stop_maze()
            disable_ai()
            get_environment().cleanup()
            starter.text = 'Start'
            pauser.text = 'Pause'
            pauser.enabled = False
    return closure

def pauseAgent(starter, pauser):
    """ return a function that pauses and continues the agent """
    def closure():
        if pauser.text == 'Continue':
            pauser.text = 'Pause'
            enable_ai()
        else:
            pauser.text = 'Continue'
            disable_ai()
    return closure

def recenter(cam):
    """ return a function that recenters the camera """
    def closure():
        cam.setPosition(Vector3f(0, 0, 80))
        cam.setTarget(Vector3f(GRID_DX * ROWS / 2, GRID_DY * COLS / 2, 0))
    return closure

def ClientMain():
    # create fog effect
    getSimContext().setFog()

    # don't show physics
    # disable_physics()

    # add a camera
    camRotateSpeed = 100
    camMoveSpeed   = 3000
    camZoomSpeed   = 500
    cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
    cam.setFarPlane(5000)
    cam.setEdgeScroll(False)
    recenter_cam = recenter(cam) # create a closure to avoid having a global variable
    recenter_cam() # call the recenter function

    # load the background
    addObject("data/terrain/Sea.xml", Vector3f(-3000 + NUDGE_X,-3000 + NUDGE_Y,-20))
    addObject("data/terrain/IslandTerrain.xml", Vector3f(-1100 + NUDGE_X, -2400 + NUDGE_Y, -17), Vector3f(0,0,-45))
    addSkyBox("data/sky/irrlicht2")

    # load the maze
    getSimContext().addLightSource(Vector3f(-500,-500,1000), 1500)
    getMod().add_maze()

    # load the GUI
    CreateGui(getGuiManager())

    # create the key binding
    ioMap = createInputMapping()
    ioMap.BindKey( "KEY_SPACE", "onPress", recenter_cam )
    getSimContext().setInputMapping(ioMap)
