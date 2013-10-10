package edu.utexas.cs.nn.opennero;

import edu.utexas.cs.nn.opennero.shaping.FitnessWeights;
import edu.utexas.cs.nn.opennero.shaping.FitnessWeights.Dimension;
import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

/**
 *
 * @author ikarpov
 */
@Root
public class Genome implements Message.Content {
    @Attribute
    private int id;
    
    @Attribute
    private int bodyId;
    
    @Attribute
    private float fitness;
    
    @Attribute
    private int timeAlive;
    
    @Attribute(required=false)
    private boolean champ = false;
    
    @Element
    private FitnessWeights rawFitness;
    
    public Genome() {
        
    }
    
    public Genome(int id, int body_id, float fitness, FitnessWeights rawFitness, int timeAlive) {
        this.id = id;
        this.bodyId = body_id;
        this.fitness = fitness;
        this.rawFitness = rawFitness;
        this.timeAlive = timeAlive;
    }
    
    public int getId() { return id; }
    
    public int getBodyId() { return bodyId; }
    
    public float getFitness() { return fitness; }
    
    public int getTimeAlive() { return timeAlive; }
    
    public FitnessWeights getRawFitness() { return rawFitness; }
    
    public boolean isChamp() { return champ; }
    
    @Override
    public String toString() {
        return "Genome(\nid: " + id + ",\nbodyId: " + bodyId + ",\nfitness: " + fitness + ",\nrawFitness:\n" + rawFitness + ")";
    }
    
    public static void main(String[] args) {
        Serializer ser = new Persister();
        FitnessWeights rawFitness = new FitnessWeights();
        for (Dimension d : Dimension.values()) {
            rawFitness.put(d, Constants.random.nextFloat());
        }
        try {
            Genome gnm = new Genome(123, 890, 45.67f, rawFitness, 123456);
            ser.write(gnm, System.out);
            System.out.println();
            System.out.println(gnm);
        } catch (Exception ex) {
            ex.printStackTrace(System.err);
        }
    }

}
