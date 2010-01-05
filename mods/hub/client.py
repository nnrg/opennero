from OpenNero import *
from nero_mod import list_mods
from inputConfig import *

from common import gui, getGuiManager, openWiki

class SwitchMod:
    def __init__(self, mod, path):
        self.mod = mod
        self.path = path
    def __call__(self):
        switchMod(self.mod, self.path)

def ClientMain():
    # set up the gui
    guiMan = getGuiManager()
    guiMan.setTransparency(1.0)
    guiMan.setFont("data/gui/fonthaettenschweiler.bmp")

    neroImg = gui.create_image( guiMan, 'nero_background', Pos2i(0,0), Pos2i(0,0), 'data/gui/nero_background.jpg' )

    mods = list_mods()
    for i, modname in enumerate(mods):
        button = gui.create_button(guiMan, modname, Pos2i(325, 100 + i * 100), Pos2i(100, 50), '')
        button.text = modname
        print 'Adding module: ' + modname
        button.OnMouseLeftClick = SwitchMod(modname, modname + ':common')

    killButton = gui.create_button( guiMan, 'kill', Pos2i(125,100), Pos2i(100,50), '' )
    killButton.text = 'Exit'
    killButton.OnMouseLeftClick = lambda: getSimContext().killGame()

    helpButton = gui.create_button( guiMan, 'help', Pos2i(125,175), Pos2i(100,50), '' )
    helpButton.text = 'Help'
    helpButton.OnMouseLeftClick = openWiki('RunningOpenNero')
