package edu.utexas.cs.nn.opennero;

import org.simpleframework.xml.Attribute;

/**
 *
 * @author ikarpov
 */
public class Command implements Message.Content {
    @Attribute
    private String command;
    @Attribute
    private String arg;
    public Command() {
    }
    public Command(String command, String arg) {
        this.command = command;
        this.arg = arg;
    }
    public String getCommand() { return command; }
    public String getArg() { return arg; }
    @Override
    public String toString() {
        return command + ": " + arg;
    }
}
