#ifndef _POOL_H_
#define _POOL_H_

#include "neat.h"
#include "organism.h"
#include "population.h"
#include "species.h"

namespace NEAT
{

    class Organism;
    class Population;
    class Species;

    // ---------------------------------------------  
    // POOL CLASS:
    //   A Pool is a group of similar Organisms
    //	 which are the highest organisms within
    //	 their species.
    // ---------------------------------------------  
    class Pool
    {

        public:

            int id;
            int age; //The age of the Species 
            std::vector<OrganismPtr> organisms;// = new std::vector(); //The organisms in the Species
            double average_est; //When playing real-time allows estimating average fitness

            bool add_Organism(OrganismPtr o);

            OrganismPtr first();

            bool print_to_file(std::ofstream &outFile);

            //Remove an organism from Species
            bool remove_org(OrganismPtr org);

            size_t size() const
            {
                return organisms.size();
            }

            bool isEmpty()
            {
                return organisms.empty();
            }

            // *** Real-time methods *** 

            //Place organisms in this species in order by their fitness
            bool rank();

            //Empty the existing organisms in the memory pool, thus removing all organisms in the pool.
            bool flush()
            {
                if (!isEmpty())
                {
                    organisms.clear();
                    return true;
                }
                return false;

            }

            //Like the usual reproduce() method except only one offspring is produced
            //Note that "generation" will be used to just count which offspring # this is over all evolution
            //Here is how to get sorted species:
            //    Sort the Species by max fitness (Use an extra list to do this)
            //    These need to use ORIGINAL fitness
            //      sorted_species.sort(order_species);
            OrganismPtr reproduce_one(int generation, PopulationPtr pop,
                                      std::vector<SpeciesPtr> &sorted_species);

            Pool();

            //Allows the creation of a Species that won't age (a novel one)
            //This protects new Species from aging inside their first generation
            //Species(int i,bool n);

            ~Pool();

    };
}

#endif
