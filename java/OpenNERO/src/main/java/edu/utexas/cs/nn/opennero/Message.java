package edu.utexas.cs.nn.opennero;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 *
 * @author <a href="mailto:ikarpov@cs.utexas.edu">Igor Karpov</a>
 */
@Root
public class Message {
    @Element
    private Content content;
    public Message() {}
    public Message(Content content) {
        this.content = content;
    }
    public Content getContent() { return content; }
    public interface Content {
    }
    @Override
    public String toString() {
        return this.content.toString();
    }
    public static void main(String[] args) {
        Content c = new Command("a", "b");
        Message m = new Message(c);
        System.out.println(SocketClient.instance.toXml(m));
    }
}
