import wx
import os
import sys
import subprocess

WIDTH = 510
HEIGHT = 600

class NeroPanel(wx.Panel):
    def __init__(self,parent):
        wx.Panel.__init__(self,parent)
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        grid= wx.GridBagSizer(hgap=5,vgap=5)
        hSizer = wx.BoxSizer(wx.HORIZONTAL)

        #Deploy Button
        self.deploy = wx.Button(self,size=(70,100),label='Deploy')
        grid.Add(self.deploy,pos = (0,0))
        self.Bind(wx.EVT_BUTTON, self.OnDeploy, self.deploy)

        #Save Button
        self.save = wx.Button(self,size=(70,30),pos = (0,105), label='Save')
        grid.Add(self.save,pos = (50,0))
        self.Bind(wx.EVT_BUTTON, self.OnSave, self.save)

        #Load Button
        self.load = wx.Button(self,size=(70,30),pos = (0,140),label='Load')
        grid.Add(self.load,pos = (0,0))
        self.Bind(wx.EVT_BUTTON, self.OnLoad, self.load)

        #Stand Ground Slider
        self.sgt = wx.StaticText(self,label = "Stand Ground", pos = (100,20), size = (120,20))
        self.sgs = wx.ScrollBar(self, pos = (220,18), size = (100,20))
        self.sgs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.sgs.SetThumbPosition(100)
        self.sgl = wx.StaticText(self,label = str(self.sgs.GetThumbPosition() - 100), pos = (320,20), size = (50,20))
        grid.Add(self.sgl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnSG,self.sgs)
        grid.Add(self.sgs,pos=(0,0),span=(1,2))
        grid.Add(self.sgt,pos=(0,0),span=(1,2))

        #Stick Together Slider
        self.stt = wx.StaticText(self,label = "Stick Together", pos = (100,40), size = (120,20))
        self.sts = wx.ScrollBar(self, pos = (220,38), size = (100,20))
        self.sts.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.sts.SetThumbPosition(100)
        self.stl = wx.StaticText(self,label = str(self.sts.GetThumbPosition()-100), pos = (320,40), size = (50,20))
        grid.Add(self.stl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnST,self.sts)
        grid.Add(self.sts,pos=(0,0),span=(1,2))
        grid.Add(self.stt,pos=(0,0),span=(1,2))

        #Together Distance Slider
        self.tdt = wx.StaticText(self,label = "Distance", pos = (380,40), size = (120,20))
        self.tds = wx.ScrollBar(self, pos = (440,38), size = (100,20))
        self.tds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.tds.SetThumbPosition(50)
        self.tdl = wx.StaticText(self,label = str(self.tds.GetThumbPosition()), pos = (540,40), size = (50,20))
        grid.Add(self.tdl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnTD,self.tds)
        grid.Add(self.tds,pos=(0,0),span=(1,2))
        grid.Add(self.tdt,pos=(0,0),span=(1,2))

        #Approach Enemy Slider
        self.aet = wx.StaticText(self,label = "Approach Enemy", pos = (100,60), size = (120,20))
        self.aes = wx.ScrollBar(self, pos = (220,58), size = (100,20))
        self.aes.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.aes.SetThumbPosition(100)
        self.ael = wx.StaticText(self,label = str(self.aes.GetThumbPosition()-100), pos = (320,60), size = (50,20))
        grid.Add(self.ael,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnAE,self.aes)
        grid.Add(self.aes,pos=(0,0),span=(1,2))
        grid.Add(self.aet,pos=(0,0),span=(1,2))

        #Enemy Distance Slider
        self.edt = wx.StaticText(self,label = "Distance", pos = (380,60), size = (120,20))
        self.eds = wx.ScrollBar(self, pos = (440,58), size = (100,20))
        self.eds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.eds.SetThumbPosition(50)
        self.edl = wx.StaticText(self,label = str(self.eds.GetThumbPosition()), pos = (540,60), size = (50,20))
        grid.Add(self.edl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnED,self.eds)
        grid.Add(self.eds,pos=(0,0),span=(1,2))
        grid.Add(self.edt,pos=(0,0),span=(1,2))

        #Approach Flag Slider
        self.aft = wx.StaticText(self,label = "Approach Flag", pos = (100,80), size = (100,20))
        self.afs = wx.ScrollBar(self, pos = (220,78), size = (100,20))
        self.afs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.afs.SetThumbPosition(100)
        self.afl = wx.StaticText(self,label = str(self.afs.GetThumbPosition()-100), pos = (320,80), size = (50,20))
        grid.Add(self.afl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnAF,self.afs)
        grid.Add(self.afs,pos=(0,0),span=(1,2))
        grid.Add(self.aft,pos=(0,0),span=(1,2))

        #Flag Distance Slider
        self.fdt = wx.StaticText(self,label = "Distance", pos = (380,80), size = (120,20))
        self.fds = wx.ScrollBar(self, pos = (440,78), size = (100,20))
        self.fds.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.fds.SetThumbPosition(50)
        self.fdl = wx.StaticText(self,label = str(self.fds.GetThumbPosition()), pos = (540,80), size = (50,20))
        grid.Add(self.fdl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnFD,self.fds)
        grid.Add(self.fds,pos=(0,0),span=(1,2))
        grid.Add(self.fdt,pos=(0,0),span=(1,2))

        #Hit Target Slider
        self.htt = wx.StaticText(self,label = "Hit Target", pos = (100,100), size = (100,20))
        self.hts = wx.ScrollBar(self, pos = (220,98), size = (100,20))
        self.hts.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.hts.SetThumbPosition(100)
        self.htl = wx.StaticText(self,label = str(self.hts.GetThumbPosition()-100), pos = (320,100), size = (50,20))
        grid.Add(self.htl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnHT,self.hts)
        grid.Add(self.hts,pos=(0,0),span=(1,2))
        grid.Add(self.htt,pos=(0,0),span=(1,2))

        #Lifetime Slider
        self.ltt = wx.StaticText(self,label = "Lifetime", pos = (100,140), size = (100,20))
        self.lts = wx.ScrollBar(self, pos = (220,138), size = (100,20))
        self.lts.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.lts.SetThumbPosition(10)
        self.ltl = wx.StaticText(self,label = str(self.lts.GetThumbPosition()), pos = (320,140), size = (50,20))
        grid.Add(self.ltl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnLT,self.lts)
        grid.Add(self.lts,pos=(0,0),span=(1,2))
        grid.Add(self.ltt,pos=(0,0),span=(1,2))

        #Friendly Fire Slider
        self.fft = wx.StaticText(self,label = "Friendly Fire", pos = (100,160), size = (100,20))
        self.ffs = wx.ScrollBar(self, pos = (220,158), size = (100,20))
        self.ffs.SetScrollbar(wx.HORIZONTAL,0,200,200)
        self.ffs.SetThumbPosition(100)
        self.ffl = wx.StaticText(self,label = str(self.sgs.GetThumbPosition()-100), pos = (320,160), size = (50,20))
        grid.Add(self.ffl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnFF,self.ffs)
        grid.Add(self.ffs,pos=(0,0),span=(1,2))
        grid.Add(self.fft,pos=(0,0),span=(1,2))

        #Explore/Exploit Slider
        self.eet = wx.StaticText(self,label = "Explore/Exploit", pos = (100,180), size = (100,20))
        self.ees = wx.ScrollBar(self, pos = (220,178), size = (100,20))
        self.ees.SetScrollbar(wx.HORIZONTAL,0,100,100)
        self.ees.SetThumbPosition(50)
        self.eel = wx.StaticText(self,label = str(self.ees.GetThumbPosition()), pos = (320,180), size = (50,20))
        grid.Add(self.eel,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnEE,self.ees)
        grid.Add(self.ees,pos=(0,0),span=(1,2))
        grid.Add(self.eet,pos=(0,0),span=(1,2))

        #Hitpoint Slider
        self.hpt = wx.StaticText(self,label = "Hitpoints", pos = (100,200), size = (100,20))
        self.hps = wx.ScrollBar(self, pos = (220,198), size = (100,20))
        self.hps.SetScrollbar(wx.HORIZONTAL,0,50,100)
        self.hps.SetThumbPosition(5)
        self.hpl = wx.StaticText(self,label = str(self.hps.GetThumbPosition()), pos = (320,200), size = (50,20))
        grid.Add(self.hpl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnHP,self.hps)
        grid.Add(self.hps,pos=(0,0),span=(1,2))
        grid.Add(self.hpt,pos=(0,0),span=(1,2))

        #Speedup Slider
        self.spt = wx.StaticText(self,label = "Speedup", pos = (100,220), size = (100,20))
        self.sps = wx.ScrollBar(self, pos = (220,218), size = (100,20))
        self.sps.SetScrollbar(wx.HORIZONTAL,0,50,100)
        self.sps.SetThumbPosition(5)
        self.spl = wx.StaticText(self,label = str(self.sps.GetThumbPosition()), pos = (320,220), size = (50,20))
        grid.Add(self.spl,pos=(0,0),span=(1,2))
        self.Bind(wx.EVT_SCROLL, self.OnSP,self.sps)
        grid.Add(self.sps,pos=(0,0),span=(1,2))
        grid.Add(self.spt,pos=(0,0),span=(1,2))

    def OnDeploy(self,event):
        print "Deploy"

    def OnSave(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Save Population File", dirname, "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            print "Save", dirname, filename
    
    def OnLoad(self,event):
        dirname = ""
        dlg = wx.FileDialog(self, "Save Population File", dirname, "", "*.*", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            print "Load", dirname, filename

    def OnSG(self,event):
        self.sgl.SetLabel(str(event.Position - 100))
        print "SG", event.Position
        sys.stdout.flush()

    def OnST(self,event):
        self.stl.SetLabel(str(event.Position - 100))
        print "ST", event.Position
        sys.stdout.flush()

    def OnTD(self,event):
        self.tdl.SetLabel(str(event.Position))
        print "TD", event.Position
        sys.stdout.flush()

    def OnAE(self,event):
        self.ael.SetLabel(str(event.Position - 100))
        print "AE", event.Position
        sys.stdout.flush()

    def OnED(self,event):
        self.edl.SetLabel(str(event.Position))
        print "ED", event.Position
        sys.stdout.flush()

    def OnAF(self,event):
        self.afl.SetLabel(str(event.Position - 100))
        print "AF", event.Position
        sys.stdout.flush()

    def OnFD(self,event):
        self.fdl.SetLabel(str(event.Position))
        print "FD", event.Position
        sys.stdout.flush()

    def OnHT(self,event):
        self.htl.SetLabel(str(event.Position - 100))
        print "HT", event.Position
        sys.stdout.flush()

    def OnLT(self,event):
        self.ltl.SetLabel(str(event.Position))
        print "LT", event.Position
        sys.stdout.flush()

    def OnFF(self,event):
        self.ffl.SetLabel(str(event.Position))
        print "FF", event.Position
        sys.stdout.flush()

    def OnEE(self,event):
        self.eel.SetLabel(str(event.Position))
        print "EE", event.Position
        sys.stdout.flush()

    def OnHP(self,event):
        self.hpl.SetLabel(str(event.Position))
        print "HP", event.Position
        sys.stdout.flush()

    def OnSP(self,event):
        self.spl.SetLabel(str(event.Position))
        print "SP", event.Position
        sys.stdout.flush()

read = None

def getReader():
    print "INITILIZING"
    global read
    global subp
    if not read:
        print "INITILIZING"
        subp = subprocess.Popen(['python', 'menu.py'],stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.PIPE)
        print "Ummm"
        read = subp.stdout
        print "STUFF"
    return read

app = wx.App(False)
frame = wx.Frame(None,title = "NERO Controls",size=(600,250))
panel = NeroPanel(frame)
frame.Show()
app.MainLoop()
