import cPickle
import select
import socket
import struct
import sys

import constants

try:
    import wx
except:
    import tkMessageBox
    tkMessageBox.showwarning('Warning!', 'Could not start the external menu for NERO because wxPython is not installed.')
    sys.exit()


marshall = cPickle.dumps
unmarshall = cPickle.loads

def send(channel, *args):
    buf = marshall(args)
    value = socket.htonl(len(buf))
    size = struct.pack("L", value)
    channel.send(size)
    channel.send(buf)

def receive(channel):
    size = struct.calcsize("L")
    size = channel.recv(size)
    try:
        size = socket.ntohl(struct.unpack("L", size)[0])
    except struct.error, e:
        return ''
    buf = ""
    while len(buf) < size:
        buf = channel.recv(size - len(buf))
    return unmarshall(buf)[0]


HOST = '127.0.0.1'
PORT = 8888

class ScriptClient:
    def __init__(self, host=HOST, port=PORT):
        self.host = host
        self.port = port
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print 'ScriptClient connected to ScriptServer at (%s, %d)' % (self.host, self.port)
        except socket.error, e:
            print 'ScriptClient could not connect to ScriptServer'

    def __del__(self):
        print 'ScriptClient closing socket...'
        self.sock.close()

    def send(self, data):
        if data and self.sock:
            send(self.sock, data)


class NeroPanel(wx.Panel, ScriptClient):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        ScriptClient.__init__(self)

        self._grid = wx.GridBagSizer(hgap=5, vgap=5)

        self._buttonIndex = 0
        self._buttonPanel = wx.Panel(self, wx.ID_ANY)
        self._buttonGrid = wx.GridBagSizer(hgap=5, vgap=5)
        self.add_buttons()
        self._buttonPanel.SetSizer(self._buttonGrid)

        self._grid.Add(self._buttonPanel, pos=(0, 0), span=(1, 6))

        self._labels = {}
        self._sliderIndex = 1
        self.add_sliders()
        self.SetSizer(self._grid)

        self._grid.Fit(parent)

    def add_buttons(self):
        self.add_button('Deploy RTNEAT', self.OnDeployRTNEAT)
        self.add_button('Deploy QLearning', self.OnDeployQLearning)
        self.add_button('Save Team', self.OnSave1)
        self.add_button('Load Team', self.OnLoad1)

    def add_sliders(self):
        self.add_slider('Stand Ground', 'SG')
        self.add_slider('Stick Together', 'ST')
        self.add_slider('Approach Enemy', 'AE')
        self.add_slider('Approach Flag', 'AF')
        self.add_slider('Hit Target', 'HT')
        self.add_slider('Avoid Fire', 'VF')
        self.add_slider('Exploit-Explore', 'EE', span=100, center=0, thumb=constants.DEFAULT_EE)
        self.add_slider('Lifetime', 'LT', span=constants.DEFAULT_LIFETIME*2, center=0, thumb=constants.DEFAULT_LIFETIME)
        self.add_slider('Hitpoints', 'HP', span=100, center=0, thumb=constants.DEFAULT_HITPOINTS)
        #speedup slider is not needed since we want to run at 100% speedup
        #self.add_slider('Speedup', 'SP', span=100, center=0, thumb=constants.DEFAULT_SPEEDUP)

    def add_button(self, label, callback):
        button = wx.Button(
            self._buttonPanel, pos=wx.DefaultPosition, size=wx.DefaultSize, label=label)
        self._buttonGrid.Add(button, pos=(0, self._buttonIndex))
        self.Bind(wx.EVT_BUTTON, callback, button)
        self._buttonIndex += 1

    def add_slider(self, label, key, span=200, center=100, thumb=100):
        explanation = wx.StaticText(
            self, label=label, pos=wx.DefaultPosition, size=wx.DefaultSize)

        slider = wx.ScrollBar(self, pos=wx.DefaultPosition, size=(200, 15))
        slider.SetScrollbar(wx.HORIZONTAL, 0, span, span)
        slider.SetThumbPosition(thumb)

        pos = slider.GetThumbPosition() - center
        self._labels[key] = wx.StaticText(
            self, label=str(pos), pos=wx.DefaultPosition, size=wx.DefaultSize)

        self.Bind(wx.EVT_SCROLL, lambda event: self.OnSlider(event, key, center), slider)

        self._grid.Add(self._labels[key], pos=(self._sliderIndex, 1))
        self._grid.Add(slider, pos=(self._sliderIndex, 2))
        self._grid.Add(explanation, pos=(self._sliderIndex, 0))

        self._sliderIndex += 1

    def OnDeployRTNEAT(self, event):
        self.send("rtneat 0")

    def OnDeployQLearning(self, event):
        self.send("qlearning 0")

    def OnSave1(self, event):
        dirname = ""
        dlg = wx.FileDialog(self, "Blue Team: Save Population File", dirname, "", "*.*", wx.FD_SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("save1 %s" % dirname)

    def OnLoad1(self, event):
        dirname = ""
        dlg = wx.FileDialog(self, "Blue Team: Load Population File", dirname, "", "*.*", wx.FD_OPEN)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetFilename()
            dirname = dlg.GetPath()
            self.send("load1 %s" % dirname)

    def OnSlider(self, event, key, center=0):
        position = event.Position - center
        self._labels[key].SetLabel(str(position))
        self.send("%s %d" % (key, event.Position))


if __name__ == '__main__':
    app = wx.App(False)
    frame = wx.Frame(None, title="NERO Controls", size=(600, 250))
    panel = NeroPanel(frame)
    frame.Show()
    app.MainLoop()
