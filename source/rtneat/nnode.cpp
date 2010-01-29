#include "core/Common.h"
#include <vector>
#include <sstream>

#include "nnode.h"

using namespace NEAT;
using namespace std;

NNode::NNode(nodetype ntype, S32 nodeid) :
    active_flag(false), 
    activesum(0), 
    activation(0), 
    output(), 
    incoming(),
    outgoing(),
    last_activation(0),
    last_activation2(0),
    gradient_flag(false), 
    gradientsum(0), 
    gradient(0), 
    gradient_count(0), 
    type(ntype), //NEURON or SENSOR type
    activation_count(0), //Inactive upon creation
    node_id(nodeid), 
    ftype(SIGMOID), 
    nodetrait(), 
    gen_node_label(HIDDEN),
    dup(), 
    analogue(), 
    override(false),
    override_value(0), 
    _sensorName(), 
    _sensorArgs(),
    frozen(false), 
    trait_id(1)
{
}

NNode::NNode(nodetype ntype, S32 nodeid, nodeplace placement) :
    active_flag(false), 
    activesum(0), 
    activation(0), 
    output(), 
    incoming(),
    outgoing(),
    last_activation(0),
    last_activation2(0),
    gradient_flag(false), 
    gradientsum(0), 
    gradient(0), 
    gradient_count(0), //Inactive upon creation
    type(ntype), //NEURON or SENSOR type
    activation_count(0), //Inactive upon creation
    node_id(nodeid), 
    ftype(SIGMOID), 
    nodetrait(),
    gen_node_label(placement), 
    dup(), 
    analogue(), 
    override(false), 
    _sensorName(), 
    _sensorArgs(),
    frozen(false),
    trait_id(1) 
{
}

NNode::NNode(nodetype ntype, S32 nodeid, nodeplace placement, functype function) :
    active_flag(false), 
    activesum(0), 
    activation(0), 
    output(), 
    incoming(),
    outgoing(),
    last_activation(0),
    last_activation2(0),
    gradient_flag(false), 
    gradientsum(0), 
    gradient(0), 
    gradient_count(0), //Inactive upon creation
    type(ntype), //NEURON or SENSOR type
    activation_count(0), //Inactive upon creation
    node_id(nodeid), 
    ftype(function), 
    nodetrait(),
    gen_node_label(placement), 
    dup(), 
    analogue(), 
    override(false), 
    _sensorName(), 
    _sensorArgs(),
    frozen(false),
    trait_id(1) 
{
}

NNode::NNode(NNodePtr n, TraitPtr t) :
    active_flag(false), 
    activesum(0), 
    activation(0), 
    output(0), 
    incoming(), 
    outgoing(), 
    last_activation(0), 
    last_activation2(0), 
    gradient_flag(false), 
    gradientsum(0), 
    gradient(0), 
    gradient_count(0),    
    type(n->type),
    activation_count(0), 
    node_id(n->node_id), 
    ftype(n->ftype),
    nodetrait(t),
    gen_node_label(n->gen_node_label), 
    dup(), 
    analogue(), 
    override(false), 
    _sensorName(n->_sensorName),
    _sensorArgs(n->_sensorArgs),
    frozen(false), 
    trait_id(t.get() == 0 ? 1 : t->trait_id)
{
}

NNode::NNode(istream &args, vector<TraitPtr> &traits) :
    active_flag(false), 
    activesum(0), 
    activation(0), 
    output(0), 
    incoming(), 
    outgoing(), 
    last_activation(0), 
    last_activation2(0), 
    gradient_flag(false), 
    gradientsum(0), 
    gradient(0), 
    gradient_count(0), //Inactive upon creation
    activation_count(0), //Inactive upon creation
    nodetrait(), 
    dup(), 
    analogue(), 
    override(false), 
    _sensorName(), 
    _sensorArgs(),
    frozen(false),
    trait_id(1)
{
    S32 traitnum;
    vector<TraitPtr>::iterator curtrait;

    activesum=0;
    gradientsum=0;

    //Get the node parameters
    args >> node_id;
    args >> traitnum;
    int x;
    args >> x;
    type = (nodetype)x;
    args >> x;
    gen_node_label = (nodeplace)x;
    args >> x;
    ftype = (functype)x;
    // Get the Sensor Name and Parameter String if the node is an input node
    if (gen_node_label == INPUT)
    {
        args >> _sensorName;
        getline(args, _sensorArgs);
    }

    frozen=false; //TODO: Maybe change

    //Get a pointer to the trait this node points to
    if (traitnum==0)
        nodetrait.reset();
    else
    {
        curtrait=traits.begin();
        while (((*curtrait)->trait_id)!=traitnum)
            ++curtrait;
        nodetrait=(*curtrait);
        trait_id=nodetrait->trait_id;
    }

    override=false;
}

// This one might be incomplete
NNode::NNode(const NNode& nnode)
{
    active_flag = nnode.active_flag;
    activesum = nnode.activesum;
    activation = nnode.activation;
    gradient_flag = nnode.gradient_flag;
    gradientsum = nnode.gradientsum;
    gradient = nnode.gradient;
    output = nnode.output;
    last_activation = nnode.last_activation;
    last_activation2 = nnode.last_activation2;
    type = nnode.type; //NEURON or SENSOR type
    activation_count = nnode.activation_count; //Inactive upon creation
    gradient_count = nnode.gradient_count; //Inactive upon creation
    node_id = nnode.node_id;
    ftype = nnode.ftype;
    nodetrait = nnode.nodetrait;
    gen_node_label = nnode.gen_node_label;
    dup = nnode.dup;
    analogue = nnode.dup;
    frozen = nnode.frozen;
    trait_id = nnode.trait_id;
    override = nnode.override;

    _sensorName = nnode._sensorName;
    _sensorArgs = nnode._sensorArgs;
}

NNode::~NNode()
{
}

//Returns the type of the node, NEURON or SENSOR
const nodetype NNode::get_type()
{
    return type;
}

//Allows alteration between NEURON and SENSOR.  Returns its argument
nodetype NNode::set_type(nodetype newtype)
{
    type=newtype;
    return newtype;
}

//If the node is a SENSOR, returns true and loads the value
bool NNode::sensor_load(F64 value)
{
    if (type==SENSOR)
    {

        //Time delay memory
        last_activation2=last_activation;
        last_activation=activation;

        activation_count++; //Puts sensor into next time-step
        activation=value;
        return true;
    }
    else
        return false;
}

//Loads the error for backprop and computes local gradient from it
void NNode::error_load(F64 value)
{
    F64 derivative = activation*(1 - activation);  // assuming sigmoid slope is 1
    gradient = value*derivative;
    gradient_count++;
}

// Note: NEAT keeps track of which links are recurrent and which
// are not even though this is unnecessary for activation.
// It is useful to do so for 2 other reasons: 
// 1. It makes networks visualization of recurrent networks possible
// 2. It allows genetic control of the proportion of connections
//    that may become recurrent

// Add an incoming connection a node
void NNode::add_incoming(NNodePtr feednode, F64 weight, bool recur)
{
    LinkPtr newlink(new Link(weight,feednode,shared_from_this(),recur));
    incoming.push_back(newlink);
    feednode->outgoing.push_back(newlink);
}

// Nonrecurrent version
void NNode::add_incoming(NNodePtr feednode, F64 weight)
{
    LinkPtr newlink(new Link(weight,feednode,shared_from_this(),false));
    incoming.push_back(newlink);
    feednode->outgoing.push_back(newlink);
}

// Return activation currently in node, if it has been activated
F64 NNode::get_active_out()
{
    if (activation_count>0)
        return activation;
    else
        return 0.0;
}

// Return activation currently in node from PREVIOUS (time-delayed) time step,
// if there is one
F64 NNode::get_active_out_td()
{
    if (activation_count>1)
        return last_activation;
    else
        return 0.0;
}

// Return gradient currently in node, if it has been activated
F64 NNode::get_gradient_out()
{
    if (gradient_count>0)
        return gradient;
    else
        return 0.0;
}

// This recursively flushes everything leading into and including this NNode, including recurrencies
void NNode::flushback()
{
    vector<LinkPtr>::iterator curlink;

    //A sensor should not flush black
    if (type!=SENSOR)
    {

        if (activation_count>0)
        {
            activation_count=0;
            activation=0;
            last_activation=0;
            last_activation2=0;
            gradient_count=0;
            gradient=0;
        }

        //Flush back recursively
        for (curlink=incoming.begin(); curlink!=incoming.end(); ++curlink)
        {
            //Flush the link itself (For future learning parameters possibility) 
            (*curlink)->added_weight=0;
            if ((((*curlink)->get_in_node())->activation_count>0))
                ((*curlink)->get_in_node())->flushback();
        }
    }
    else
    {
        //Flush the SENSOR
        activation_count=0;
        activation=0;
        last_activation=0;
        last_activation2=0;
        gradient_count=0;
        gradient=0;
    }

}

// Reserved for future system expansion
void NNode::derive_trait(TraitPtr curtrait)
{

    if (curtrait!=0)
    {
        for (S32 count=0; count<NEAT::num_trait_params; count++)
            params[count]=(curtrait->params)[count];
    }
    else
    {
        for (S32 count=0; count<NEAT::num_trait_params; count++)
            params[count]=0;
    }

    if (curtrait!=0)
        trait_id=curtrait->trait_id;
    else
        trait_id=1;

}

// Returns the gene that created the node
NNodePtr NNode::get_analogue()
{
    return analogue;
}

// Force an output value on the node
void NNode::override_output(F64 new_output)
{
    override_value=new_output;
    override=true;
}

// Tell whether node has been overridden
bool NNode::overridden()
{
    return override;
}

// Set activation to the override value and turn off override
void NNode::activate_override()
{
    activation=override_value;
    override=false;
}

void NNode::print_to_file(std::ofstream &outFile)
{
    outFile<<"node "<<node_id<<" ";
    if (nodetrait!=0)
        outFile<<nodetrait->trait_id<<" ";
    else
        outFile<<"0 ";
    outFile<<type<<" ";
    outFile<<gen_node_label<<" ";
    outFile<<ftype<<endl;
}

const char* NEAT::getNodePlaceString(nodeplace place) {
    switch (place) {
    case (HIDDEN):
        return "hidden";
    case INPUT:
        return "input";
    case OUTPUT:
        return "output";
    case BIAS:
        return "bias";
    default:
        return "unknown";
    }
}
