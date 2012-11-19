package edu.utexas.cs.nn.opennero.shaping;

import com.google.common.collect.ForwardingMap;
import edu.utexas.cs.nn.opennero.Constants;
import edu.utexas.cs.nn.opennero.Message;
import java.util.EnumMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.simpleframework.xml.ElementMap;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

/**
 * An XML serializable class that represents fitness weights.
 * @author <a href="mailto:ikarpov@cs.utexas.edu">Igor Karpov</a>
 */
public class FitnessWeights 
    extends ForwardingMap<FitnessWeights.Dimension, Float>
    implements Message.Content {

    public enum Dimension {
        STAND_GROUND("Stand Ground"),
        STICK_TOGETHER("Stick Together"),
        APPROACH_ENEMY("Approach Enemy"),
        APPROACH_FLAG("Approach Flag"),
        HIT_TARGET("Hit Target"),
        AVOID_FIRE("Avoid Fire");

        String text;

        float defaultValue = 0;

        Dimension(String text) {
            this.text = text;
        }
        
        Dimension(String text, Float defaultValue) {
            this.text = text;
            this.defaultValue = defaultValue;
        }
        
        @Override
        public String toString() {
            return text;
        }
    }
    
    @ElementMap(inline=true, attribute=true)
    private EnumMap<Dimension, Float> weights = new EnumMap<Dimension, Float>(Dimension.class);
    
    public FitnessWeights() {
        for (Dimension d : Dimension.values()) {
            put(d, d.defaultValue);
        }
    }
    
    @Override
    protected Map<Dimension, Float> delegate() {
        return weights;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (Dimension d : Dimension.values()) {
            sb.append(d).append(": ").append(get(d)).append("\n");
        }
        return sb.toString();
    }

    public static void main(String args[]) {
        Map<FitnessWeights.Dimension, Float> map = new FitnessWeights();
        for (Dimension d : Dimension.values()) {
            map.put(d, Constants.random.nextFloat() * 200.0f - 100.0f);
        }
        for (Dimension d : map.keySet()) {
            System.out.println(d.name() + "(" + d + "): " + map.get(d));
        }
        Serializer ser = new Persister();
        try {
            ser.write(map, System.out);
        } catch (Exception ex) {
            Logger.getLogger(FitnessWeights.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
