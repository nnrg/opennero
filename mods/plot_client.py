#!/usr/bin/env python
"""Process and display log files from OpenNERO.

plot_client reads a file and sends it over the network for plot_server to process.
"""

import sys
import socket
from subprocess import Popen

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4096

class NetworkLogWriter:
    def __init__(self, host = HOST, port = PORT, start_server = True):
        self.addr = (host, port)
        self.connected = False
        self.failed = False
        self.server_process = None
        if start_server:
            try:
                import wx, matplotlib, numpy, pylab
                self.server_process = Popen(['python','plot_server.py'])
                print 'plot server started!'
            except:
                print 'Could not start plot server!'

    def connect(self):
        if self.connected:
            return True
        if self.failed:
            return False
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(self.addr)
            self.connected = True
        except:
            self.connected = False
        return self.connected

    def write(self, msg):
        if self.connect():
            try:
                self.sock.send(msg)
            except socket.error, e:
                self.failed = True

    def flush(self):
        pass

    def close(self):
        if self.connected and not self.failed:
            self.sock.sendto('', self.addr)
            self.sock.close()
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
