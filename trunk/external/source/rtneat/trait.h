#ifndef _TRAIT_H_
#define _TRAIT_H_

#include "neat.h"
#include "XMLSerializable.h"
#include <iostream>
#include <fstream>

namespace NEAT
{

    // ------------------------------------------------------------------ 
    // TRAIT: A Trait is a group of parameters that can be expressed     
    //        as a group more than one time.  Traits save a genetic      
    //        algorithm from having to search vast parameter landscapes  
    //        on every node.  Instead, each node can simply point to a trait 
    //        and those traits can evolve on their own 
    class Trait : public XMLSerializable
    {
            friend class boost::serialization::access;

            // ************ LEARNING PARAMETERS *********** 
            // The following parameters are for use in    
            //   neurons that learn through habituation,
            //   sensitization, or Hebbian-type processes  

        public:
            S32 trait_id; // Used in file saving and loading
            F64 params[NEAT::num_trait_params]; // Keep traits in an array

            Trait();

            Trait(S32 id, F64 p1, F64 p2, F64 p3, F64 p4, F64 p5, F64 p6,
                  F64 p7, F64 p8, F64 p9);

            // Copy Constructor
            Trait(const Trait& t);

            // Create a trait exactly like another trait
            Trait(TraitPtr t);

            // Special constructor off a file assume word "trait" has been read in
            Trait(std::istream &argline);

            // Special Constructor creates a new Trait which is the average of 2 existing traits passed in
            Trait(TraitPtr t1, TraitPtr t2);

            // Dump trait to a file
            void print_to_file(std::ofstream &file);

            // Perturb the trait parameters slightly
            void mutate();

            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::trait");
                ar & BOOST_SERIALIZATION_NVP(trait_id);
            }
    };

    std::ostream& operator<<(std::ostream& out, const TraitPtr& x);

} // namespace NEAT

#endif
