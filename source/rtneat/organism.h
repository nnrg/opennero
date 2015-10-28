#ifndef _ORGANISM_H_
#define _ORGANISM_H_

#include "neat.h"
#include "genome.h"
#include "species.h"
#include <string>
#include <ostream>
#include <map>

namespace NEAT
{

    class Species;
    class Population;

    /// ORGANISM CLASS:
    /// Organisms are Genomes and Networks with fitness information, 
    /// i.e. The genotype and phenotype together
    class Organism
    {
        Organism() {}
        
        public:
            double fitness; //A measure of fitness for the Organism
            double orig_fitness; ///< A fitness measure that won't change during adjustments
            double error; ///< Used just for reporting purposes
            bool winner; ///< Win marker (if needed for a particular task)
            GenomePtr gnome; ///< The Organism's genotype 
            NetworkPtr net; ///< The Organism's phenotype
            SpeciesWeakPtr species; ///< The Organism's Species (not owner)
            double expected_offspring; ///< Number of children this Organism may have
            int generation; ///< Tells which generation this Organism is from
            bool eliminate; ///< Marker for destruction of inferior Organisms
            bool champion; ///< Marks the species champ
            int super_champ_offspring; ///< Number of reserved offspring for a population leader
            bool pop_champ; ///< Marks the best in population
            bool pop_champ_child; ///< Marks the duplicate child of a champion (for tracking purposes)
            double high_fit; ///< DEBUG variable- high fitness of champ
            int time_alive; ///< When playing in real-time allows knowing the maturity of an individual

            // Track its origin- for debugging or analysis- we can tell how the organism was born
            bool mut_struct_baby;
            bool mate_baby;

            // MetaData for the object
            std::string metadata;
            bool modified;

            // Regenerate the network based on a change in the genotype 
            void update_phenotype();

            // Update genotype based on changes in the network (for Lamarckian evolution)
            void update_genotype();

            Organism(double fit, GenomePtr g, int gen,
                     const std::string &md = "");
            Organism(const Organism& org); ///<  Copy Constructor
            ~Organism();

            //Should this organism have a fitness penalty?
            bool smited;
    };

    // This is used for list sorting of Organisms by fitness..highest fitness first
    bool order_orgs(OrganismPtr x, OrganismPtr y);

    bool order_orgs_by_adjusted_fit(OrganismPtr x, OrganismPtr y);

    /// Utility class used for managing tokens stores in the metadata of an organism.
    class MetadataParser
    {
        public:

                MetadataParser(OrganismPtr org,
                               const std::string &delimToken = " ");

            /// get the value associated with a given key
            std::string GetKeyVal(const std::string &key);

            /// set a key value pair
            void SetKeyVal(const std::string &key, const std::string &val);

            /// update the organism with the metadata we have modified
            void UpdateOrganism();

        private:

            OrganismPtr mParsedOrganism; ///< organism whose metadata we are parsing
            std::map<std::string,std::string> mTokens; ///< metadata key value pair tokens
    };


    std::ostream& operator<<(std::ostream& out, const OrganismPtr& organism);
    std::ostream& operator<<(std::ostream& out, const Organism& organism);
} // namespace NEAT

#endif
