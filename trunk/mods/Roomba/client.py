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

def remove_bots_closure(removeBotsButton, addBotsButton):
    def closure():
        removeBotsButton.enabled = False
        addBotsButton.enabled = True
        getMod().remove_bots()
    return closure
    
def add_bots_closure(removeBotsButton, addBotsButton, botTypeBox, numBotBox):
    def closure():
        removeBotsButton.enabled = True
        addBotsButton.enabled = False
        getMod().add_bots(botTypeBox.text, numBotBox.text)
    return closure

def CreateGui(guiMan):

    window_width = 250 # width

    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")
    
    botTypeBox = gui.create_edit_box(guiMan, 'botType', Pos2i(10,10), Pos2i(110,30), 'Script')
    numBotBox = gui.create_edit_box(guiMan, 'numBot', Pos2i(130,10), Pos2i(40,30), '5')
    
    addBotButton = gui.create_button(guiMan, 'addBot', Pos2i(180,10), Pos2i(60,30), '')
    addBotButton.text = "Add bots"
    
    changeBotButton = gui.create_button(guiMan, 'changeBot', Pos2i(10,50), Pos2i(230,30), '')
    changeBotButton.text = "Switch to rtNEAT"
    changeBotButton.OnMouseLeftClick = lambda:toggle_bot_type(changeBotButton, botTypeBox)

    w = (window_width - 40) / 3

    pauseButton = gui.create_button( guiMan, 'pause', Pos2i(10,90), Pos2i(w,30), '' )
    pauseButton.text = 'Pause!'
    pauseButton.OnMouseLeftClick = lambda:toggle_ai_callback(pauseButton)
    
    removeBotButton = gui.create_button(guiMan, 'cleanBot', Pos2i(10 + (w + 10),90), Pos2i(w,30), '')
    removeBotButton.text = "Remove bots"
    
    exitButton = gui.create_button(guiMan, 'exit', Pos2i(10 + 2 * (w + 10),90), Pos2i(w,30), '')
    exitButton.text = "Exit"
    exitButton.OnMouseLeftClick = lambda: switchToHub() 
    
    addBotButton.OnMouseLeftClick = add_bots_closure(removeBotButton, addBotButton, botTypeBox, numBotBox)
    removeBotButton.OnMouseLeftClick = remove_bots_closure(removeBotButton, addBotButton)
    addBotButton.enabled = True
    removeBotButton.enabled = False

    AiWindow = gui.create_window( guiMan, 'AiWindow', Pos2i(530,20), Pos2i(window_width,150), 'AI Controls' )
    AiWindow.addChild(botTypeBox)
    AiWindow.addChild(numBotBox)
    AiWindow.addChild(changeBotButton)
    AiWindow.addChild(addBotButton)
    AiWindow.addChild(pauseButton)
    AiWindow.addChild(removeBotButton)
    AiWindow.addChild(exitButton)

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
