from OpenNero import *
from nero_mod import list_mods
from inputConfig import *

from common import gui, getGuiManager, openWiki

mods = list_mods()

def SwitchToSelectedMod(combo_box):
    def closure():
        i = combo_box.getSelected()
        modname = mods[i]
        modpath = modname + ":common"
        switchMod(modname, modpath)
    return closure

def ClientMain():
    # set up the gui
    guiMan = getGuiManager()
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    neroImg = gui.create_image( guiMan, 'nero_background', Pos2i(0,0), Pos2i(800,600), 'data/gui/nero_background.png' )

    combo_box = gui.create_combo_box(guiMan, "mod_selector", Pos2i(400, 200), Pos2i(200, 25))
    for i, modname in enumerate(mods):
        combo_box.addItem(modname)

    startButton = gui.create_button( guiMan, 'start', Pos2i(400,250), Pos2i(60,25), '' )
    startButton.text = 'Start'
    startButton.OnMouseLeftClick = SwitchToSelectedMod(combo_box)

    helpButton = gui.create_button( guiMan, 'help', Pos2i(470,250), Pos2i(60,25), '' )
    helpButton.text = 'Help'
    helpButton.OnMouseLeftClick = openWiki('RunningOpenNero')

    killButton = gui.create_button( guiMan, 'kill', Pos2i(540,250), Pos2i(60,25), '' )
    killButton.text = 'Exit'
    killButton.OnMouseLeftClick = lambda: getSimContext().killGame()
