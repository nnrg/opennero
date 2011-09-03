#include "core/Common.h"
#include "gene.h"
#include "XMLSerializable.h"
#include <sstream>

using namespace NEAT;
using namespace std;

Gene::Gene(double w, NNodePtr inode, NNodePtr onode, bool recur, double innov,
           double mnum) :
    lnk(new Link(w, inode, onode, recur)), innovation_num(innov), mutation_num(mnum),
        enable(true), frozen(false)
{
}

Gene::Gene(TraitPtr tp, double w, const NNodePtr inode, const NNodePtr onode,
           bool recur, double innov, double mnum) :
    lnk(new Link(tp,w,inode,onode,recur)), innovation_num(innov), mutation_num(mnum),
        enable(true), frozen(false)
{
}

Gene::Gene(GenePtr g, TraitPtr tp, const NNodePtr inode, const NNodePtr onode) :
    lnk(new Link(tp,(g->lnk)->weight,inode,onode,(g->lnk)->is_recurrent)), innovation_num(g->innovation_num),
        mutation_num(g->mutation_num), enable(g->enable), frozen(g->frozen)
{
}

Gene::Gene(istream &args, vector<TraitPtr> &traits, vector<NNodePtr> &nodes)
{
    //Gene parameter holders
    S32 traitnum;
    S32 inodenum;
    S32 onodenum;
    NNodePtr inode;
    NNodePtr onode;
    F64 weight;
    S32 recur;
    TraitPtr traitptr;

    vector<TraitPtr>::iterator curtrait;
    vector<NNodePtr>::iterator curnode;

    //Get the gene parameters

    // gene 1 1 22 0.000000 0 1.000000 0.000000 1
    args >> traitnum;
    args >> inodenum;
    args >> onodenum;
    args >> weight;
    args >> recur;
    args >> innovation_num;
    args >> mutation_num;
    args >> enable;

    frozen=false; //TODO: MAYBE CHANGE

    //Get a pointer to the linktrait
    if (traitnum==0)
        traitptr.reset();
    else
    {
        curtrait=traits.begin();
        while (((*curtrait)->trait_id)!=traitnum)
            ++curtrait;
        traitptr=(*curtrait);
    }

    //Get a pointer to the input node
    curnode=nodes.begin();
    while (curnode != nodes.end() && ((*curnode)->node_id)!=inodenum)
        ++curnode;

    if (curnode != nodes.end())
        inode=(*curnode);

    //Get a pointer to the output node
    curnode=nodes.begin();
    while (((*curnode)->node_id)!=onodenum)
        ++curnode;
    onode=(*curnode);

    lnk.reset(new Link(traitptr,weight,inode,onode,recur != 0));

}

// new Gene(0,0,0,0,0,0,0)
Gene::Gene() :
  lnk(new Link(TraitPtr(), 0, NNodePtr(), NNodePtr(), false)), 
  innovation_num(0), mutation_num(0), enable(true), frozen(false)
{
}

Gene::Gene(const Gene& gene)
{
    innovation_num = gene.innovation_num;
    mutation_num = gene.mutation_num;
    enable = gene.enable;
    frozen = gene.frozen;

    lnk.reset(new Link(*gene.lnk));
}

Gene::~Gene()
{
}

void Gene::print_to_file(std::ofstream &outFile)
{
    outFile<<"gene ";

    //Start off with the trait number for this gene
    if (!lnk->linktrait)
    {
        outFile<<"0 ";
    }
    else
    {
        outFile<<((lnk->linktrait)->trait_id)<<" ";
    }
    outFile<<(lnk->get_in_node())->node_id<<" ";
    outFile<<(lnk->get_out_node())->node_id<<" ";
    outFile<<(lnk->weight)<<" ";
    outFile<<(lnk->is_recurrent)<<" ";
    outFile<<innovation_num<<" ";
    outFile<<mutation_num<<" ";
    outFile<<enable<<endl;
}
