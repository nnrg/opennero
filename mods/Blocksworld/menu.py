import wx
import os
import sys
from menu_utils import ScriptClient

class BlocksworldPanel(wx.Panel, ScriptClient):
    def __init__(self,parent):
        wx.Panel.__init__(self,parent)
        ScriptClient.__init__(self)
        
        grid = wx.GridBagSizer(hgap = 5, vgap = 5)

        # Panel for the buttons
        self.buttonPanel = wx.Panel(self, wx.ID_ANY)
        buttonGrid = wx.GridBagSizer(hgap = 5, vgap = 5)

        #Deploy Button
        self.deploy = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize,label='Deploy')
        buttonGrid.Add(self.deploy, pos = (0,0) )
        self.Bind(wx.EVT_BUTTON, self.OnDeploy, self.deploy)

        #Save1 Button
        self.save = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Save Pop 1')
        buttonGrid.Add(self.save,pos=(0,1) )
        self.Bind(wx.EVT_BUTTON, self.OnSave1, self.save)
        
        #Load1 Button
        self.load = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Load Pop 1')
        buttonGrid.Add(self.load, pos = (0,2) )
        self.Bind(wx.EVT_BUTTON, self.OnLoad1, self.load)

        #Save2 Button
        self.save2 = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Save Pop 2')
        buttonGrid.Add(self.save2,pos=(0,3) )
        self.Bind(wx.EVT_BUTTON, self.OnSave2, self.save2)
        
        #Load2 Button
        self.load2 = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Load Pop 2')
        buttonGrid.Add(self.load2, pos = (0,4) )
        self.Bind(wx.EVT_BUTTON, self.OnLoad2, self.load2)
        
        self.buttonPanel.SetSizer(buttonGrid)

        grid.Add(self.buttonPanel, pos=(0,0), span=(1,6))

        #Team 1 - Team/Individual Fitness Slifer
        self.f1t = wx.StaticText(self,label = "Team 1 - Team/Individual", pos = wx.DefaultPosition, size=wx.DefaultSize)
        self.f1s = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.f1s.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.f1s.SetThumbPosition(5)
        self.f1l = wx.StaticText(self,label = str(self.f1s.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.f1l,pos=(2,1))
        self.Bind(wx.EVT_SCROLL, self.OnF1, self.f1s)
        grid.Add(self.f1s,pos=(2,2))
        grid.Add(self.f1t,pos=(2,0))

        #Team 2 - Team/Individual Fitness Slifer
        self.f2t = wx.StaticText(self,label = "Team 2 - Team/Individual", pos = wx.DefaultPosition, size=wx.DefaultSize)
        self.f2s = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.f2s.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.f2s.SetThumbPosition(5)
        self.f2l = wx.StaticText(self,label = str(self.f2s.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.f2l,pos=(3,1))
        self.Bind(wx.EVT_SCROLL, self.OnF2, self.f2s)
        grid.Add(self.f2s,pos=(3,2))
        grid.Add(self.f2t,pos=(3,0))

        #Team 1 - Offense/Defense Capturing Slider for THEIR COINS
        self.c1t = wx.StaticText(self,label = "Team 1 -cap- coins O/D", pos = wx.DefaultPosition, size=wx.DefaultSize)
        self.c1s = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.c1s.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.c1s.SetThumbPosition(5)
        self.c1l = wx.StaticText(self,label = str(self.c1s.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.c1l,pos=(4,1))
        self.Bind(wx.EVT_SCROLL, self.OnC1, self.c1s)
        grid.Add(self.c1s,pos=(4,2))
        grid.Add(self.c1t,pos=(4,0))
        
        #Team 2 - Offense/Defense Captureing Silder for THEIR COINS
        self.c2t = wx.StaticText(self,label = "Team 2 -cap- coins O/D", pos = wx.DefaultPosition, size=wx.DefaultSize)
        self.c2s = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.c2s.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.c2s.SetThumbPosition(5)
        self.c2l = wx.StaticText(self,label = str(self.c2s.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.c2l,pos=(5,1))
        self.Bind(wx.EVT_SCROLL, self.OnC2, self.c2s)
        grid.Add(self.c2s,pos=(5,2))
        grid.Add(self.c2t,pos=(5,0))


        #Hitpoint Slider
        self.hpt = wx.StaticText(self,label = "Hitpoints", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.hps = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.hps.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.hps.SetThumbPosition(5)
        self.hpl = wx.StaticText(self,label = str(self.hps.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.hpl,pos=(6,1))
        self.Bind(wx.EVT_SCROLL, self.OnHP,self.hps)
        grid.Add(self.hps,pos=(6,2))
        grid.Add(self.hpt,pos=(6,0))

        #Speedup Slider
        self.spt = wx.StaticText(self,label = "Speedup", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.sps = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.sps.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.sps.SetThumbPosition(5)
        self.spl = wx.StaticText(self,label = str(self.sps.GetThumbPosition()), size=wx.DefaultSize)
        grid.Add(self.spl,pos=(7,1))
        self.Bind(wx.EVT_SCROLL, self.OnSP,self.sps)
        grid.Add(self.sps,pos=(7,2))
        grid.Add(self.spt,pos=(7,0))

        self.SetSizer(grid)
        grid.Fit(parent)

    def OnDeploy(self,event):
        self.send("deploy")

    def OnSave1(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Save Population File", dirname, "", "*.*", wx.FD_SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("save1 %s" % dirname)
    
    def OnLoad1(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load1 %s" % dirname)

    def OnSave2(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Save Population File", dirname, "", "*.*", wx.FD_SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("save2 %s" % dirname)
    
    def OnLoad2(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load2 %s" % dirname)

    def OnF1(self,event):
        self.f1l.SetLabel(str(event.Position))
        self.send("F1 %d" % event.Position)

    def OnF2(self,event):
        self.f2l.SetLabel(str(event.Position))
        self.send("F2 %d" % event.Position)
    
    def OnC1(self,event):
        self.c1l.SetLabel(str(event.Position))
        self.send("C1 %d" % event.Position)

    def OnC2(self,event):
        self.c2l.SetLabel(str(event.Position))
        self.send("C2 %d" % event.Position)
    
    def OnEE(self,event):
        self.eel.SetLabel(str(event.Position))
        self.send("EE %d" % event.Position)

    def OnHP(self,event):
        self.hpl.SetLabel(str(event.Position))
        self.send("HP %d" % event.Position)

    def OnSP(self,event):
        self.spl.SetLabel(str(event.Position))
        self.send("SP %d" % event.Position)

app = wx.App(False)
frame = wx.Frame(None,title = "Blocksworld Controls",size=(600,250))
panel = BlocksworldPanel(frame)
frame.Show()
app.MainLoop()
