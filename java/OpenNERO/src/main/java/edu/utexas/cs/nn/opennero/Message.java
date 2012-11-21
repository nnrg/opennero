package edu.utexas.cs.nn.opennero;

import java.io.StringWriter;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

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
    
    private static final Serializer serializer = new Persister();
    
    public String toXML() {
        StringWriter sw = new StringWriter();
        try {
            serializer.write(this, sw);
        } catch (Exception ex) {
            return "";
        }
        return sw.toString();
    }
    
    public static void main(String[] args) {
        Content c = new Command("a", "b");
        Message m = new Message(c);
        System.out.println(m.toXML());
    }
}
