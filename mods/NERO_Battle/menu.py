import os
import sys

sys.path.append(os.path.join(sys.path[0], os.pardir))

import NERO.menu

try:
    import wx
except:
    import tkMessageBox
    tkMessageBox.showwarning('Warning!', 'Could not start the external menu for NERO because wxPython is not installed.')
    sys.exit()


class NeroPanel(NERO.menu.NeroPanel):
    def add_buttons(self):
        self.add_button('Deploy RTNEAT', self.OnDeployRTNEAT)
        self.add_button('Deploy QLearning', self.OnDeployQLearning)
        self.add_button('Load Blue Team', self.OnLoad1)
        self.add_button('Load Red Team', self.OnLoad2)

    def add_sliders(self):
        self.add_slider('Hitpoints', 'HP', span=100, center=0, thumb=20)
        self.add_slider('Speedup', 'SP', span=100, center=0, thumb=80)

    def OnLoad2(self, event):
        dirname = ""
        dlg = wx.FileDialog(self, "Red Team: Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load2 %s" % dirname)


if __name__ == '__main__':
    print 'creating NERO Battle controls'
    app = wx.App(False)
    frame = wx.Frame(None, title="NERO Battle Controls", size=(600, 250))
    panel = NeroPanel(frame)
    frame.Show()
    app.MainLoop()
