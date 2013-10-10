package edu.utexas.cs.nn.opennero;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * A local representation of the remote evolving collection of genomes.
 * @author ikarpov
 */
public class Population {
    /**
     * there is a fixed number of bodies and when we receive a new genome-body 
     * assignment we replace the old ones.
     */
    private Map<Integer, Genome> bodies = new HashMap<Integer, Genome>();
    
    /**
     * The genome with the current maximum fitness.
     */
    private Genome fittest = null;
    
    public Genome getFittest() {
        if (bodies.isEmpty()) {
            return null;
        }
        if (fittest == null) {
            List<Genome> genomes = new ArrayList<Genome>(bodies.values());
            Collections.max(genomes, new Comparator<Genome>() {
                public int compare(Genome o1, Genome o2) {
                    if (o1.getFitness() == o2.getFitness()) {
                        return 0;
                    }
                    return o1.getFitness() < o2.getFitness() ? -1 : 1;
                }
                
            });
            fittest = genomes.get(genomes.size()-1);
        }
        return fittest;
    }
    
    public void add(Genome genome) {
        bodies.put(genome.getBodyId(), genome);
        if (genome.isChamp()) {
            System.err.println("champ received!");
            fittest = genome;
        }
        fittest = null;
    }
    
    public int size() {
        return bodies.size();
    }
}
