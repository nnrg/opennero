#ifndef _INNOVATION_H_
#define _INNOVATION_H_

#include "neat.h"
#include "XMLSerializable.h"

namespace NEAT
{

    enum innovtype
    {
        NEWNODE = 0,
        NEWLINK = 1
    };

    // ------------------------------------------------------------
    // This Innovation class serves as a way to record innovations
    //   specifically, so that an innovation in one genome can be 
    //   compared with other innovations in the same epoch, and if they
    //   are the same innovation, they can both be assigned the same
    //   innovation number.
    //
    //  This class can encode innovations that represent a new link
    //  forming, or a new node being added.  In each case, two 
    //  nodes fully specify the innovation and where it must have
    //  occured.  (Between them)                                     
    // ------------------------------------------------------------ 
    class Innovation : public XMLSerializable
    {
        private:
            friend class boost::serialization::access;
            Innovation() {}

        public:
            innovtype innovation_type; //Either NEWNODE or NEWLINK

            int node_in_id; //Two nodes specify where the innovation took place
            int node_out_id;

            double innovation_num1; //The number assigned to the innovation
            double innovation_num2; // If this is a new node innovation, then there are 2 innovations (links) added for the new node 

            double new_weight; //  If a link is added, this is its weight 
            int new_traitnum; // If a link is added, this is its connected trait 

            int newnode_id; // If a new node was created, this is its node_id 

            double old_innov_num; // If a new node was created, this is the innovnum of the gene's link it is being stuck inside 

            bool recur_flag;

            //Constructor for the new node case
            Innovation(int nin, int nout, double num1, double num2, int newid,
                       double oldinnov);

            //Constructor for new link case
            Innovation(int nin, int nout, double num1, double w, int t);

            //Constructor for a recur link
            Innovation(int nin, int nout, double num1, double w, int t,
                       bool recur);
                       
            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::innovation");
                ar & BOOST_SERIALIZATION_NVP(innovation_type);
                ar & BOOST_SERIALIZATION_NVP(node_in_id);
                ar & BOOST_SERIALIZATION_NVP(node_out_id);
                ar & BOOST_SERIALIZATION_NVP(innovation_num1);
                ar & BOOST_SERIALIZATION_NVP(innovation_num2);
                ar & BOOST_SERIALIZATION_NVP(new_weight);
                ar & BOOST_SERIALIZATION_NVP(new_traitnum);
                ar & BOOST_SERIALIZATION_NVP(newnode_id);
                ar & BOOST_SERIALIZATION_NVP(old_innov_num);
                ar & BOOST_SERIALIZATION_NVP(recur_flag);
                
            }
    };

} // namespace NEAT

#endif
