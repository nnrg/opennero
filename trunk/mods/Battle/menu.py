import wx
import os
import sys
from ..common.menu import ScriptClient

class NeroPanel(wx.Panel, ScriptClient):
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
        
        #Load Pop 1 Button
        self.load1= wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Load Pop 1')
        buttonGrid.Add(self.load1,pos=(0,1) )
        self.Bind(wx.EVT_BUTTON, self.OnLoad1, self.load1)
        
        #Load Pop 2 Button
        self.load2 = wx.Button(self.buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label='Load Pop 2')
        buttonGrid.Add(self.load2,pos=(0,2) )
        self.Bind(wx.EVT_BUTTON, self.OnLoad2, self.load2)
        
        self.buttonPanel.SetSizer(buttonGrid)
        
        grid.Add(self.buttonPanel, pos=(0,0), span=(1,6))
        
        #Explore/Exploit Slider
        self.eet = wx.StaticText(self,label = "Explore/Exploit", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.ees = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.ees.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.ees.SetThumbPosition(50)
        self.eel = wx.StaticText(self,label = str(self.ees.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.eel,pos=(1,1))
        self.Bind(wx.EVT_SCROLL, self.OnEE,self.ees)
        grid.Add(self.ees,pos=(1,2))
        grid.Add(self.eet,pos=(1,0))

        #Speedup Slider
        self.spt = wx.StaticText(self,label = "Speedup", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.sps = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.sps.SetScrollbar(wx.HORIZONTAL,0,50,100)
        self.sps.SetThumbPosition(5)
        self.spl = wx.StaticText(self,label = str(self.sps.GetThumbPosition()), size=wx.DefaultSize)
        grid.Add(self.spl,pos=(2,1))
        self.Bind(wx.EVT_SCROLL, self.OnSP,self.sps)
        grid.Add(self.sps,pos=(2,2))
        grid.Add(self.spt,pos=(2,0))

        self.SetSizer(grid)
        grid.Fit(parent)

    def OnDeploy(self,event):
        self.send("deploy")

    def OnSave(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Save Population File", dirname, "", "*.*", wx.FD_SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("save %s" % dirname)
    
    def OnLoad1(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load1 %s" % dirname)

    def OnLoad2(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load2 %s" % dirname)
    
    def OnEE(self,event):
        self.eel.SetLabel(str(event.Position))
        self.send("EE %d" % event.Position)
    
    def OnSP(self,event):
        self.spl.SetLabel(str(event.Position))
        self.send("SP %d" % event.Position)

app = wx.App(False)
frame = wx.Frame(None,title = "NERO Controls",size=(600,250))
panel = NeroPanel(frame)
frame.Show()
app.MainLoop()
