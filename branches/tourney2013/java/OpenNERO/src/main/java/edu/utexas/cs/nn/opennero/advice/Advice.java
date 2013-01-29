package edu.utexas.cs.nn.opennero.advice;

import edu.utexas.cs.nn.opennero.Message;
import java.io.StringWriter;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

/**
 * An XML serializable class that represents a piece of advice.
 * @author <a href="mailto:ikarpov@cs.utexas.edu">Igor Karpov</a>
 */
public class Advice implements Message.Content {
    @Element
    private String advice;
    
    public Advice() {}
    
    public Advice(String advice) {
        this.advice = advice;
    }
    
    public String getAdvice() {
        return this.advice;
    }
    
    public String getXml() {
        StringWriter sw = new StringWriter();
        Serializer ser = new Persister();
        try {
            ser.write(this, sw);
        } catch (Exception e) {
            return null;
        }
        return sw.toString();
    }
    
    @Override
    public String toString() {
        return advice;
    }
}
