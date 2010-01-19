#!/usr/bin/env python
"""Process and display log files from OpenNERO.

plot_client reads a file and sends it over the network for plot_server to process.
"""

import sys
import socket

HOST, PORT = "localhost", 9999
ADDR = (HOST, PORT)
BUFSIZE = 4096

def main():
    # Create socket
    UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    f = sys.stdin
    if len(sys.argv) > 1:
        f = open(sys.argv[1])
    # Send messages
    for line in f.xreadlines():
        line = line.strip()
        if line:
            UDPSock.sendto(line,ADDR)
    UDPSock.sendto('',ADDR)
    f.close()
    UDPSock.close()

if __name__ == "__main__":
    main()
