#ifndef _POPULATION_H_
#define _POPULATION_H_

#include "neat.h"
#include "innovation.h"
#include "genome.h"
#include "species.h"
#include "organism.h"
#include "pool.h"
#include <boost/enable_shared_from_this.hpp>

namespace NEAT
{

    class Species;
    class Organism;
    class Pool;

    // ---------------------------------------------  
    // POPULATION CLASS:
    //   A Population is a group of Organisms   
    //   including their species                        
    // ---------------------------------------------  
    class Population : public boost::enable_shared_from_this<Population>
    {
        private:
            friend class boost::serialization::access;
        
            Population() {}
        
            // A Population can be spawned off of a single Genome 
            // There will be size Genomes added to the Population 
            // The Population does not have to be empty to add Genomes 
            bool spawn(GenomePtr g, S32 size);

        public:

			PopulationPtr copy(PopulationPtr p);

			std::vector<OrganismPtr> organisms; //The organisms in the Population

            std::vector<SpeciesPtr> species; // Species in the Population. Note that the species should comprise all the genomes 

            // ******* Member variables used during reproduction *******
            std::vector<InnovationPtr> innovations; // For holding the genetic innovations of the newest generation
            S32 cur_node_id; //Current label number available
            F64 cur_innov_num;

            S32 last_species; //The highest species number

            // ******* Fitness Statistics *******
            F64 mean_fitness;
            F64 variance;
            F64 standard_deviation;

            S32 winnergen; //An integer that when above zero tells when the first winner appeared

            // ******* When do we need to delta code? *******
            F64 highest_fitness; //Stagnation detector
            S32 highest_last_changed; //If too high, leads to delta coding

            // Separate the Organisms into species
            bool speciate();

            // Print all the genomes in the population to a file
            bool print_to_file(std::ofstream &outFile);

            // Print Population to a file in speciated order with comments separating each species
            bool print_to_file_by_species(std::ofstream &outFile);

            // Prints the champions of each species to files starting with directory_prefix
            // The file name are as follows: [prefix]g[generation_num]cs[species_num]
            // Thus, they can be indexed by generation or species
            bool print_species_champs_tofiles(std::string& directory_prefix,
                                              S32 generation);

            // Run verify on all Genomes in this Population (Debugging)
            bool verify();

            // Turnover the population to a new generation using fitness 
            // The generation argument is the next generation
            bool epoch(S32 generation);

            // *** Real-time methods *** 

            // Places the organisms in species in order from best to worst fitness 
            bool rank_within_species();

            // Estimates average fitness for all existing species
            void estimate_all_averages();

            //Reproduce only out of the pop champ
            OrganismPtr reproduce_champ(S32 generation);

            // Probabilistically choose a species to reproduce
            // Note that this method is effectively real-time fitness sharing in that the 
            // species will tend to produce offspring in an amount proportional
            // to their average fitness, which approximates the generational
            // method of producing the next generation of the species en masse
            // based on its average (shared) fitness.  
            SpeciesPtr choose_parent_species();

            //Remove a species from the species list (sometimes called by remove_worst when a species becomes empty)
            bool remove_species(SpeciesPtr spec);

            // Removes worst member of population that has been around for a minimum amount of time and returns
            // a pointer to the Organism that was removed (note that the pointer will not point to anything at all,
            // since the Organism it was pointing to has been deleted from memory)
            OrganismPtr remove_worst();

            OrganismPtr remove_worst_probabilistic();

            // Similar to remove_worst(), but considers only the first range organisms.
            OrganismPtr remove_worst(U32 range);

            //KEN: New 2/17/04
            //This method takes an Organism and reassigns what Species it belongs to
            //It is meant to be used so that we can reasses where Organisms should belong
            //as the speciation threshold changes.
            void reassign_species(OrganismPtr org);

            //Move an Organism from one Species to another (called by reassign_species)
            void switch_species(OrganismPtr org, SpeciesPtr orig_species,
                                SpeciesPtr new_species);

            // Add an organism to the population and to the proper species.
            void add_organism(OrganismPtr org);

            // Construct off of a single spawning Genome 
            Population(GenomePtr g, S32 size);

            // Construct off of a single spawning Genome without mutation
            Population(GenomePtr g, S32 size, F32 power);

            //MSC Addition
            // Construct off of a vector of genomes with a mutation rate of "power"
            Population(std::vector<Genome*> genomeList, F32 power);

            bool clone(GenomePtr g, S32 size, F32 power);

            //// Special constructor to create a population of random topologies     //PFHACK
            //// uses Genome(int i, int o, int n,int nmax, bool r, double linkprob) 
            //// See the Genome constructor for the argument specifications
            //Population(int size,int i,int o, int nmax, bool r, double linkprob);

            // Construct off of a file of Genomes 
            Population(const std::string& filename);

            // Construct off of a file of Genomes to the specified size.
            Population(const std::string& filename, S32 size);

            // It can delete a Population in two ways:
            //    -delete by killing off the species
            //    -delete by killing off the organisms themselves (if not speciated)
            // It does the latter if it sees the species list is empty
            ~Population();
            
            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::population");
                ar & BOOST_SERIALIZATION_NVP(organisms);
                ar & BOOST_SERIALIZATION_NVP(species);
                ar & BOOST_SERIALIZATION_NVP(last_species);
                ar & BOOST_SERIALIZATION_NVP(cur_innov_num);
                ar & BOOST_SERIALIZATION_NVP(cur_node_id);
                ar & BOOST_SERIALIZATION_NVP(innovations);
                ar & BOOST_SERIALIZATION_NVP(mean_fitness);
                ar & BOOST_SERIALIZATION_NVP(variance);
                ar & BOOST_SERIALIZATION_NVP(standard_deviation);
                ar & BOOST_SERIALIZATION_NVP(winnergen);
                ar & BOOST_SERIALIZATION_NVP(highest_fitness);
                ar & BOOST_SERIALIZATION_NVP(highest_last_changed);
            }
    };
    
    /// write the population out to stream (XML serialization)
    std::ostream& operator<<(std::ostream& out, const PopulationPtr& x);
    
    /// read the population from stream (XML serialization)
    std::istream& operator>>(std::istream& in, PopulationPtr& x);
    
} // namespace NEAT

#endif
