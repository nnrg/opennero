import os
import sys

sys.path.append(os.path.join(sys.path[0], os.pardir))

import NERO.menu
import NERO.constants

try:
    import wx
except:
    import tkMessageBox
    tkMessageBox.showwarning('Warning!', 'Could not start the external menu for NERO because wxPython is not installed.')
    sys.exit()


class NeroPanel(NERO.menu.NeroPanel):
    def __init__(self, parent):
        NERO.menu.NeroPanel.__init__(self, parent)
        self._sliders['HP'].Enable()

    def add_buttons(self):
        self.add_button('Load Blue Team', self.OnLoad1)
        self.add_button('Load Red Team', self.OnLoad2)
        self.add_button('Continue', self.OnPause, disabled=True)
        self.add_button('Help', self.OnHelp)

    def add_sliders(self):
        self.add_slider('Hitpoints', 'HP', span=100, center=0, thumb=NERO.constants.DEFAULT_HITPOINTS)

    def OnLoad2(self, event):
        dirname = ""
        dlg = wx.FileDialog(self, "Red Team: Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load2 %s" % dirname)
            self.loaded2 = True
            if self.loaded1 and self.loaded2:
                self._buttons['OnPause'].Enable()

if __name__ == '__main__':
    print 'creating NERO Battle controls'
    app = wx.App(False)
    frame = wx.Frame(None, title="NERO Battle Controls", size=(600, 250))
    panel = NeroPanel(frame)
    frame.Show()
    app.MainLoop()
