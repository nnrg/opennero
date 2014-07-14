#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "neat.h"
#include "nnode.h"
#include "XMLSerializable.h"

namespace NEAT
{
    class Genome;

    /// A NETWORK is a LIST of input NODEs and a LIST of output NODEs           
    ///   The point of the network is to define a single entity which can evolve
    ///   or learn on its own, even though it may be part of a larger framework 
    class Network : public XMLSerializable
    {

            friend class Genome;
            friend class boost::serialization::access;
            Network() {}

        protected:

            S32 numnodes; ///< The number of nodes in the net (-1 means not yet counted)
            S32 numlinks; ///< The number of links in the net (-1 means not yet counted)

            std::vector<NNodePtr>::iterator input_iter; ///< For GUILE network inputting  //PFHACK

            void destroy(); ///< Kills all nodes and links within
            void destroy_helper(NNodePtr curnode,
                                std::vector<NNodePtr> &seenlist); ///< helper for destroy()

            void nodecounthelper(const NNodePtr curnode, S32 &counter,
                                 std::vector<NNodePtr> &seenlist) const;
            void linkcounthelper(const NNodePtr curnode, S32 &counter,
                                 std::vector<NNodePtr> &seenlist) const;

        public:

            std::vector<NNodePtr> all_nodes; /// A list of all the nodes

            GenomeWeakPtr genotype; ///< Allows Network to be matched with its Genome

            std::string name; ///< Every Network or subNetwork can have a name
            std::vector<NNodePtr> inputs; ///< NNodes that input into the network
            std::vector<NNodePtr> outputs; ///< Values output by the network

            S32 net_id; ///< Allow for a network id

            F64 maxweight; ///< Maximum weight in network for adaptation purposes

            bool adaptable; ///< Tells whether network can adapt or not

            /// This constructor allows the input and output lists to be supplied
            Network(std::vector<NNodePtr> in, std::vector<NNodePtr> out,
                    std::vector<NNodePtr> all, S32 netid);

            /// This constructs a net with empty input and output lists
            Network(S32 netid);

            /// Copy Constructor
            Network(const Network& network);

            ~Network();

            /// Puts the network back into an inactive state
            void flush();

            /// Verify flushedness for debugging
            void flush_check();

            /// Activates the net such that all outputs are active
            bool activate();

            /// Back-propagates error in the net such that all inputs are active
            bool backprop();

            /// Prints the values of all nodes, inputs, or outputs
            void show_activation() const;
            void show_input() const;
            void show_output() const;

            /// Add a new input node
            void add_input(NNodePtr);

            /// Add a new output node
            void add_output(NNodePtr);

            /// Takes an array of sensor values and loads it into SENSOR inputs ONLY
            void load_sensors(const F64*);
            /// Takes a vector of sensor values and loads it into SENSOR inputs ONLY
            void load_sensors(const std::vector<F64> &sensvals);

            /// Takes an array of error values and loads it into OUTPUT nodes
            void load_errors(const std::vector<F64> &errorvals);

            /// Takes and array of output activations and OVERRIDES the outputs' actual 
            /// activations with these values (for adaptation)
            void override_outputs(F64*);

            /// Name the network
            void give_name(const std::string& name);

            /// Counts the number of nodes in the net if not yet counted
            S32 nodecount() const;

            /// Counts the number of links in the net if not yet counted
            S32 linkcount() const;

            /// This checks a POTENTIAL link between a potential in_node
            /// and potential out_node to see if it must be recurrent 
            /// Use count and thresh to jump out in the case of an infinite loop 
            bool is_recur(NNodePtr potin_node, NNodePtr potout_node,
                          S32 &count, S32 thresh) const;

            /// Some functions to help GUILE input into Networks   //PFHACK
            S32 input_start();
            S32 load_in(F64 d);

            /// If all output are not active then return true
            bool outputsoff() const;

            /// If all nodes are not active then return true
            bool nodesoff() const;

            /// If the gradient in all nodes are not active then return true
            bool gradientoff() const;

            /// Just print connections weights with carriage returns
            void print_links_tofile(const std::string& filename) const;
            
            /// Just print connections weights with carriage returns
            void print_links() const;
            
            /// serialize this object to/from a Boost serialization archive
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                //LOG_F_DEBUG("rtNEAT", "serialize::network");
                ar & BOOST_SERIALIZATION_NVP(all_nodes);
                ar & BOOST_SERIALIZATION_NVP(genotype);
                ar & BOOST_SERIALIZATION_NVP(name);
                ar & BOOST_SERIALIZATION_NVP(numnodes);
                ar & BOOST_SERIALIZATION_NVP(numlinks);
                ar & BOOST_SERIALIZATION_NVP(inputs);
                ar & BOOST_SERIALIZATION_NVP(outputs);
                ar & BOOST_SERIALIZATION_NVP(net_id);
                ar & BOOST_SERIALIZATION_NVP(adaptable);
            }
    };

    std::ostream& operator<<(std::ostream& out, const NetworkPtr& node);

} // namespace NEAT

#endif
