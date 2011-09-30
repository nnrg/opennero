# maze observer for OpenNERO maze environment

from Tkinter import *
from mazer import Maze
import random
from math import *
import sys
from threading import Thread
import struct
from driver import *

# window parameters
WINDOW_WIDTH = 500
WINDOW_HEIGHT = 500

# default maze parameters
DX = 20
DY = 20
ROWS = 8
COLS = 8
rad = 0.25 * DX

# unique names of events sent over the wires and for the GUI
E_QUIT = '<<quit>>'        # exit
E_MAZE = '<<maze>>'        # init the whole maze
E_SEED = '<<seed>>'        # init the maze with default parameters by specifying the seed
E_POSE = '<<update-pose>>' # update agent pose
E_TEXT = '<<text>>'        # set the text value
E_TITLE = '<<title>>'      # set the title value

class MazeObserver(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.title = "[live]"
        self.text = None
        self.text_id = None
        self.maze = None
        self.seed = None
        self.width = WINDOW_WIDTH
        self.height = WINDOW_HEIGHT
        self.server = None

    def dimensions(self):
        return (self.width, self.height)

    def dxdy(self):
        """
        size of cell on the canvas
        """
        (w,h) = self.dimensions()
        dx,dy = float(w)/self.maze.rows, float(h)/self.maze.cols
        return (dx,dy)

    def rc2canvas(self,r,c):
        """
        center of the r,c cell on the canvas
        """
        (dx,dy) = self.dxdy()
        return ((r + 0.5) * dx, (c + 0.5) * dy)

    def xy2canvas(self,x,y):
        """
        scale maze xy to full canvas size
        """
        (dx,dy) = self.dxdy()
        return (x*dx/self.maze.dx - dx/2.0, y*dy/self.maze.dy - dy/2.0)

    def draw_wall(self,c,r,dc,dr):
        (dx,dy) = self.dxdy()
        (x0,y0) = self.rc2canvas(r,c) # center of square one
        (x1,y1) = self.rc2canvas(r,c) # center of square one
        if dr != 0:
            # vertical line between two x positions
            x0 += dx * 0.5 * dr
            x1 += dx * 0.5 * dr
            y0 -= dy * 0.5
            y1 += dy * 0.5
        if dc != 0:
            # horizontal line between two y positions
            x0 -= dx * 0.5
            x1 += dx * 0.5
            y0 += dy * 0.5 * dc
            y1 += dy * 0.5 * dc
        self.canvas.create_line(x0,y0,x1,y1,width=3)

    def draw_maze(self):
        if not self.maze:
            assert(self.seed)
            random.seed(self.seed)
            self.maze = Maze.generate(ROWS, COLS, DX, DY)
        self.canvas.delete(ALL)
        self.me = None
        self.me_dir = None

        (x,y) = self.rc2canvas(0,0) # start circle
        self.canvas.create_oval(x-rad, y-rad, x+rad, y+rad, fill='blue', width=0)

        (x,y) = self.rc2canvas(self.maze.rows - 1, self.maze.cols - 1) # finish circle
        self.canvas.create_oval(x-rad, y-rad, x+rad, y+rad, fill='red', width=0)

        for r in range(self.maze.rows):
            for c in range(self.maze.cols):
                for (dr,dc) in [(-1,0),(0,-1),(1,0),(0,1)]:
                    if self.maze.is_wall(r,c,dr,dc):
                        self.draw_wall(r, c, dr, dc)

    def draw_text(self):
        (dx,dy) = self.dxdy()
        if self.text_id is not None:
            self.canvas.delete(self.text_id)
            self.text_id = None
        if self.text:
            self.text_id = self.canvas.create_text(5, 5, width = dx * (1 + self.maze.rows) - 10, text = self.text, anchor = NW)

    def update_pose(self):
    # x and y are intentionally reversed to match with maze walls
        (t,y,x,th) = self.pose
        self.text = "%s t: %f x: %f y: %f, th: %f" % (self.title, t, y, x, th)
        self.draw_text()
        if self.me is not None:
            self.canvas.delete(self.me)
            self.me = None
        if self.me_dir is not None:
            self.canvas.delete(self.me_dir)
            self.me_dir = None
        (x,y) = self.xy2canvas(x,y)
        self.me = self.canvas.create_oval(x-rad/2.0,y-rad/2.0,x+rad/2.0,y+rad/2.0,fill='green',width=0)
        self.me_dir = self.canvas.create_line(x,y,x+rad*1.5*sin(th),y+rad*1.5*cos(th),width=3)

    def quit(self):
        self.stop_server()
        self.root.generate_event(E_QUIT)

    def stop_server(self):
        if self.server:
            self.server.stopped = True
            self.server = None

    def on_close(self):
        self.stop_server()
        self.root.destroy() # destroy Tk

    def set_seed(self, seed):
        print 'Seed: %d' % seed
        self.seed = seed
        self.maze = None
        self.root.event_generate(E_SEED)

    def set_maze(self, maze_data):
        print 'Maze received'
        self.maze = Maze.from_data(maze_data)
        print self.maze
        self.seed = None
        self.root.event_generate(E_MAZE)

    def set_text(self, text):
        print 'Text: %s' % text
        self.text = text
        self.root.event_generate(E_TEXT)

    def set_title(self, title):
        print 'Title: %s' % title
        self.title = title
        self.root.event_generate(E_TITLE)

    def set_pose(self, pose):
        self.pose = pose
        self.root.event_generate(E_POSE)

    def play_log(self, logfile):
        """ take a log file with poses and play it back """
        last_time = 0
        for line in logfile.xreadlines():
            line = line.strip()
            m = re_pose.match(line)
            if m:
                (t,x,y,z,th) = tuple([float(x) for x in m.group(1,2,3,4,5)])
                self.set_pose((t,x,y,radians(th)))
        print t, x, y, z, th
        if t > last_time:
            time.sleep(t - last_time)
        last_time = t
        m = re_seed.match(line)
        if m:
            seed = int(m.group(1))
            self.set_seed(seed)
            print 'seed:', seed

    def configure(self, event):
        self.width = event.width
        self.height = event.height
        self.draw_maze()

    def run(self):
        # gui
        self.root = Tk()
        # event bindings
        self.root.bind(E_QUIT, lambda event: self.quit())
        self.root.bind(E_POSE, lambda event: self.update_pose())
        self.root.bind(E_MAZE, lambda event: self.draw_maze())
        self.root.bind(E_SEED, lambda event: self.draw_maze())
        self.root.bind(E_TEXT, lambda event: self.draw_text())
        self.root.bind(E_TITLE, lambda event: self.root.title(self.title))
        # window properties
        self.root.title('OpenNERO maze observer')
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        # drawing canvas
        self.canvas = Canvas(self.root, width=WINDOW_WIDTH, height=WINDOW_HEIGHT)
        self.canvas.bind('<Configure>', lambda event: self.configure(event))
        self.canvas.pack(fill=BOTH, expand=1)
        self.me = None
        self.me_dir = None
        mainloop()

import SocketServer
import cPickle as pickle

class PoseHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        global observer,server
        start_time = time.time()
        while 1:
            # read a data length marker
            data = self.connection.recv(4)
            if len(data) < 4:
                break
            slen = struct.unpack(">L", data)[0]
            assert(slen < 1024) # small messages only!
            data = self.connection.recv(slen)
            while len(data) < slen:
                data += self.connection.recv(slen - len(data))
            obj = pickle.loads(data)
            if obj == E_QUIT:
                observer.stop_server()
                observer.root.event_generate(E_QUIT)
                break
            elif obj[0] == E_SEED:
                observer.set_seed(obj[1])
            elif obj[0] == E_MAZE:
                observer.set_maze(obj[1])
                start_time = time.time()
            elif obj[0] == E_TEXT:
                observer.set_text(obj[1])
            elif obj[0] == E_TITLE:
                observer.set_title(obj[1])
            else:
                (x,y,th) = obj
                observer.set_pose((time.time() - start_time, x, y, th))

class PoseServer(SocketServer.TCPServer):
    def serve_forever(self):
        self.stopped = False
        while not self.stopped:
            try:
                self.handle_request()
            except KeyboardInterrupt:
                self.stopped = True

HOST = '127.0.0.1'        # The remote host
PORT = 50007              # The same port as used by the server

def connect(host, port):
    s = None
    for res in socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_STREAM):
        af, socktype, proto, canonname, sa = res
        try:
            s = socket.socket(af, socktype, proto)
        except socket.error, msg:
            s = None
            continue
        try:
            s.connect(sa)
        except socket.error, msg:
            s.close()
            s = None
            continue
        break
    if s is None:
        print 'could not open socket'
        return None
    return s

def send(s,obj):
    payload = pickle.dumps(obj)
    s.send(struct.pack(">L",len(payload)))
    s.send(payload)

def main():
    global observer

    observer = MazeObserver()
    observer.seed = 12345
    observer.start()

    if len(sys.argv) > 1:
        filename = sys.argv[1]
        logfile = open(filename)
        if not logfile:
            print >> sys.stderr, "Could not open file %s" % filename
            sys.exit(1)
        observer.play_log(logfile)
    else:
        # otherwise start in server mode
        print "Accepting connections on port %d" % PORT
        server = PoseServer( ('', PORT), PoseHandler)
        observer.server = server
        server.serve_forever()

# start the maze observer if run directly
if __name__=='__main__':
    main()
