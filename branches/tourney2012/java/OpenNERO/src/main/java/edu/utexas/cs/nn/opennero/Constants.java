package edu.utexas.cs.nn.opennero;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

/**
 * This is a "constants with default values" class.
 *
 * To define custom values for these constants, use System.setProperty() or the
 * -D parameter to the JVM. For example, it's possible to set the MOVEMENT_SPEED
 * by specifying:
 *
 *    java ... -DMOVEMENT_SPEED=150
 *
 * @author Igor Karpov (ikarpov@cs.utexas.edu)
 */
public enum Constants {
    /**
     * Print debugging messages
     */
    DEBUG("false"),
    
    /**
     * The host to connect to
     */
    HOST("localhost"),
    
    /**
     * The port to connect to
     */
    PORT(8888), 
    
    /**
     * Log file for recording the training procedure.
     */
    LOG_FILE("Training.log");
        
    private String defaultValue;

    Constants(Object o) {
        this.defaultValue = String.valueOf(o);
    }

    public String get() {
        return System.getProperty(name(), defaultValue);
    }

    public int getInt() {
        return Integer.valueOf(get());
    }

    public double getDouble() {
        return Double.valueOf(get());
    }

    public float getFloat() {
        return Float.valueOf(get());
    }

    public boolean getBoolean() {
        return Boolean.valueOf(get());
    }

    public static Random random = new Random();

    public static String timestamp(String filename) {
        if (filename.endsWith(".db")) {
            filename = filename.substring(0,filename.length() - 3);
        }
        Date date = new Date();
        SimpleDateFormat df = new SimpleDateFormat("-yyyyMMdd-HHmmss");
        filename = filename + df.format(date) + ".db";
        return filename;
    }
}
