#ifndef _GENE_H_
#define _GENE_H_

#include "neat.h"
#include "trait.h"
#include "link.h"
#include "network.h"

namespace NEAT
{

    class Gene
    {
        public:

            LinkPtr lnk;
            double innovation_num;
            double mutation_num; //Used to see how much mutation has changed the link
            bool enable; //When this is off the Gene is disabled
            bool frozen; //When frozen, the linkweight cannot be mutated

            /// default constructor (0s everywhere)
            Gene();

            //Construct a gene with no trait
            Gene(double w, const NNodePtr inode, const NNodePtr onode,
                 bool recur, double innov, double mnum);

            //Construct a gene with a trait
            Gene(TraitPtr tp, double w, const NNodePtr inode,
                 const NNodePtr onode, bool recur, double innov, double mnum);

            //Construct a gene off of another gene as a duplicate
            Gene(GenePtr g, TraitPtr tp, const NNodePtr inode,
                 const NNodePtr onode);

            //Construct a gene from a file spec given traits and nodes
            Gene(std::istream &iFile, std::vector<TraitPtr> &traits,
                 std::vector<NNodePtr> &nodes);

            // Copy Constructor
            Gene(const Gene& gene);

            ~Gene();

            //Print gene to a file- called from Genome
            void print_to_file(std::ofstream &outFile);
            
    };

} // namespace NEAT


#endif
