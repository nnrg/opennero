from OpenNero import *
from random import seed, randint

# add the key and mouse bindings
from inputConfig import createInputMapping

from common import *
import common.gui as gui

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

    button_w = 250 # button width
    button_h = 30  # button height

    agentComboBox = gui.create_combo_box(guiMan, "agentComboBox", Pos2i(5,button_h + 5), Pos2i(button_w-10, button_h - 10))

    for agent_name, agent_function in AGENTS:
        agentComboBox.addItem(agent_name)

    newMazeButton = gui.create_button(guiMan, 'newMazeButton', Pos2i(5,5), Pos2i(button_w/3-10, button_h-10), '')
    newMazeButton.text = 'New Maze'
    newMazeButton.OnMouseLeftClick = lambda: getMod().generate_new_maze()
    
    def startAgent():
        i = agentComboBox.getSelected()
        (agent_name, agent_function) = AGENTS[i]
        print 'Starting', agent_name
        agent_function()

    startAgentButton = gui.create_button(guiMan, 'startAgentButton', Pos2i(button_w/3+5, 5), Pos2i(button_w/3-10, button_h-10), '')
    startAgentButton.text = 'Start'
    startAgentButton.OnMouseLeftClick = startAgent
                
    def pauseAgent(pauser):
        def closure():
            if pauser.text == 'Continue':
                pauser.text = 'Pause'
                enable_ai()
            else:
                pauser.text = 'Continue'
                disable_ai()
        return closure

    pauseAgentButton = gui.create_button(guiMan, 'pauseAgentButton', Pos2i(2*button_w/3+5, 5), Pos2i(button_w/3-10, button_h-10), '')
    pauseAgentButton.text = 'Pause'
    pauseAgentButton.OnMouseLeftClick = pauseAgent(pauseAgentButton)
    
    agentWindow = gui.create_window(guiMan, 'agentWindow', Pos2i(20, 20), Pos2i(button_w, 2*button_h+25), 'Agent')
    agentWindow.addChild(agentComboBox)
    agentWindow.addChild(newMazeButton)
    agentWindow.addChild(startAgentButton)
    agentWindow.addChild(pauseAgentButton)

    epsilon_percent = int(INITIAL_EPSILON * 100)
    epsilonValue = gui.create_text(guiMan, 'epsilonEditBox', Pos2i(260,0), Pos2i(100,30), str(epsilon_percent))

    epsilonLabel = gui.create_text(guiMan, 'epsilonLabel', Pos2i(10,0), Pos2i(100,30), 'Exploit-Explore:')

    # this can be used to adjust the exploration-exploitation tradeoff (fraction of
    # champion organisms in the case of rt-NEAT and fraction of greedy actions in the case
    # of the epsilon-greedy RL methods like Sarsa and Q-learning)
    epsilonScroll = gui.create_scroll_bar(guiMan, 'epsilonScroll', Pos2i(100,0), Pos2i(150,20), True)
    epsilonScroll.setMax(100)
    epsilonScroll.setLargeStep(10)
    epsilonScroll.setSmallStep(1)
    epsilonScroll.setPos(epsilon_percent)
    getMod().set_epsilon(INITIAL_EPSILON)
    epsilonScroll.OnScrollBarChange = epsilon_adjusted(epsilonScroll, epsilonValue)

    speedupValue = gui.create_text(guiMan, 'speedupEditBox', Pos2i(260, 30), Pos2i(100, 30), str(0))

    speedupLabel = gui.create_text(guiMan, 'speedupLabel', Pos2i(10, 30), Pos2i(100, 30), 'Speedup:')

    speedupScroll = gui.create_scroll_bar(guiMan, 'speedupScroll', Pos2i(100, 30), Pos2i(150,20), True)
    speedupScroll.setMax(100)
    speedupScroll.setLargeStep(10)
    speedupScroll.setSmallStep(1)
    speedupScroll.setPos(0)
    getMod().set_speedup(0)
    speedupScroll.OnScrollBarChange = speedup_adjusted(speedupScroll, speedupValue)

    paramWindow = gui.create_window(guiMan, 'paramWindow', Pos2i(20, 500), Pos2i(260,75), 'Parameters')
    paramWindow.addChild(epsilonLabel)
    paramWindow.addChild(epsilonScroll)
    paramWindow.addChild(epsilonValue)
    paramWindow.addChild(speedupLabel)
    paramWindow.addChild(speedupScroll)
    paramWindow.addChild(speedupValue)

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

def recenter(cam):
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
