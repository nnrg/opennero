package edu.utexas.cs.nn.opennero;

/**
 *
 * @author <a href="mailto:ikarpov@cs.utexas.edu">Igor Karpov</a>
 */
import java.io.*;
import java.net.*;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

public class SocketClient {
    
    Socket socket = null;
    DataInputStream dis = null;
    DataOutputStream dos = null;
    
    public static final SocketClient instance = new SocketClient();
    
    private SocketClient() {
        try {
            socket = new Socket(Constants.HOST.get(), Constants.PORT.getInt());
            dis = new DataInputStream(socket.getInputStream());
            dos = new DataOutputStream(socket.getOutputStream());
        } catch (UnknownHostException e) {
            System.err.println("Java UI not connect to OpenNERO");
        } catch (IOException e) {
            System.err.println("Java UI not connect to OpenNERO");
        }
    }
    
    public String receive() {
        if (socket == null) {
            return null;
        }
        try {
            int size = dis.readInt();
            byte[] buf = new byte[size];
            dis.readFully(buf);
            return new String(buf);
        } catch (IOException ex) {
            return null;
        }
    }

    public boolean send(String s) {
        if (s == null || s.isEmpty()) {
            return false;
        }
        if (socket == null) {
            System.out.println(s);
            return true;
        }
        try {
            dos.writeInt(s.getBytes().length);
            dos.writeBytes(s);
            return true;
        } catch (IOException ex) {
            return false;
        }
    }

    public void close() {
        if (dis != null) {
            try {
                dis.close();
            } catch (IOException ex) {
                //ignore
            }
        }
        if (dos != null) {
            try {
                dos.close();
            } catch (IOException ex) {
                //ignore
            }
        }        if (socket != null) {
            try {
                socket.close();
            } catch (IOException ex) {
                //ignore
            }
        }        
    }
}
