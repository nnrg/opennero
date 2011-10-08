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
    #('Depth First Search',      lambda: getMod().start_dfs(),           False),
    #('Breadth First Search',    lambda: getMod().start_bfs(),           False),
    #('Single Agent A*',         lambda: getMod().start_astar(),         False),
    #('Teleporting A*',          lambda: getMod().start_astar2(),        False),
    #('Front A*',                lambda: getMod().start_astar3(),        False),
    #('First Person',            lambda: getMod().start_fps(),           False),
    ('Random Baseline',         lambda: getMod().start_random(),        False),
    ('Sarsa',                   lambda: getMod().start_sarsa(),         True),
    ('Q-Learning',              lambda: getMod().start_qlearning(),     True),
    ('CustomRL',               lambda: getMod().start_customrl(),      True),
    ('Harder Q-Learning',      lambda: getMod().start_qlearning(ContMazeEnvironment), True),
    ('Harder CustomRL',        lambda: getMod().start_customrl(ContMazeEnvironment), True),
]

class UI:
    pass

def CreateGui(guiMan):
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    ui = UI()

    window_w = 350 # width
    button_h = 30  # height

    x, y = 5, 3 * button_h + 5

    ui.agentComboBox = gui.create_combo_box(guiMan, "agentComboBox", Pos2i(x, y), Pos2i(window_w-10, button_h - 10))

    for agent_name, agent_function, ee_enabled in AGENTS:
        ui.agentComboBox.addItem(agent_name)

    x, y = 5, 0 * button_h

    # this can be used to adjust the exploration-exploitation tradeoff
    epsilon_percent = int(INITIAL_EPSILON * 100)
    ui.epsilonLabel = gui.create_text(guiMan, 'epsilonLabel', Pos2i(x, y), Pos2i(100,30), 'Exploit-Explore:')
    ui.epsilonScroll = gui.create_scroll_bar(guiMan, 'epsilonScroll', Pos2i(x + 100, y), Pos2i(190, 20), True)
    ui.epsilonValue = gui.create_text(guiMan, 'epsilonEditBox', Pos2i(x + window_w - 50, y), Pos2i(50,30), str(epsilon_percent))
    ui.epsilonScroll.setMax(100)
    ui.epsilonScroll.setLargeStep(10)
    ui.epsilonScroll.setSmallStep(1)
    ui.epsilonScroll.setPos(epsilon_percent)
    ui.epsilonScroll.enabled = False
    ui.epsilonValue.visible = False
    ui.epsilonLabel.visible = False
    ui.epsilonScroll.visible = False
    getMod().set_epsilon(INITIAL_EPSILON)
    ui.epsilonScroll.OnScrollBarChange = epsilon_adjusted(ui)

    x, y = 5, 2 * button_h

    ui.startAgentButton = gui.create_button(guiMan, 'startAgentButton', Pos2i(x + 0*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    ui.pauseAgentButton = gui.create_button(guiMan, 'pauseAgentButton', Pos2i(x + 1*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    ui.startAgentButton.text = 'Start'
    ui.pauseAgentButton.text = 'Pause'
    ui.pauseAgentButton.enabled = False
    ui.startAgentButton.OnMouseLeftClick = startAgent(ui)
    ui.pauseAgentButton.OnMouseLeftClick = pauseAgent(ui)

    ui.newMazeButton = gui.create_button(guiMan, 'newMazeButton', Pos2i(x + 2*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    ui.newMazeButton.text = 'New Maze'
    ui.newMazeButton.OnMouseLeftClick = lambda: getMod().generate_new_maze()

    ui.helpButton = gui.create_button(guiMan, 'helpButton', Pos2i(x + 3*(window_w-20)/4, y), Pos2i(window_w/4-10, button_h-5), '')
    ui.helpButton.text = 'Help'
    ui.helpButton.OnMouseLeftClick = openWiki('MazeMod')

    x, y = 5, 1 * button_h

    # this can adjust the speed of the simulation
    ui.speedupLabel = gui.create_text(guiMan, 'speedupLabel', Pos2i(x, y), Pos2i(100, 30), 'Speedup:')
    ui.speedupScroll = gui.create_scroll_bar(guiMan, 'speedupScroll', Pos2i(x + 100, y), Pos2i(190,20), True)
    ui.speedupValue = gui.create_text(guiMan, 'speedupEditBox', Pos2i(x + window_w - 50, y), Pos2i(50, 30), str(0))
    ui.speedupScroll.setMax(100)
    ui.speedupScroll.setLargeStep(10)
    ui.speedupScroll.setSmallStep(1)
    ui.speedupScroll.setPos(0)
    getMod().set_speedup(0)
    ui.speedupScroll.OnScrollBarChange = speedup_adjusted(ui)

    ui.agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(window_w, 4*button_h+25), 'Agent')
    ui.agentWindow.addChild(ui.agentComboBox)
    ui.agentWindow.addChild(ui.newMazeButton)
    ui.agentWindow.addChild(ui.startAgentButton)
    ui.agentWindow.addChild(ui.pauseAgentButton)
    ui.agentWindow.addChild(ui.helpButton)
    ui.agentWindow.addChild(ui.epsilonLabel)
    ui.agentWindow.addChild(ui.epsilonScroll)
    ui.agentWindow.addChild(ui.epsilonValue)
    ui.agentWindow.addChild(ui.speedupLabel)
    ui.agentWindow.addChild(ui.speedupScroll)
    ui.agentWindow.addChild(ui.speedupValue)

def epsilon_adjusted(ui):
    # generate a closure that will be called whenever the epsilon slider is adjusted
    ui.epsilonValue.text = str(ui.epsilonScroll.getPos())
    getMod().set_epsilon(float(ui.epsilonScroll.getPos())/100)
    def closure():
        ui.epsilonValue.text = str(ui.epsilonScroll.getPos())
        getMod().set_epsilon(float(ui.epsilonScroll.getPos())/100)
    return closure

def speedup_adjusted(ui):
    # generate a closure that will be called whenever the speedup slider is adjusted
    ui.speedupValue.text = str(ui.speedupScroll.getPos())
    getMod().set_speedup(float(ui.speedupScroll.getPos())/100)
    def closure():
        ui.speedupValue.text = str(ui.speedupScroll.getPos())
        getMod().set_speedup(float(ui.speedupScroll.getPos())/100)
    return closure

def startAgent(ui):
    """ return a function that starts or stops the agent """
    def closure():
        if ui.startAgentButton.text == 'Start':
            i = ui.agentComboBox.getSelected()
            (agent_name, agent_function, ee_enabled) = AGENTS[i]
            if ee_enabled:
                ui.epsilonScroll.enabled = True
                ui.epsilonValue.visible = True
                ui.epsilonLabel.visible = True
                ui.epsilonScroll.visible = True
            print 'Starting', agent_name
            agent_function()
            ui.pauseAgentButton.text = 'Pause'
            ui.pauseAgentButton.enabled = True
            ui.startAgentButton.text = 'Reset'
            ui.agentComboBox.enabled = False
        else:
            getMod().stop_maze()
            disable_ai()
            ui.epsilonScroll.enabled = False
            ui.epsilonValue.visible = False
            ui.epsilonLabel.visible = False
            ui.epsilonScroll.visible = False
            get_environment().cleanup()
            ui.startAgentButton.text = 'Start'
            ui.pauseAgentButton.text = 'Pause'
            ui.pauseAgentButton.enabled = False
            ui.agentComboBox.enabled = True
    return closure

def pauseAgent(ui):
    """ return a function that pauses and continues the agent """
    def closure():
        if ui.pauseAgentButton.text == 'Continue':
            ui.pauseAgentButton.text = 'Pause'
            enable_ai()
        else:
            ui.pauseAgentButton.text = 'Continue'
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
