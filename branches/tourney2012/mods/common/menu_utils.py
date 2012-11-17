import cPickle
import socket
import struct
import select
import time

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

    def read_data(self):
        self.outputs = []
        try:
            inputready, outputready, exceptready = select.select(self.inputs, self.outputs, [], 0)
        except select.error, e:
            print 'ScriptServer select error'
            return None
        for s in inputready:
            if s == self.server:
                # this is a new connection
                client, address = self.server.accept()
                print 'ScriptServer got connection %d from %s' % (client.fileno(), address)
                self.scriptmap[client] = address
                self.outputs.append(client)
                self.inputs.append(client)
            else:
                # this is a message on an existing connection
                try:
                    data = receive(s)
                    if data:
                        return data
                    else:
                        print 'ScriptServer: %d hung up' % s.fileno()
                        s.close()
                        if s in self.inputs:
                            self.inputs.remove(s)
                        if s in self.outputs:
                            self.outputs.remove(s)
                except socket.error, e:
                    print 'ScriptServer socket error: %s' % e
                    s.close();
                    if s in self.inputs:
                        self.inputs.remove(s)
                    if s in self.outputs:
                        self.outputs.remove(s)
        return None

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
