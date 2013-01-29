package edu.utexas.cs.nn.opennero;

import edu.utexas.cs.nn.opennero.Message.Content;
import org.simpleframework.xml.Attribute;

/**
 *
 * @author ikarpov
 */
public class ErrorMessage implements Content {
    @Attribute
    public String name;
    @Attribute
    public String text;
    
    @Override
    public String toString() {
        return "Error: " + name + ", " + text;
    }
}
