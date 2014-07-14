import cPickle
import socket
import struct
import select
import time
import Queue
import sys

marshall = cPickle.dumps
unmarshall = cPickle.loads

def send(channel, *args):
    buf = marshall(args)
    value = socket.htonl(len(buf))
    size = struct.pack("I", value)
    channel.send(size)
    channel.send(buf)

def receive(channel):
    size = struct.calcsize("I")
    try:
        size = channel.recv(size)
    except socket.error as msg:
        return ''
    try:
        size = socket.ntohl(struct.unpack("I", size)[0])
    except struct.error as e:
        return ''
    buf = ''
    while len(buf) < size:
        try:
            buf += channel.recv(size - len(buf))
        except socket.error as msg:
            return ''
    try:
        return unmarshall(buf)[0]
    except:
        return buf

HOST = '127.0.0.1'
PORT = 8888

class ScriptServer:
    __single = None

    def __init__(self, port = PORT, backlog = 5):
        # only init once!
        if ScriptServer.__single:
            raise ScriptServer.__single
        ScriptServer.__single = self
        self.scriptmap = {}
        self.outputs = []
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # This will allow us to reopen the socket after an un-clean shutdown.
        # See https://code.google.com/p/opennero/issues/detail?id=110
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        attempts = 10
        while True:
            try:
                self.server.bind((HOST, port))
                break
            except socket.error:
                print 'ScriptServer: port %d busy, waiting (%d more attempts) ...' % (port, attempts)
                time.sleep(1)
                attempts -= 1
                if attempts == 0:
                    raise
        print 'ScriptServer listening to port', port, '...'
        self.server.listen(backlog)
        self.inputs = [self.server]
        self.msg_queues = {}

    def read_data(self):
        try:
            inputready, outputready, exceptready = select.select(self.inputs, self.outputs, [], 0)
        except select.error, e:
            print 'ScriptServer select error'
            return None
        # handle input if available
        for s in inputready:
            if s is self.server:
                # this is a new connection
                client, address = self.server.accept()
                self.scriptmap[client] = address
                #client.setblocking(0)
                self.inputs.append(client)
                # Give the connection a queue for data we want to send
                self.msg_queues[client] = Queue.Queue()
            else:
                # this is a message on an existing connection
                data = receive(s)
                if data is not None:
                    return data
                else:
                    s.close()
                    if s in self.inputs:
                        self.inputs.remove(s)
                    if s in self.outputs:
                        self.outputs.remove(s)
        # Handle output, if ready
        for s in outputready:
            try:
                next_msg = self.msg_queues[s].get_nowait()
            except Queue.Empty:
                self.outputs.remove(s)
            else:
                size = len(next_msg)
                size = socket.htonl(size)
                size = struct.pack("I", size)
                s.send(size)
                s.send(next_msg)
        # Handle "exceptional conditions"
        for s in exceptready:
            # Stop listening for input on the connection
            self.inputs.remove(s)
            if s in self.outputs:
                self.outputs.remove(s)
            s.close()
            # Remove message queue
            del self.msg_queues[s]
        return None
        
    
    def write_data(self, msg):
        for s in self.msg_queues.keys():
            self.msg_queues[s].put(msg)
            if s not in self.outputs:
                self.outputs.append(s)

def GetScriptServer(port=PORT):
    """
    Only allow the code to create one script server
    """
    try:
        theServer = ScriptServer(port)
    except ScriptServer, s:
        theServer = s
    return theServer

class ScriptClient:
    def __init__(self, host = HOST, port = PORT):
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
