#!/usr/bin/env pythonw
"""
Plot the performance of AI algorithms in OpenNERO.

The program can plot either a saved log file or a live session streaming
over the network. To open a file, specify it as the command line argument.
Otherwise, the program will start in server mode and listen for a 
connection from plot_client.
"""

import sys
import os
import re
import time
import threading
import SocketServer

# we are going to use matplotlib within a WX application
import wx
import matplotlib
matplotlib.use('WXAgg')
import matplotlib.pyplot as pl
import matplotlib.mlab as mlab
import numpy as np
import pylab
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar

__author__ = "Igor Karpov (ikarpov@cs.utexas.edu)"

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4086
SAVE_EVERY = 5

ai_tick_pattern = re.compile(r'(?P<date>[^\[]*)\.(?P<msec>[0-9]+) \(m\) \[ai\.tick\]\s+(?P<id>\S+)\s+(?P<episode>\S+)\s+(?P<step>\S+)\s+(?P<reward>\S+)\s+(?P<fitness>\S+)')
timestamp_fmt = r'%Y-%b-%d %H:%M:%S'
file_timestamp_fmt = r'%Y-%m-%d-%H-%M-%S'

def timestamped_filename(prefix = '', postfix = ''):
    return '%s%s%s' % (prefix, time.strftime(file_timestamp_fmt), postfix)

class XYData:
    """ 2-D data with min and max values """

    def __init__(self):
        self.xmin = None
        self.xmax = None
        self.ymin = None
        self.ymax = None
        self.x = []
        self.y = []
        self.handle = None # plot handle

    def append(self,x,y):
        self.xmin = min(self.xmin, x) if self.xmin else x
        self.xmax = max(self.xmax, x) if self.ymax else x
        self.ymin = min(self.ymin, y) if self.ymin else y
        self.ymax = max(self.ymax, y) if self.ymax else y
        self.x.append(x)
        self.y.append(y)

    def __len__(self):
        return len(self.x)

class AgentHistory:
    """ The history of a particular agent """

    def __init__(self):
        self.episode_fitness = XYData()
        self.fitness = XYData()

    def append(self, ms, fitness):
        self.episode_fitness.append(ms, fitness)

    def episode(self):        
        #if len(self.episode_fitness) > 0:
        #    self.fitness.append(self.episode_fitness.x[-1], self.episode_fitness.y[-1])
        #self.episode_fitness = XYData()
        pass

    def plot(self, axes):
        # plot the within-episode reward
        t = np.array(self.episode_fitness.x)
        f = np.array(self.episode_fitness.y)
        if self.episode_fitness.handle:
            self.episode_fitness.handle.set_xdata(t)
            self.episode_fitness.handle.set_ydata(f)
        else:
            self.episode_fitness.handle = axes.plot(t, f, linewidth=1, color=(1, 1, 0))[0]
        # plot the between-episodes fitness
        #t = np.array(self.fitness.x)
        #f = np.array(self.fitness.y)
        #if self.fitness.handle:
        #    self.fitness.handle.set_xdata(t)
        #    self.fitness.handle.set_ydata(f)
        #else:
        #    self.fitness.handle = axes.plot(t, f, linewidth=1, color=(1, 1, 0))[0]

class LearningCurve:
    """ The learning curve of a group of agents """

    def __init__(self):
        self.histories = {}
        self.total = XYData()

    def append(self, id, ms, episode, step, reward, fitness):
        record = None
        if id in self.histories:
            record = self.histories[id]
        else:
            record = AgentHistory()
            self.histories[id] = record
        if step == 0:
            record.episode()
        record.append(ms, fitness)
        self.total.append(ms, fitness)

    def plot(self, axes):
        axes.hold(False)
        t = np.array(self.total.x)
        f = np.array(self.total.y)
        for id in self.histories:
            self.histories[id].plot(axes)
            axes.hold(True)

    def process_line(self, line):
        """
        Process a line of the log file and record the information in it in the LearningCurve
        """
        line = line.strip().lower()
        m = ai_tick_pattern.search(line)
        if m:
            id = int(m.group('id')) # id of the agent
            t = time.strptime(m.group('date'), timestamp_fmt) # time of the record
            ms = int(m.group('msec')) # milliseconds
            episode = int(m.group('episode'))
            step = int(m.group('step'))
            reward = float(m.group('reward'))
            fitness = float(m.group('fitness'))
            ms += time.mktime(t) * 1000000
            self.append( id, ms, episode, step, reward, fitness )

    def process_file(self, f):
        line = f.readline()
        while line:
            self.process_line(line.strip())
            line = f.readline()

class GraphFrame(wx.Frame):
    """ The main frame of the application
    """
    title = 'OpenNERO performance plot'
    
    def __init__(self, learning_curve):
        wx.Frame.__init__(self, None, -1, self.title)

        self.learning_curve = learning_curve        
        self.paused = False

        self.create_menu()
        self.create_main_panel()
        self.create_status_bar()
        
        self.redraw_timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
        self.redraw_timer.Start(100)

    def create_menu(self):
        self.menubar = wx.MenuBar()        
        menu_file = wx.Menu()
        m_save = menu_file.Append(-1, "&Save plot\tCtrl-S", "Save plot to file")
        self.Bind(wx.EVT_MENU, self.on_save_plot, m_save)
        menu_file.AppendSeparator()
        m_quit = menu_file.Append(-1, "Q&uit\tCtrl-Q", "Quit")
        self.Bind(wx.EVT_MENU, self.on_exit, m_quit)                
        self.menubar.Append(menu_file, "&File")
        self.SetMenuBar(self.menubar)

    def create_main_panel(self):
        self.panel = wx.Panel(self)
        
        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)
        
        self.pause_button = wx.Button(self.panel, -1, "Pause")
        self.Bind(wx.EVT_BUTTON, self.on_pause_button, self.pause_button)
        self.Bind(wx.EVT_UPDATE_UI, self.on_update_pause_button, self.pause_button)
        
        self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox1.Add(self.pause_button, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        
        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)        
        self.vbox.Add(self.hbox1, 0, flag=wx.ALIGN_LEFT | wx.TOP)
        
        self.panel.SetSizer(self.vbox)
        self.vbox.Fit(self)

    def create_status_bar(self):
        self.statusbar = self.CreateStatusBar()

    def init_plot(self):
        self.dpi = 100
        self.fig = Figure((3.0, 3.0), dpi=self.dpi)

        self.axes = self.fig.add_subplot(111)
        self.axes.set_axis_bgcolor('black')
        self.axes.set_title('Agent fitness over time', size=12)
        
        pylab.setp(self.axes.get_xticklabels(), fontsize=8)
        pylab.setp(self.axes.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        self.learning_curve.plot(self.axes)

    def draw_plot(self):
        """ Redraws the plot
        """
        # bounds on x and y
        xmin = self.learning_curve.total.xmin if self.learning_curve.total.xmin else 0
        xmax = self.learning_curve.total.xmax if self.learning_curve.total.xmax else 50
        ymax = self.learning_curve.total.ymax if self.learning_curve.total.ymax else 1
        ymin = self.learning_curve.total.ymin if self.learning_curve.total.ymin else 0
        
        self.axes.set_xbound(lower=xmin, upper=xmax)
        self.axes.set_ybound(lower=ymin, upper=ymax)
        
        self.axes.grid(True, color='gray')
        
        pylab.setp(self.axes.get_xticklabels(), visible=True)
        
        self.learning_curve.plot(self.axes)
        
        self.canvas.draw()
    
    def on_pause_button(self, event):
        self.paused = not self.paused
    
    def on_update_pause_button(self, event):
        label = "Resume" if self.paused else "Pause"
        self.pause_button.SetLabel(label)
    
    def on_cb_grid(self, event):
        self.draw_plot()
    
    def on_cb_xlab(self, event):
        self.draw_plot()
    
    def on_save_plot(self, event):
        file_choices = "PNG (*.png)|*.png"
        dlg = wx.FileDialog(
            self, 
            message="Save plot as...",
            defaultDir=os.getcwd(),
            defaultFile="plot.png",
            wildcard=file_choices,
            style=wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.canvas.print_figure(path, dpi=self.dpi)
            self.flash_status_message("Saved to %s" % path)
    
    def on_redraw_timer(self, event):
        # if paused do not add data, but still redraw the plot
        # (to respond to scale modifications, grid change, etc.)
        #if not self.paused:
        #    self.data.append(self.source.next())
        self.draw_plot()
    
    def on_exit(self, event):
        self.Destroy()
    
    def flash_status_message(self, msg, flash_len_ms=1500):
        self.statusbar.SetStatusText(msg)
        self.timeroff = wx.Timer(self)
        self.Bind(
            wx.EVT_TIMER, 
            self.on_flash_status_off, 
            self.timeroff)
        self.timeroff.Start(flash_len_ms, oneShot=True)
    
    def on_flash_status_off(self, event):
        self.statusbar.SetStatusText('')

app = None

def start_server():
    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer(ADDR, PlotTCPHandler)
    # Start a thread with the server -- that thread will then start one
    # more thread for each request
    server_thread = threading.Thread(target=server.serve_forever)
    # Exit the server thread when the main thread terminates
    server_thread.setDaemon(True)
    server_thread.start()

class PlotTCPHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        global app
        lc = LearningCurve()
        if app:
            app.frame.learning_curve = lc
        lc.process_file(self.rfile)

def main():
    global app
    if len(sys.argv) > 1:
        print 'opening OpenNERO log file', sys.argv[1]
        f = open(sys.argv[1])
        lc = LearningCurve()
        lc.process_file(f)
        f.close()
        app = wx.PySimpleApp()
        app.frame = GraphFrame(lc)
        app.frame.Show()
        app.MainLoop()
    else:
        lc = LearningCurve()
        app = wx.PySimpleApp()
        app.frame = GraphFrame(lc)
        start_server()
        app.frame.Show()
        print 'Listening on ', ADDR
        app.MainLoop()
    print 'done'

if __name__ == "__main__":
    main()
