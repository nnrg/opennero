#ifndef _LINK_H_
#define _LINK_H_

#include "neat.h"
#include "trait.h"
#include "nnode.h"
#include <string>

namespace NEAT
{
    // ----------------------------------------------------------------------- 
    // A LINK is a connection from one node to another with an associated weight 
    // It can be marked as recurrent 
    // Its parameters are made public for efficiency 
    class Link
    {
            Link() {}
    
    
            NNodeWeakPtr in_node; // NNode inputting into the link
            NNodeWeakPtr out_node; // NNode that the link affects
        public:
            F64 weight; // Weight of connection
            bool is_recurrent;
            bool time_delay;

            S32 trait_id; // identify the trait derived by this link

            TraitPtr linktrait; // Points to a trait of parameters for genetic creation

            // ************ LEARNING PARAMETERS *********** 
            // These are link-related parameters that change during Hebbian type learning

            F64 added_weight; // The amount of weight adjustment 
            F64 params[NEAT::num_trait_params];

            Link(F64 w, NNodePtr inode, NNodePtr onode, bool recur);

            // Including a trait pointer in the Link creation
            Link(TraitPtr lt, F64 w, const NNodePtr inode,
                 const NNodePtr onode, bool recur);

            // For when you don't know the connections yet
            Link(F64 w);

            // Copy Constructor
            Link(const Link& link);

            // Derive a trait into link params
            void derive_trait(TraitPtr curtrait);

            // get input node
            NNodePtr get_in_node() const
            {
                return in_node.lock();
            }

            // set input node
            void set_in_node(NNodePtr p)
            {
                in_node = p;
            }

            // get output node
            NNodePtr get_out_node() const
            {
                return out_node.lock();
            }

            // set output node
            void set_out_node(NNodePtr p)
            {
                out_node = p;
            }

    };

} // namespace NEAT

#endif
