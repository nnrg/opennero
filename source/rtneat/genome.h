#ifndef _GENOME_H_
#define _GENOME_H_

#include <vector>
#include <ostream>
#include <string>
#include <boost/enable_shared_from_this.hpp>
#include "neat.h"
#include "XMLSerializable.h"

namespace NEAT
{
    enum mutator
    {
        GAUSSIAN = 0,
        COLDGAUSSIAN = 1
    };

    //-----------------------------------------------------------------------
    //A Genome is the primary source of genotype information used to create
    //a phenotype.  It contains 3 major constituents:
    //  1) A list of Traits
    //  2) A list of NNodes pointing to a Trait from (1)
    //  3) A list of Genes with Links that point to Traits from (1)
    //(1) Reserved parameter space for future use
    //(2) NNode specifications
    //(3) Is the primary source of innovation in the evolutionary Genome.
    //    Each Gene in (3) has a marker telling when it arose historically.
    //    Thus, these Genes can be used to speciate the population, and the
    //    list of Genes provide an evolutionary history of innovation and
    //    link-building.

    class Genome : public boost::enable_shared_from_this<Genome>, public XMLSerializable
    {
            friend class boost::serialization::access;
            Genome() {}

        public:
            S32 genome_id;

            std::vector<TraitPtr> traits; //parameter conglomerations
            std::vector<NNodePtr> nodes; //List of NNodes for the Network
            std::vector<GenePtr> genes; //List of innovation-tracking genes
            std::vector<FactorPtr> factors; // list of factors that have had an influence on this gene

            NetworkWeakPtr phenotype; //Allows Genome to be matched with its Network

            S32 get_last_node_id(); //Return id of final NNode in Genome
            F64 get_last_gene_innovnum(); //Return last innovation number in Genome

            void print_genome(); //Displays Genome on screen

            //Constructor which takes full genome specs and puts them into the new one
            Genome(S32 id, std::vector<TraitPtr> t, std::vector<NNodePtr> n,
                   std::vector<GenePtr> g, std::vector<FactorPtr> f);

            //Constructor which takes in links (not genes) and creates a Genome
            Genome(S32 id, std::vector<TraitPtr> t, std::vector<NNodePtr> n,
                   std::vector<LinkPtr> links);

            // Copy constructor
            Genome(const Genome& genome);

            //Special constructor which spawns off an input file
            //This constructor assumes that some routine has already read in GENOMESTART
            Genome(S32 id, std::ifstream &inFile);

            // This special constructor creates a Genome
            // with i inputs, o outputs, n out of nmax hidden units, and random
            // connectivity.  If r is true then recurrent connections will
            // be included.
            // The last input is a bias
            // Linkprob is the probability of a link
            Genome(S32 new_id, S32 i, S32 o, S32 n, S32 nmax, bool r,
                   F64 linkprob);

            //Special constructor that creates a Genome of 3 possible types:
            //0 - Fully linked, no hidden nodes
            //1 - Fully linked, one hidden node splitting each link
            //2 - Fully connected with a hidden layer, recurrent
            //num_hidden is only used in type 2
            Genome(S32 num_in, S32 num_out, S32 num_hidden, S32 type);

            // Loads a new Genome from a file (doesn't require knowledge of Genome's id)
            static GenomePtr new_Genome_load(const std::string& filename);

            //Destructor kills off all lists (including the trait vector)
            ~Genome();

            //Generate a network phenotype from this Genome with specified id
            NetworkPtr genesis(int);

            // Lamarckian weight changes from network phenotype
            void Lamarck();

            // Dump this genome to specified file
            void print_to_file(std::ofstream &outFile);

            // Wrapper for print_to_file above
            void print_to_filename(const std::string& filename);

            // Duplicate this Genome to create a new one with the specified id
            GenomePtr duplicate(S32 new_id);

            // For debugging: A number of tests can be run on a genome to check its
            // integrity
            // Note: Some of these tests do not indicate a bug, but rather are meant
            // to be used to detect specific system states
            bool verify();

            // ******* MUTATORS *******

            // Perturb params in one trait
            void mutate_random_trait();

            // Change random link's trait. Repeat times times
            void mutate_link_trait(S32 times);

            // Change random node's trait times times
            void mutate_node_trait(S32 times);

            // Add Gaussian noise to linkweights either GAUSSIAN or COLDGAUSSIAN (from zero)
            void mutate_link_weights(F64 power, F64 rate, mutator mut_type);

            // toggle genes on or off
            void mutate_toggle_enable(S32 times);

            // Find first disabled gene and enable it
            void mutate_gene_reenable();

            // These last kinds of mutations return false if they fail
            //   They can fail under certain conditions,  being unable
            //   to find a suitable place to make the mutation.
            //   Generally, if they fail, they can be called again if desired.

            // Mutate genome by adding a node respresentation
            bool mutate_add_node(std::vector<InnovationPtr> &innovs,
                                 S32 &curnode_id, F64 &curinnov);

            // Mutate the genome by adding a new link between 2 random NNodes
            bool mutate_add_link(std::vector<InnovationPtr> &innovs,
                                 F64 &curinnov, S32 tries);

            void mutate_add_sensor(std::vector<InnovationPtr> &innovs,
                                   double &curinnov);

            // ****** MATING METHODS *****

            // This method mates this Genome with another Genome g.
            //   For every point in each Genome, where each Genome shares
            //   the innovation number, the Gene is chosen randomly from
            //   either parent.  If one parent has an innovation absent in
            //   the other, the baby will inherit the innovation
            //   Interspecies mating leads to all genes being inherited.
            //   Otherwise, excess genes come from most fit parent.
            GenomePtr mate_multipoint(GenomePtr g, S32 genomeid, F64 fitness1,
                                      F64 fitness2, bool interspec_flag);

            //This method mates like multipoint but instead of selecting one
            //   or the other when the innovation numbers match, it averages their
            //   weights
            GenomePtr mate_multipoint_avg(GenomePtr g, S32 genomeid,
                                          F64 fitness1, F64 fitness2,
                                          bool interspec_flag);

            // This method is similar to a standard single point CROSSOVER
            //   operator.  Traits are averaged as in the previous 2 mating
            //   methods.  A point is chosen in the smaller Genome for crossing
            //   with the bigger one.
            GenomePtr mate_singlepoint(GenomePtr g, S32 genomeid);

            // ******** COMPATIBILITY CHECKING METHODS ********

            // This function gives a measure of compatibility between
            //   two Genomes by computing a linear combination of 3
            //   characterizing variables of their compatibilty.
            //   The 3 variables represent PERCENT DISJOINT GENES,
            //   PERCENT EXCESS GENES, MUTATIONAL DIFFERENCE WITHIN
            //   MATCHING GENES.  So the formula for compatibility
            //   is:  disjoint_coeff*pdg+excess_coeff*peg+mutdiff_coeff*mdmg.
            //   The 3 coefficients are global system parameters
            F64 compatibility(GenomePtr g);

            F64 trait_compare(TraitPtr t1, TraitPtr t2);

            // Return number of non-disabled genes
            S32 extrons();

            // Randomize the trait pointers of all the node and connection genes
            void randomize_traits();

            // For the Sensor Registry
            std::vector<std::string> getSensorNames() const;
            std::vector<std::string> getSensorArgs() const;

            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::genome");
                ar & BOOST_SERIALIZATION_NVP(genome_id);
                ar & BOOST_SERIALIZATION_NVP(traits);
                ar & BOOST_SERIALIZATION_NVP(nodes);
                ar & BOOST_SERIALIZATION_NVP(genes);
                ar & BOOST_SERIALIZATION_NVP(factors);
                // ar & BOOST_SERIALIZATION_NVP(phenotype); // TODO: don't really need to save the network
            }
        protected:
            //Inserts a NNode into a given ordered list of NNodes in order
            void node_insert(std::vector<NNodePtr> &nlist, NNodePtr n);

            //Adds a new gene that has been created through a mutation in the
            //*correct order* into the list of genes in the genome
            void add_gene(std::vector<GenePtr> &glist, GenePtr g);

    };

    //Calls special constructor that creates a Genome of 3 possible types:
    //0 - Fully linked, no hidden nodes
    //1 - Fully linked, one hidden node splitting each link
    //2 - Fully connected with a hidden layer
    //num_hidden is only used in type 2
    //Saves to file "auto_genome"
    GenomePtr new_Genome_auto(int num_in, int num_out, int num_hidden,
                              int type, const std::string& filename);

    void print_Genome_tofile(GenomePtr g, const std::string& filename);

    /// write genome to stream
    std::ostream& operator<<(std::ostream& out, const GenomePtr& x);

    /// read genome from stream
    std::istream& operator>>(std::istream& in, GenomePtr& x);
} // namespace NEAT

#endif
