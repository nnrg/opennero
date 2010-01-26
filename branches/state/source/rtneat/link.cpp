#include "core/Common.h"
#include "link.h"
#include <iostream>

using namespace NEAT;
using namespace std;

Link::Link(F64 w, NNodePtr inode, NNodePtr onode, bool recur) :
    in_node(inode), 
    out_node(onode), 
    weight(w), 
    is_recurrent(recur),
    time_delay(false), 
    trait_id(1), 
    linktrait(), 
    added_weight(0)
{
}

Link::Link(TraitPtr lt, F64 w, NNodePtr inode, NNodePtr onode, bool recur) :
    in_node(inode), 
    out_node(onode), 
    weight(w), 
    is_recurrent(recur),
    time_delay(false), 
    trait_id(1), 
    linktrait(lt), 
    added_weight(0)
{
    if (lt.get()!=0)
        trait_id=lt->trait_id;
}

Link::Link(F64 w) :
    in_node(), 
    out_node(), 
    weight(w), 
    is_recurrent(false), 
    time_delay(false), 
    trait_id(1),
    linktrait(),
    added_weight(0)
{
}

Link::Link(const Link& link) :
    in_node(link.in_node), 
    out_node(link.out_node), 
    weight(link.weight), 
    is_recurrent(link.is_recurrent), 
    time_delay(link.time_delay),
    trait_id(link.trait_id),
    linktrait(link.linktrait), 
    added_weight(link.added_weight)
{
}

void Link::derive_trait(TraitPtr curtrait)
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

