#ifndef _ORGANISM_H_
#define _ORGANISM_H_

#include "neat.h"
#include "genome.h"
#include "species.h"
#include "pool.h"
#include "XMLSerializable.h"
#include <string>
#include <ostream>
#include <map>

namespace NEAT
{

    class Species;
    class Population;
    class Pool;

    /// ORGANISM CLASS:
    /// Organisms are Genomes and Networks with fitness information, 
    /// i.e. The genotype and phenotype together
    class Organism : public XMLSerializable
    {
            friend class boost::serialization::access;

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

            // Print the Organism's genome to a file preceded by a comment detailing the organism's species, number, and fitness 
            bool print_to_file(const std::string& filename); //PFHACK
            bool write_to_file(std::ofstream &outFile);

            Organism(double fit, GenomePtr g, int gen,
                     const std::string &md = "");
            Organism(const Organism& org); ///<  Copy Constructor
            ~Organism();

            //Should this organism have a fitness penalty?
            bool smited;
            
            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::organism");
                ar & BOOST_SERIALIZATION_NVP(fitness);
                ar & BOOST_SERIALIZATION_NVP(winner);
                ar & BOOST_SERIALIZATION_NVP(gnome);
                // ar & BOOST_SERIALIZATION_NVP(net);
                ar & BOOST_SERIALIZATION_NVP(species);
                ar & BOOST_SERIALIZATION_NVP(expected_offspring);
                ar & BOOST_SERIALIZATION_NVP(generation);
                ar & BOOST_SERIALIZATION_NVP(eliminate);
                ar & BOOST_SERIALIZATION_NVP(champion);
                ar & BOOST_SERIALIZATION_NVP(super_champ_offspring);
                ar & BOOST_SERIALIZATION_NVP(pop_champ);
                ar & BOOST_SERIALIZATION_NVP(pop_champ_child);        
            }
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

    /// write organism to stream
    std::ostream& operator<<(std::ostream& out, const OrganismPtr& x);
    
    /// read organism from stream
    std::istream& operator>>(std::istream& in, OrganismPtr& x);
} // namespace NEAT

#endif
