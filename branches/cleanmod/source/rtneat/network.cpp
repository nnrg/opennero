#include "core/Common.h"
#include "network.h"

using namespace NEAT;
using namespace std;

Network::Network(vector<NNodePtr> in, vector<NNodePtr> out,
                 vector<NNodePtr> all, S32 netid) :
    numnodes(),
    numlinks(-1), 
    all_nodes(all), 
    genotype(), 
    name(""), 
    inputs(in), 
    outputs(out), 
    net_id(netid), 
    adaptable(false)
{
}

Network::Network(S32 netid) :
    numnodes(-1), 
    numlinks(-1), 
    name(""), 
    net_id(netid), 
    adaptable(false)
{
}

Network::Network(const Network& network) :
    numnodes(network.numnodes), 
    numlinks(network.numlinks),
    name(network.name), 
    net_id(network.net_id), 
    adaptable(network.adaptable)
{
    vector<NNodePtr>::const_iterator curnode;

    // Copy all the inputs
    for (curnode = network.inputs.begin(); curnode != network.inputs.end(); ++curnode)
    {
        NNodePtr n(new NNode(**curnode));
        inputs.push_back(n);
        all_nodes.push_back(n);
    }

    // Copy all the outputs
    for (curnode = network.outputs.begin(); curnode != network.outputs.end(); ++curnode)
    {
        NNodePtr n(new NNode(**curnode));
        outputs.push_back(n);
        all_nodes.push_back(n);
    }
}

Network::~Network()
{
    destroy(); // Kill off all the nodes and links
}

// Puts the network back into an initial state
void Network::flush()
{
    vector<NNodePtr>::iterator curnode;

    for (curnode=outputs.begin(); curnode!=outputs.end(); ++curnode)
    {
        (*curnode)->flushback();
    }
}

// If all output are not active then return true
bool Network::outputsoff() const
{
    vector<NNodePtr>::const_iterator curnode;

    for (curnode=outputs.begin(); curnode!=outputs.end(); ++curnode)
    {
        if ((*curnode)->activation_count == 0)
            return true;
    }

    return false;
}

// If all nodes are not active then return true
bool Network::nodesoff() const
{
    vector<NNodePtr>::const_iterator curnode;

    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        if ((*curnode)->activation_count == 0)
            return true;
    }

    return false;
}

// If the gradient in all nodes are not active then return true
bool Network::gradientoff() const
{
    vector<NNodePtr>::const_iterator curnode;

    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        if ((*curnode)->gradient_count == 0)
            return true;
    }

    return false;
}

// Print the connections weights to a file separated by only carriage returns
void Network::print_links_tofile(const std::string& filename) const
{
    vector<NNodePtr>::const_iterator curnode;
    vector<LinkPtr>::const_iterator curlink;

    ofstream oFile(filename.c_str(), ios::out);

    //Make sure it worked
    if (!oFile)
    {
        cerr<<"Can't open "<<filename<<" for output"<<endl;
        return;
    }

    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        if (((*curnode)->type)!=SENSOR)
        {
            for (curlink=((*curnode)->incoming).begin(); curlink!=((*curnode)->incoming).end(); ++curlink)
            {
                oFile<<(*curlink)->get_in_node()->node_id<<" -> "<<(*curlink)->get_out_node()->node_id<<" : "<<(*curlink)->weight<<endl;
            } // end for loop on links
        } //end if
    } //end for loop on nodes

    oFile.close();

} //print_links_tofile

// Print the connections weights separated by only carriage returns
void Network::print_links() const
{
    vector<NNodePtr>::const_iterator curnode;
    vector<LinkPtr>::const_iterator curlink;

    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        if (((*curnode)->type)!=SENSOR)
        {
            for (curlink=((*curnode)->incoming).begin(); curlink!=((*curnode)->incoming).end(); ++curlink)
            {
                cout<<(*curlink)->get_in_node()->node_id<<" -> "<<(*curlink)->get_out_node()->node_id<<" : "<<(*curlink)->weight;
                if ((*curlink)->is_recurrent) cout<<" recur";
                cout<<endl;
            } // end for loop on links
        } //end if
    } //end for loop on nodes

} //print_links

// Activates the net such that all outputs are active
// Returns true on success;
bool Network::activate()
{
    vector<NNodePtr>::iterator curnode;
    vector<LinkPtr>::iterator curlink;
    F64 add_amount; //For adding to the activesum
    bool onetime; //Make sure we at least activate once
    S32 abortcount=0; //Used in case the output is somehow truncated from the network

    //cout<<"Activating network: "<<this->genotype<<endl;

    //Keep activating until all the nodes have become active 
    //(This only happens on the first activation, because after that they
    // are always active)

    onetime=false;
    
    while (nodesoff()||!onetime)
    {

        ++abortcount;

        if (abortcount==20)
        {
            return false;
            cerr<<"Inputs disconnected from output!"<<endl;
        }
        //cerr<<"Outputs are off"<<endl;

        // For each node, compute the sum of its incoming activation 
        for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
        {
            //Ignore SENSORS

            if (((*curnode)->type)!=SENSOR)
            {
                (*curnode)->activesum=0;
                (*curnode)->active_flag=false; //This will tell us if it has any active inputs

                // For each incoming connection, add the activity from the connection to the activesum 
                for (curlink=((*curnode)->incoming).begin(); curlink!=((*curnode)->incoming).end(); ++curlink)
                {
                    //Handle possible time delays
                    if (!((*curlink)->time_delay))
                    {
                        add_amount=((*curlink)->weight)*(((*curlink)->get_in_node())->get_active_out());
                        if ((((*curlink)->get_in_node())->active_flag)||(((*curlink)->get_in_node())->type==SENSOR))
                            (*curnode)->active_flag=true;
                        (*curnode)->activesum+=add_amount;
                        //cout<<"Node "<<(*curnode)->node_id<<" adding "<<add_amount<<" from node "<<((*curlink)->get_in_node())->node_id<<endl;
                    }
                    else
                    {
                        //Input over a time delayed connection
                        add_amount=((*curlink)->weight)*(((*curlink)->get_in_node())->get_active_out_td());
                        (*curnode)->activesum+=add_amount;
                    }

                } //End for over incoming links

            } //End if (((*curnode)->type)!=SENSOR) 

        } //End for over all nodes

        // Now activate all the non-sensor nodes off their incoming activation 
        for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
        {

            if (((*curnode)->type)!=SENSOR)
            {
                //Only activate if some active input came in
                if ((*curnode)->active_flag)
                {
                    //cout<<"Activating "<<(*curnode)->node_id<<" with "<<(*curnode)->activesum<<": ";

                    //Keep a memory of activations for potential time delayed connections
                    (*curnode)->last_activation2=(*curnode)->last_activation;
                    (*curnode)->last_activation=(*curnode)->activation;

                    //If the node is being overrided from outside,
                    //stick in the override value
                    if ((*curnode)->overridden())
                    {
                        //Set activation to the override value and turn off override
                        (*curnode)->activate_override();
                    }
                    else
                    {
                        //Now run the net activation through an activation function
                        if ((*curnode)->ftype==SIGMOID)
                            (*curnode)->activation=fsigmoid((*curnode)->activesum, 4.924273, 2.4621365); //Sigmoidal activation- see comments under fsigmoid
                        else if ((*curnode)->ftype==LINEAR)
                            (*curnode)->activation=flinear((*curnode)->activesum, 1.0, 0.0);
                    }
                    //cout<<(*curnode)->activation<<endl;

                    //Increment the activation_count
                    //First activation cannot be from nothing!!
                    (*curnode)->activation_count++;
                }
            }
        }

        onetime=true;
    }

    if (adaptable)
    {

        // ADAPTATION:  Adapt weights based on activations 
        for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
        {
            //Ignore SENSORS

            //cout<<"On node "<<(*curnode)->node_id<<endl;

            if (((*curnode)->type)!=SENSOR)
            {

                // For each incoming connection, perform adaptation based on the trait of the connection 
                for (curlink=((*curnode)->incoming).begin(); curlink!=((*curnode)->incoming).end(); ++curlink)
                {

                    if (((*curlink)->trait_id==2)||((*curlink)->trait_id==3)||((*curlink)->trait_id==4))
                    {

                        //In the recurrent case we must take the last activation of the input for calculating hebbian changes
                        if ((*curlink)->is_recurrent)
                        {
                            (*curlink)->weight=hebbian((*curlink)->weight, maxweight, (*curlink)->get_in_node()->last_activation, (*curlink)->get_out_node()->get_active_out(), (*curlink)->params[0], (*curlink)->params[1], (*curlink)->params[2]);

                        }
                        else
                        { //non-recurrent case
                            (*curlink)->weight=hebbian((*curlink)->weight, maxweight, (*curlink)->get_in_node()->get_active_out(), (*curlink)->get_out_node()->get_active_out(), (*curlink)->params[0], (*curlink)->params[1], (*curlink)->params[2]);
                        }
                    }

                }

            }

        }

    } //end if (adaptable)

    return true;
}

// Back-propagates error in the net such that all inputs are active
// Returns true on success;
bool Network::backprop()
{
    vector<NNodePtr>::iterator curnode;
    F64 add_amount; //For adding to the activesum
    bool onetime; //Make sure we at least activate once
    S32 abortcount=0; //Used in case the output is somehow truncated from the network

    //Keep activating until all the nodes have their gradients active 
    //(This only happens on the first activation, because after that they
    // are always active)

    onetime=false;

    while (gradientoff()||!onetime)
    {

        ++abortcount;

        if (abortcount==20)
        {
            return false;
            cerr<<"Inputs disconnected from output!"<<endl;
        }

        // For all nodes except outputs, compute the sum of its incoming gradients
        for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode) {
            if (((*curnode)->gen_node_label)!=OUTPUT) {
                (*curnode)->gradientsum=0;
                (*curnode)->gradient_flag=false; //This will tell us if it has any active gradients

                // For each outgoing connection, add the activity from the connection to the gradientsum 
                vector<LinkWeakPtr>::iterator curlink;
                for (curlink=((*curnode)->outgoing).begin(); curlink!=((*curnode)->outgoing).end(); ++curlink) {
                    LinkPtr curlinkptr = (*curlink).lock();
                    add_amount=(curlinkptr->weight)*((curlinkptr->get_out_node())->get_gradient_out());
                    if (((curlinkptr->get_out_node())->gradient_flag)||((curlinkptr->get_out_node())->gen_node_label==OUTPUT))
                        (*curnode)->gradient_flag=true;
                    (*curnode)->gradientsum+=add_amount;
                } //End for over outgoing links
            }
        } //End for over all nodes

        // Now activate all the non-output nodes off their incoming gradients
        for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode) {
            if (((*curnode)->gen_node_label)!=OUTPUT) {
                //Only activate if some active gradient came in
                if ((*curnode)->gradient_flag) {
                    F64 derivative = (*curnode)->activation*(1 - (*curnode)->activation);  // assuming sigmoid slope is 1
                    (*curnode)->gradient = derivative*((*curnode)->gradientsum);
                    (*curnode)->gradient_count++;
                }
            }
        }

        onetime=true;
    }

    // Update weights
    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        if (((*curnode)->type)!=SENSOR)
        {
            vector<LinkPtr>::iterator curlink;
            for (curlink=((*curnode)->incoming).begin(); curlink!=((*curnode)->incoming).end(); ++curlink)
            {
                (*curlink)->weight = (*curlink)->weight + NEAT::backprop_learning_rate*(((*curlink)->get_out_node())->get_gradient_out())*(((*curlink)->get_in_node())->get_active_out());
            } // end for loop on links
        } //end if
    } //end for loop on nodes

    return true;
}

// Prints the values of all its nodes
void Network::show_activation() const
{
    vector<NNodePtr>::const_iterator curnode;

    cout<<"Network "<<name<<" with id "<<net_id<<": (";

    for (curnode=all_nodes.begin(); curnode!=all_nodes.end(); ++curnode)
    {
        cout<<"[Node #"<<(*curnode)->node_id<<": "<<(*curnode)->get_active_out()<<"] ";
    }

    cout<<")"<<endl;
}

// Prints the values of its inputs
void Network::show_input() const
{
    vector<NNodePtr>::const_iterator curnode;
    S32 count;

    cout << "Network "<< name << " with id "<< net_id << ": (";

    count=1;
    for (curnode=inputs.begin(); curnode!=inputs.end(); ++curnode)
    {
        cout<<"[Input #"<<count<<": "<<(*curnode)->get_active_out()<<"] ";
        count++;
    }

    cout << ")"<< endl;
}

// Prints the values of its outputs
void Network::show_output() const
{
    vector<NNodePtr>::const_iterator curnode;
    S32 count;

    cout<<"Network "<<name<<" with id "<<net_id<<": (";

    count=1;
    for (curnode=outputs.begin(); curnode!=outputs.end(); ++curnode)
    {
        cout<<"[Output #"<<count<<": "<<(*curnode)->get_active_out()<<"] ";
        count++;
    }

    cout<<")"<<endl;
}

// Add an input
void Network::add_input(NNodePtr in_node)
{
    inputs.push_back(in_node);
    all_nodes.push_back(in_node);
}

// Add an output
void Network::add_output(NNodePtr out_node)
{
    outputs.push_back(out_node);
    all_nodes.push_back(out_node);
}

/**
   BIASes are also SENSORS.  See definition of enums nodetype and
   nodeplace in nnode.h, and Genome constructors in genome.cpp where
   SENSOR NNodes are created.  According to the NEAT documentation,
   nodetype of NNode is used inside Networks and nodeplace of NNode is
   used inside Genomes. The consequence of BIASes also being SENSORS
   is that bias values must also be passed in sensvals.
*/
void Network::load_sensors(const F64 *sensvals)
{
    vector<NNodePtr>::iterator sensPtr;

    for (sensPtr=inputs.begin(); sensPtr!=inputs.end(); ++sensPtr)
    {
        if (((*sensPtr)->type)==SENSOR)
        {
            (*sensPtr)->sensor_load(*sensvals);
            sensvals++;
        }
    }
}

/**
   BIASes are also SENSORS.  See definition of enums nodetype and
   nodeplace in nnode.h, and Genome constructors in genome.cpp where
   SENSOR NNodes are created.  According to the NEAT documentation,
   nodetype of NNode is used inside Networks and nodeplace of NNode is
   used inside Genomes. The consequence of BIASes also being SENSORS
   is that bias values must also be passed in sensvals.
*/
void Network::load_sensors(const vector<F64> &sensvals)
{
    vector<NNodePtr>::iterator sensPtr;
    vector<F64>::const_iterator valPtr;
    
    AssertMsg
        (sensvals.size() == inputs.size(), "Got " << sensvals.size() 
        << " sensors for a network with " << inputs.size()
        << " inputs");

    for (valPtr = sensvals.begin(), sensPtr = inputs.begin(); sensPtr
        != inputs.end() && valPtr != sensvals.end(); ++sensPtr, ++valPtr)
    {
        if (((*sensPtr)->type)==SENSOR)
        {
            (*sensPtr)->sensor_load(*valPtr);
        }
    }
}

void Network::load_errors(const vector<F64> &errorvals)
{
    vector<NNodePtr>::iterator outPtr;
    vector<F64>::const_iterator valPtr;

    for (valPtr = errorvals.begin(), outPtr = outputs.begin(); outPtr
        != outputs.end() && valPtr != errorvals.end(); ++outPtr, ++valPtr)
    {
        (*outPtr)->error_load(*valPtr);
    }
}

// Takes and array of output activations and OVERRIDES 
// the outputs' actual activations with these values (for adaptation)
void Network::override_outputs(F64* outvals)
{

    vector<NNodePtr>::iterator outPtr;

    for (outPtr=outputs.begin(); outPtr!=outputs.end(); ++outPtr)
    {
        (*outPtr)->override_output(*outvals);
        outvals++;
    }

}

void Network::give_name(const string& newname)
{
    name = newname;
}

// The following two methods recurse through a network from outputs
// down in order to count the number of nodes and links in the network.
// This can be useful for debugging genotype->phenotype spawning 
// (to make sure their counts correspond)

S32 Network::nodecount() const
{
    S32 counter=0;
    vector<NNodePtr>::const_iterator curnode;
    vector<NNodePtr>::const_iterator location;
    vector<NNodePtr> seenlist; //List of nodes not to doublecount

    for (curnode=outputs.begin(); curnode!=outputs.end(); ++curnode)
    {

        location=find(seenlist.begin(), seenlist.end(), (*curnode));
        if (location==seenlist.end())
        {
            counter++;
            seenlist.push_back(*curnode);
            nodecounthelper((*curnode), counter, seenlist);
        }
    }

    const_cast<S32&>(numnodes)=counter;

    return counter;

}

void Network::nodecounthelper(NNodePtr curnode, S32 &counter,
                              vector<NNodePtr> &seenlist) const
{
    vector<LinkPtr> innodes=curnode->incoming;
    vector<LinkPtr>::iterator curlink;
    vector<NNodePtr>::iterator location;

    if (!((curnode->type)==SENSOR))
    {
        for (curlink=innodes.begin(); curlink!=innodes.end(); ++curlink)
        {
            location=find(seenlist.begin(), seenlist.end(), ((*curlink)->get_in_node()));
            if (location==seenlist.end())
            {
                counter++;
                seenlist.push_back((*curlink)->get_in_node());
                nodecounthelper((*curlink)->get_in_node(), counter, seenlist);
            }
        }

    }

}

S32 Network::linkcount() const
{
    S32 counter=0;
    vector<NNodePtr>::const_iterator curnode;
    vector<NNodePtr> seenlist; //List of nodes not to doublecount

    for (curnode=outputs.begin(); curnode!=outputs.end(); ++curnode)
    {
        linkcounthelper((*curnode), counter, seenlist);
    }

    const_cast<S32&>(numlinks)=counter;

    return counter;

}

void Network::linkcounthelper(NNodePtr curnode, S32 &counter,
                              vector<NNodePtr> &seenlist) const
{
    vector<LinkPtr> inlinks=curnode->incoming;
    vector<LinkPtr>::iterator curlink;
    vector<NNodePtr>::iterator location;

    location=find(seenlist.begin(), seenlist.end(), curnode);
    if ((!((curnode->type)==SENSOR))&&(location==seenlist.end()))
    {
        seenlist.push_back(curnode);

        for (curlink=inlinks.begin(); curlink!=inlinks.end(); ++curlink)
        {
            counter++;
            linkcounthelper((*curlink)->get_in_node(), counter, seenlist);
        }

    }

}

// Destroy will find every node in the network and subsequently
// delete them one by one.  Since deleting a node deletes its incoming
// links, all nodes and links associated with a network will be destructed
// Note: Traits are parts of genomes and not networks, so they are not
//       deleted here
void Network::destroy()
{
    vector<NNodePtr>::iterator curnode;
    vector<NNodePtr>::iterator location;
    vector<NNodePtr> seenlist; //List of nodes not to doublecount

    // Erase all nodes from all_nodes list 
    all_nodes.clear();
}

void Network::destroy_helper(NNodePtr curnode, vector<NNodePtr> &seenlist)
{
    vector<LinkPtr> innodes=curnode->incoming;
    vector<LinkPtr>::iterator curlink;
    vector<NNodePtr>::iterator location;

    if (!((curnode->type)==SENSOR))
    {
        for (curlink=innodes.begin(); curlink!=innodes.end(); ++curlink)
        {
            location=find(seenlist.begin(), seenlist.end(), ((*curlink)->get_in_node()));
            if (location==seenlist.end())
            {
                seenlist.push_back((*curlink)->get_in_node());
                destroy_helper((*curlink)->get_in_node(), seenlist);
            }
        }

    }

}

// This checks a POTENTIAL link between a potential in_node and potential out_node to see if it must be recurrent 
bool Network::is_recur(NNodePtr potin_node, NNodePtr potout_node, S32 &count,
                       S32 thresh) const
{
    vector<LinkPtr>::const_iterator curlink;

    ++count; //Count the node as visited

    if (count>thresh)
    {
        //cout<<"returning false"<<endl;
        return false; //Short out the whole thing- loop detected
    }

    if (potin_node==potout_node)
        return true;
    else
    {
        //Check back on all links...
        for (curlink=(potin_node->incoming).begin(); curlink!=(potin_node->incoming).end(); curlink++)
        {
            //But skip links that are already recurrent
            //(We want to check back through the forward flow of signals only
            if (!((*curlink)->is_recurrent))
            {
                if (is_recur((*curlink)->get_in_node(), potout_node, count, thresh))
                    return true;
            }
        }
        return false;
    }
}

S32 Network::input_start()
{
    input_iter=inputs.begin();
    return 1;
}

S32 Network::load_in(F64 d)
{
    (*input_iter)->sensor_load(d);
    input_iter++;
    if (input_iter==inputs.end())
        return 0;
    else
        return 1;
}
