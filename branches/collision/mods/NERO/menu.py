import wx
import os
import sys
from menu_utils import ScriptClient

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

        #Stand Ground Slider
        self.sgt = wx.StaticText(self,label = "Stand Ground", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.sgs = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.sgs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.sgs.SetThumbPosition(100)
        self.sgl = wx.StaticText(self,label = str(self.sgs.GetThumbPosition() - 100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.sgl, pos=(1,1))
        self.Bind(wx.EVT_SCROLL, self.OnSG,self.sgs)
        grid.Add(self.sgs, pos=(1,2))
        grid.Add(self.sgt, pos=(1,0))

        #Stick Together Slider
        self.stt = wx.StaticText(self,label = "Stick Together", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.sts = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.sts.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.sts.SetThumbPosition(100)
        self.stl = wx.StaticText(self,label = str(self.sts.GetThumbPosition()-100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.stl,pos=(2,1))
        self.Bind(wx.EVT_SCROLL, self.OnST,self.sts)
        grid.Add(self.sts,pos=(2,2))
        grid.Add(self.stt,pos=(2,0))

        #Together Distance Slider
        self.tdt = wx.StaticText(self,label = "Distance", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.tds = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.tds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.tds.SetThumbPosition(50)
        self.tdl = wx.StaticText(self,label = str(self.tds.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.tdl,pos=(2,5))
        self.Bind(wx.EVT_SCROLL, self.OnTD,self.tds)
        grid.Add(self.tds,pos=(2,6))
        grid.Add(self.tdt,pos=(2,4))

        #Approach Enemy Slider
        self.aet = wx.StaticText(self,label = "Approach Enemy", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.aes = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.aes.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.aes.SetThumbPosition(100)
        self.ael = wx.StaticText(self,label = str(self.aes.GetThumbPosition()-100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.ael,pos=(3,1))
        self.Bind(wx.EVT_SCROLL, self.OnAE,self.aes)
        grid.Add(self.aes,pos=(3,2))
        grid.Add(self.aet,pos=(3,0))

        #Enemy Distance Slider
        self.edt = wx.StaticText(self,label = "Distance", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.eds = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.eds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.eds.SetThumbPosition(50)
        self.edl = wx.StaticText(self,label = str(self.eds.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.edl,pos=(3,5))
        self.Bind(wx.EVT_SCROLL, self.OnED,self.eds)
        grid.Add(self.eds,pos=(3,6))
        grid.Add(self.edt,pos=(3,4))

        #Approach Flag Slider
        self.aft = wx.StaticText(self,label = "Approach Flag", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.afs = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.afs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.afs.SetThumbPosition(100)
        self.afl = wx.StaticText(self,label = str(self.afs.GetThumbPosition()-100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.afl,pos=(4,1))
        self.Bind(wx.EVT_SCROLL, self.OnAF,self.afs)
        grid.Add(self.afs,pos=(4,2))
        grid.Add(self.aft,pos=(4,0))

        #Flag Distance Slider
        self.fdt = wx.StaticText(self,label = "Distance", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.fds = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.fds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.fds.SetThumbPosition(50)
        self.fdl = wx.StaticText(self,label = str(self.fds.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.fdl,pos=(4,5))
        self.Bind(wx.EVT_SCROLL, self.OnFD,self.fds)
        grid.Add(self.fds,pos=(4,6))
        grid.Add(self.fdt,pos=(4,4))

        #Hit Target Slider
        self.htt = wx.StaticText(self,label = "Hit Target", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.hts = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.hts.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.hts.SetThumbPosition(100)
        self.htl = wx.StaticText(self,label = str(self.hts.GetThumbPosition()-100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.htl,pos=(5,1))
        self.Bind(wx.EVT_SCROLL, self.OnHT,self.hts)
        grid.Add(self.hts,pos=(5,2))
        grid.Add(self.htt,pos=(5,0))

        #Lifetime Slider
        self.ltt = wx.StaticText(self,label = "Lifetime", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.lts = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.lts.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.lts.SetThumbPosition(10)
        self.ltl = wx.StaticText(self,label = str(self.lts.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.ltl,pos=(6,1))
        self.Bind(wx.EVT_SCROLL, self.OnLT,self.lts)
        grid.Add(self.lts,pos=(6,2))
        grid.Add(self.ltt,pos=(6,0))

        #Friendly Fire Slider
        self.fft = wx.StaticText(self,label = "Friendly Fire", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.ffs = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.ffs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.ffs.SetThumbPosition(100)
        self.ffl = wx.StaticText(self,label = str(self.sgs.GetThumbPosition()-100), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.ffl,pos=(7,1))
        self.Bind(wx.EVT_SCROLL, self.OnFF,self.ffs)
        grid.Add(self.ffs,pos=(7,2))
        grid.Add(self.fft,pos=(7,0))

        #Explore/Exploit Slider
        self.eet = wx.StaticText(self,label = "Explore/Exploit", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.ees = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.ees.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.ees.SetThumbPosition(50)
        self.eel = wx.StaticText(self,label = str(self.ees.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.eel,pos=(8,1))
        self.Bind(wx.EVT_SCROLL, self.OnEE,self.ees)
        grid.Add(self.ees,pos=(8,2))
        grid.Add(self.eet,pos=(8,0))

        #Hitpoint Slider
        self.hpt = wx.StaticText(self,label = "Hitpoints", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.hps = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.hps.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.hps.SetThumbPosition(5)
        self.hpl = wx.StaticText(self,label = str(self.hps.GetThumbPosition()), pos=wx.DefaultPosition, size=wx.DefaultSize)
        grid.Add(self.hpl,pos=(9,1))
        self.Bind(wx.EVT_SCROLL, self.OnHP,self.hps)
        grid.Add(self.hps,pos=(9,2))
        grid.Add(self.hpt,pos=(9,0))

        #Speedup Slider
        self.spt = wx.StaticText(self,label = "Speedup", pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.sps = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200,15))
        self.sps.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.sps.SetThumbPosition(5)
        self.spl = wx.StaticText(self,label = str(self.sps.GetThumbPosition()), size=wx.DefaultSize)
        grid.Add(self.spl,pos=(10,1))
        self.Bind(wx.EVT_SCROLL, self.OnSP,self.sps)
        grid.Add(self.sps,pos=(10,2))
        grid.Add(self.spt,pos=(10,0))

        self.SetSizer(grid)
        grid.Fit(parent)

    def OnDeploy(self,event):
        self.send("deploy 0")

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

    def OnSG(self,event):
        self.sgl.SetLabel(str(event.Position - 100))
        self.send("SG %d" % event.Position)

    def OnST(self,event):
        self.stl.SetLabel(str(event.Position - 100))
        self.send("ST %d" % event.Position)

    def OnTD(self,event):
        self.tdl.SetLabel(str(event.Position))
        self.send("TD %d" % event.Position)

    def OnAE(self,event):
        self.ael.SetLabel(str(event.Position - 100))
        self.send("AE %d" % event.Position)

    def OnED(self,event):
        self.edl.SetLabel(str(event.Position))
        self.send("ED %d" % event.Position)

    def OnAF(self,event):
        self.afl.SetLabel(str(event.Position - 100))
        self.send("AF %d" % event.Position)

    def OnFD(self,event):
        self.fdl.SetLabel(str(event.Position))
        self.send("FD %d" % event.Position)

    def OnHT(self,event):
        self.htl.SetLabel(str(event.Position - 100))
        self.send("HT %d" % event.Position)

    def OnLT(self,event):
        self.ltl.SetLabel(str(event.Position))
        self.send("LT %d" % event.Position)

    def OnFF(self,event):
        self.ffl.SetLabel(str(event.Position))
        self.send("FF %d" % event.Position)

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
frame = wx.Frame(None,title = "NERO Controls",size=(600,250))
panel = NeroPanel(frame)
frame.Show()
app.MainLoop()
