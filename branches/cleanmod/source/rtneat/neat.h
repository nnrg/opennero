#ifndef _NERO_NEAT_H_
#define _NERO_NEAT_H_

#include "core/Common.h"
#include "mersennetwister.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace NEAT
{
    using namespace OpenNero;
    extern U32 time_alive_minimum; // Minimum time alive to be considered for selection or death in real-time evolution 
    const S32 num_trait_params = 8;

    extern F64 trait_param_mut_prob;
    extern F64 trait_mutation_power; // Power of mutation on a signle trait param 
    extern F64 linktrait_mut_sig; // Amount that mutation_num changes for a trait change inside a link
    extern F64 nodetrait_mut_sig; // Amount a mutation_num changes on a link connecting a node that changed its trait 
    extern F64 weight_mut_power; // The power of a linkweight mutation 
    extern F64 recur_prob; // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 

    // These 3 global coefficients are used to determine the formula for
    // computating the compatibility between 2 genomes.  The formula is:
    // disjoint_coeff*pdg+excess_coeff*peg+mutdiff_coeff*mdmg.
    // See the compatibility method in the Genome class for more info
    // They can be thought of as the importance of disjoint Genes,
    // excess Genes, and parametric difference between Genes of the
    // same function, respectively. 
    extern F64 disjoint_coeff;
    extern F64 excess_coeff;
    extern F64 mutdiff_coeff;

    // This global tells compatibility threshold under which two Genomes are considered the same species 
    extern F64 compat_threshold;

    // Globals involved in the epoch cycle - mating, reproduction, etc.. 
    extern F64 age_significance; // How much does age matter? 
    extern F64 survival_thresh; // Percent of ave fitness for survival 
    extern F64 mutate_only_prob; // Prob. of a non-mating reproduction 
    extern F64 mutate_random_trait_prob;
    extern F64 mutate_link_trait_prob;
    extern F64 mutate_node_trait_prob;
    extern F64 mutate_link_weights_prob;
    extern F64 mutate_toggle_enable_prob;
    extern F64 mutate_gene_reenable_prob;
    extern F64 mutate_add_node_prob;
    extern F64 mutate_add_link_prob;
    extern F64 interspecies_mate_rate; // Prob. of a mate being outside species 
    extern F64 mate_multipoint_prob;
    extern F64 mate_multipoint_avg_prob;
    extern F64 mate_singlepoint_prob;
    extern F64 mate_only_prob; // Prob. of mating without mutation 
    extern F64 recur_only_prob; // Probability of forcing selection of ONLY links that are naturally recurrent 
    extern S32 pop_size; // Size of population 
    extern S32 dropoff_age; // Age where Species starts to be penalized 
    extern S32 newlink_tries; // Number of tries mutate_add_link will attempt to find an open link 
    extern S32 print_every; // Tells to print population to file every n generations 
    extern S32 babies_stolen; // The number of babies to siphon off to the champions 
    extern F64 backprop_learning_rate; // Learning rate of back-propagation algorithm
    extern F64 max_link_weight; // Link weights are capped at this (and negative of this) value

    extern MTRand NEATRandGen; // Random number generator; can pass seed value as argument

    // Inline Random Functions 
    extern inline S32 randposneg()
    {
        if (NEAT::NEATRandGen.randInt()%2)
            return 1;
        else
            return -1;
    }

    extern inline S32 randint(S32 x, S32 y)
    {
        return NEAT::NEATRandGen.randInt()%(y-x+1)+x;
    }

    extern inline F64 randfloat()
    {
        return NEAT::NEATRandGen.rand();
    }

    // SIGMOID FUNCTION ********************************
    // This is a signmoidal activation function, which is an S-shaped squashing function
    // It smoothly limits the amplitude of the output of a neuron to between 0 and 1
    // It is a helper to the neural-activation function get_active_out
    // It is made inline so it can execute quickly since it is at every non-sensor 
    // node in a network.
    // NOTE:  In order to make node insertion in the middle of a link possible,
    // the signmoid can be shifted to the right and more steeply sloped:
    // slope=4.924273
    // constant= 2.4621365
    // These parameters optimize mean squared error between the old output,
    // and an output of a node inserted in the middle of a link between
    // the old output and some other node. 
    // When not right-shifted, the steepened slope is closest to a linear
    // ascent as possible between -0.5 and 0.5
    extern F64 fsigmoid(F64, F64, F64);

    // Linear Activation Function
    // It specifies a function with a slope and offset.
    extern F64 flinear(F64, F64, F64);

    // Hebbian Adaptation Function
    // Based on equations in Floreano & Urzelai 2000
    // Takes the current weight, the maximum weight in the containing network,
    // the activation coming in and out of the synapse,
    // and three learning rates for hebbian, presynaptic, and postsynaptic
    // modification
    // Returns the new modified weight
    // NOTE: For an inhibatory connection, it makes sense to
    //      emphasize decorrelation on hebbian learning!	
    extern F64 hebbian(F64 weight, F64 maxweight, F64 active_in,
                       F64 active_out, F64 hebb_rate, F64 pre_rate,
                       F64 post_rate);

    // Returns a normally distributed deviate with 0 mean and unit variance
    extern F64 gaussrand();

    bool load_neat_params(const std::string& filename);

    class Population;
    typedef boost::shared_ptr<Population> PopulationPtr; ///< shared automatic pointer to a population
    typedef boost::weak_ptr<Population> PopulationWeakPtr; ///< weak automatic pointer to a population (break reference cycles)

    class Network;
    typedef boost::shared_ptr<Network> NetworkPtr;
    typedef boost::weak_ptr<Network> NetworkWeakPtr;

    class Genome;
    typedef boost::shared_ptr<Genome> GenomePtr;
    typedef boost::weak_ptr<Genome> GenomeWeakPtr;

    class Organism;
    typedef boost::shared_ptr<Organism> OrganismPtr;
    typedef boost::weak_ptr<Organism> OrganismWeakPtr;

    class Species;
    typedef boost::shared_ptr<Species> SpeciesPtr;
    typedef boost::weak_ptr<Species> SpeciesWeakPtr;

    class NNode;
    typedef boost::shared_ptr<NNode> NNodePtr;
    typedef boost::weak_ptr<NNode> NNodeWeakPtr;

    class Innovation;
    typedef boost::shared_ptr<Innovation> InnovationPtr;
    typedef boost::weak_ptr<Innovation> InnovationWeakPtr;

    class Gene;
    typedef boost::shared_ptr<Gene> GenePtr;
    typedef boost::weak_ptr<Gene> GeneWeakPtr;

    class Trait;
    typedef boost::shared_ptr<Trait> TraitPtr;
    typedef boost::weak_ptr<Trait> TraitWeakPtr;

    class Link;
    typedef boost::shared_ptr<Link> LinkPtr;
    typedef boost::weak_ptr<Link> LinkWeakPtr;

    class Factor;
    typedef boost::shared_ptr<Factor> FactorPtr;
    typedef boost::weak_ptr<Factor> FactorWeakPtr;

    const extern std::string INDENT;

} // namespace NEAT

#endif
