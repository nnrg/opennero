#!/usr/bin/env python
"""Process and display log files from OpenNERO.

plot_client reads a file and sends it over the network for plot_server to process.
"""

import sys
import socket
from subprocess import Popen

HOST, PORT = "127.0.0.1", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4096

class NetworkLogWriter:
    def __init__(self, host = HOST, port = PORT, start_server = True):
        self.addr = (host, port)
        self.connected = False
        self.failed = False
        self.server_process = None
        if start_server:
            missing = set([])
            try:
                import wx
            except ImportError:
                missing.add('wx')
            try:
                import matplotlib
            except ImportError:
                missing.add('matplotlib')
            try:
                import numpy
            except ImportError:
                missing.add('numpy')
            if not len(missing):
                try:
                    self.server_process = Popen(['python','plot_server.py'])
                    print 'plot server started!'
                except:
                    print 'Could not start plot server!'
            else:
                import tkMessageBox
                tkMessageBox.showwarning('Warning!', 'Could not start plot window because the following module(s) are missing: ' + ', '.join(missing))
                

    def connect(self):
        if self.connected:
            return True
        if self.failed:
            return False
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(self.addr)
            self.connected = True
        except socket.error:
            self.connected = False
        except IOError:
            self.connected = False
        return self.connected

    def write(self, msg):
        if self.connect():
            try:
                self.sock.send(msg)
            except socket.error:
                self.failed = True
                print 'socket.error'
            except IOError:
                self.failed = True
                print 'IOError'
        if self.failed:
            print msg

    def flush(self):
        pass

    def close(self):
        if self.connected and not self.failed:
            try:
                self.sock.sendto('', self.addr)
                self.sock.close()
            except socket.error:
                pass
            except IOError:
                pass
            self.connected = False
        if self.server_process:
            self.server_process.kill()

    def __del__(self):
        self.close()

def main():
    # open input
    f = sys.stdin
    if len(sys.argv) > 1:
        f = open(sys.argv[1])
    # Create writer
    output = NetworkLogWriter()
    # Send messages
    for line in f.xreadlines():
        print >>output, line,
    f.close()

if __name__ == "__main__":
    main()
