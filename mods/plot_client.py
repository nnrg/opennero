#!/usr/bin/env python
"""Process and display log files from OpenNERO.

plot_client reads a file and sends it over the network for plot_server to process.
"""

import sys
import socket

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4096

class NetworkLogWriter:
    def __init__(self, host = HOST, port = PORT):
        self.addr = (host, port)
        self.connected = False
        self.connect()
    def connect(self):
        if self.connected:
            return
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(self.addr)
            self.connected = True
        except:
            self.connected = False
    def write(self, msg):
        self.connect()
        self.sock.send(msg)
    def flush(self):
        pass
    def close(self):
        self.sock.sendto('', self.addr)
        self.sock.close()

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
    output.close()
    f.close()

if __name__ == "__main__":
    main()
