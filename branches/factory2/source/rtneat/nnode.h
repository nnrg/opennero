#ifndef _NNODE_H_
#define _NNODE_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/enable_shared_from_this.hpp>
#include "neat.h"
#include "trait.h"
#include "link.h"
#include "XMLSerializable.h"

namespace NEAT
{
    enum nodetype
    {
        NEURON = 0,
        SENSOR = 1
    };

    enum nodeplace
    {
        HIDDEN = 0,
        INPUT = 1,
        OUTPUT = 2,
        BIAS = 3
    };

    const char* getNodePlaceString(nodeplace place);

    enum functype
    {
        SIGMOID = 0,
        LINEAR = 1
    };

    class Link;

    // ----------------------------------------------------------------------- 
    // A NODE is either a NEURON or a SENSOR.  
    //   - If it's a sensor, it can be loaded with a value for output
    //   - If it's a neuron, it has a list of its incoming input signals (List<Link> is used) 
    // Use an activation count to avoid flushing
    class NNode : public boost::enable_shared_from_this<NNode>, public XMLSerializable
    {

            friend class Network;
            friend class Genome;
            friend class boost::serialization::access;
            
            NNode() {}

    public:
            bool active_flag; // To make sure outputs are active
            F64 activesum; // The incoming activity before being processed 
            F64 activation; // The total activation entering the NNode 
            // NOT USED IN NEAT - covered by "activation" above
            F64 output; // Output of the NNode- the value in the NNode 
            std::vector<LinkPtr> incoming; // A list of pointers to incoming weighted signals from other nodes
            std::vector<LinkWeakPtr> outgoing; // A list of pointers to links carrying this node's signal

            F64 last_activation; // Holds the previous step's activation for recurrency
            F64 last_activation2; // Holds the activation BEFORE the prevous step's

            bool gradient_flag; // To make sure inputs are active in backprop
            F64 gradientsum; // The gradient activity before being processed 
            F64 gradient; // Local gradient used in backprop
            S32 gradient_count; // keeps track of which activation the node is currently in

            nodetype type; // type is either NEURON or SENSOR

            S32 activation_count; // keeps track of which activation the node is currently in

            S32 node_id; // A node can be given an identification number for saving in files

            functype ftype; // type is either SIGMOID ..or others that can be added

            // This is necessary for a special recurrent case when the innode
            // of a recurrent link is one time step ahead of the outnode.
            // The innode then needs to send from TWO time steps ago

            TraitPtr nodetrait; // Points to a trait of parameters

            nodeplace gen_node_label; // Used for genetic marking of nodes


            NNodePtr dup; // Used for Genome duplication

            NNodePtr analogue; // Used for Gene decoding

            U32 linkcount;  // Used for making Lamarckian weight changes

            bool override; // The NNode cannot compute its own output- something is overriding it

            F64 override_value; // Contains the activation value that will override this node's activation

            // Pointer to the Sensor corresponding to this Body.
            std::string _sensorName;
            std::string _sensorArgs;

            bool frozen; // When frozen, cannot be mutated (meaning its trait pointer is fixed)

            S32 trait_id; // identify the trait derived by this node

            // ************ LEARNING PARAMETERS *********** 
            // The following parameters are for use in    
            //   neurons that learn through habituation,
            //   sensitization, or Hebbian-type processes  

            F64 params[NEAT::num_trait_params];

            NNode(nodetype ntype, S32 nodeid);

            NNode(nodetype ntype, S32 nodeid, nodeplace placement);

            NNode(nodetype ntype, S32 nodeid, nodeplace placement, functype function);

            // Construct a NNode off another NNode for genome purposes
            NNode(NNodePtr n, TraitPtr t);

            // Construct the node out of a file specification using given list of traits
            NNode(std::istream &iFile, std::vector<TraitPtr> &traits);

            // Copy Constructor
            NNode(const NNode& nnode);

            ~NNode();

            // Just return activation for step
            F64 get_active_out();

            // Return activation from PREVIOUS time step
            F64 get_active_out_td();

            // Just return gradient for step
            F64 get_gradient_out();

            // Returns the type of the node, NEURON or SENSOR
            const nodetype get_type();

            // Allows alteration between NEURON and SENSOR.  Returns its argument
            nodetype set_type(nodetype);

            // If the node is a SENSOR, returns true and loads the value
            bool sensor_load(F64);

           // Loads the error for backprop and computes local gradient from it
            void error_load(F64);

            // Adds a NONRECURRENT Link to a new NNode in the incoming List
            void add_incoming(NNodePtr, F64);

            // Adds a Link to a new NNode in the incoming List
            void add_incoming(NNodePtr, F64, bool);

            // Recursively deactivate backwards through the network
            void flushback();

            // Print the node to a file
            void print_to_file(std::ofstream &outFile);

            // Have NNode gain its properties from the trait
            void derive_trait(TraitPtr curtrait);

            // Returns the gene that created the node
            NNodePtr get_analogue();

            // Force an output value on the node
            void override_output(F64 new_output);

            // Tell whether node has been overridden
            bool overridden();

            // Set activation to the override value and turn off override
            void activate_override();

            // Writes back changes weight values into the genome
            // (Lamarckian trasnfer of characteristics)
            void Lamarck();

            // Get the Name and Arguments for the Sensor
            std::string getSensorName()
            {
                return _sensorName;
            }
            std::string getSensorArgs()
            {
                return _sensorArgs;
            }
            
            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::nnode");
                ar & BOOST_SERIALIZATION_NVP(node_id);
                ar & BOOST_SERIALIZATION_NVP(incoming);
                ar & BOOST_SERIALIZATION_NVP(outgoing);
                ar & BOOST_SERIALIZATION_NVP(type);
                ar & BOOST_SERIALIZATION_NVP(activation_count);
                ar & BOOST_SERIALIZATION_NVP(ftype);
                ar & BOOST_SERIALIZATION_NVP(nodetrait);
                ar & BOOST_SERIALIZATION_NVP(gen_node_label);
                ar & BOOST_SERIALIZATION_NVP(_sensorName);
                ar & BOOST_SERIALIZATION_NVP(_sensorArgs);
                ar & BOOST_SERIALIZATION_NVP(frozen);
                ar & BOOST_SERIALIZATION_NVP(trait_id);        
            }

    };

    std::ostream& operator<<(std::ostream& out, const NNodePtr& x);
    
} // namespace NEAT

#endif
