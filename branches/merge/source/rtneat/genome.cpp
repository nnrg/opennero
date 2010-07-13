#include "core/Common.h"
#include "genome.h"
#include "innovation.h"
#include "gene.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include <boost/tokenizer.hpp>

using namespace NEAT;
using namespace std;

Genome::Genome(S32 id, vector<TraitPtr> t, vector<NNodePtr> n, vector<GenePtr> g)
    : genome_id(id)
    , traits(t)
    , nodes(n)
    , genes(g)
{
}

Genome::Genome(S32 id, vector<TraitPtr> t, vector<NNodePtr> n, vector<LinkPtr> links)
    : genome_id(id)
    , traits(t)
    , nodes(n)
{
    vector<LinkPtr>::iterator curlink;

    //We go through the links and turn them into original genes
    for (curlink=links.begin(); curlink!=links.end(); ++curlink)
    {
        //Create genes one at a time
        GenePtr tempgene(new Gene((*curlink)->linktrait, (*curlink)->weight,(*curlink)->get_in_node(),(*curlink)->get_out_node(),(*curlink)->is_recurrent,1.0,0.0));
        genes.push_back(tempgene);
    }

}

Genome::Genome(const Genome& genome)
    : genome_id(genome.genome_id)
{
    vector<TraitPtr>::const_iterator curtrait;
    vector<NNodePtr>::const_iterator curnode;
    vector<GenePtr>::const_iterator curgene;

    for (curtrait=genome.traits.begin(); curtrait!=genome.traits.end(); ++curtrait)
    {
        TraitPtr p(new Trait(**curtrait));
        traits.push_back(p);
    }

    TraitPtr assoc_trait;
    //Duplicate NNodes
    for (curnode=genome.nodes.begin(); curnode!=genome.nodes.end(); ++curnode)
    {
        //First, find the trait that this node points to
        if (!(*curnode)->nodetrait)
            assoc_trait.reset();
        else
        {
            curtrait=traits.begin();
            while (((*curtrait)->trait_id)!=(((*curnode)->nodetrait)->trait_id))
                ++curtrait;
            assoc_trait=(*curtrait);
        }

        NNodePtr newnode(new NNode(*curnode,assoc_trait));

        (*curnode)->dup=newnode; //Remember this node's old copy
        //    (*curnode)->activation_count=55;
        nodes.push_back(newnode);
    }

    NNodePtr inode; //For forming a gene 
    NNodePtr onode; //For forming a gene

    //Duplicate Genes
    for (curgene=genome.genes.begin(); curgene!=genome.genes.end(); ++curgene)
    {
        //First find the nodes connected by the gene's link

        inode=(((*curgene)->lnk)->get_in_node())->dup;
        onode=(((*curgene)->lnk)->get_out_node())->dup;

        //Get a pointer to the trait expressed by this gene
        TraitPtr traitptr(((*curgene)->lnk)->linktrait);
        if (traitptr.get() == 0)
            assoc_trait.reset();
        else
        {
            curtrait=traits.begin();
            while (((*curtrait)->trait_id)!=(traitptr->trait_id))
                ++curtrait;
            assoc_trait=(*curtrait);
        }

        GenePtr newgene(new Gene(*curgene,assoc_trait,inode,onode));
        genes.push_back(newgene);

    }
}

Genome::Genome(S32 id, std::ifstream &iFile)
    : genome_id(id)
{
    string curword; //max word size of 128 characters
    string curline; //max line size of 1024 characters

    bool done=false;
    
    //Loop until file is finished, parsing each line
    while (!done && iFile)
    {
        curline = "";
        getline(iFile, curline);
        std::istringstream line(curline);
        line >> curword;
        //Check for end of Genome
        if (curword == "genomeend")
        {
            S32 idcheck;
            line >> idcheck;
            if (idcheck != genome_id) {
                cerr << "ERROR: id mismatch in genome " << genome_id << " expected, found " << idcheck << endl;
                cerr << "line was: " << line.str() << endl;
                cerr << "id check was: " << idcheck << endl;
                throw std::runtime_error("ID mismatch in genome");
            }
            done=true;
        }
        //Ignore comments surrounded by - they get printed to screen
        else if (curword == "/*")
        {
            while (curword.find("*/") != string::npos)
                line >> curword;
        }
        //Read in a trait
        else if (curword == "trait")
        {
            TraitPtr newtrait(new Trait(line));
            //Add trait to vector of traits
            traits.push_back(newtrait);
        }
        //Read in a node
        else if (curword == "node")
        {
            NNodePtr newnode(new NNode(line,traits));
            //Add the node to the list of nodes
            nodes.push_back(newnode);
        }
        //Read in a Gene
        else if (curword == "gene")
        {
            //Allocate the new Gene
            GenePtr newgene(new Gene(line,traits,nodes));

            //Add the gene to the genome
            genes.push_back(newgene);
        }
    }
    assert(nodes.size() > 0);
}

Genome::Genome(S32 new_id, S32 i, S32 o, S32 n, S32 nmax, bool r, F64 linkprob)
    : genome_id(new_id)
{
    int totalnodes;
    bool *cm; //The connection matrix which will be randomized
    bool *cmp; //Connection matrix pointer
    int matrixdim;
    int count;

    int ncount; //Node and connection counters
    int ccount;

    int row; //For navigating the matrix
    int col;

    double new_weight;

    int maxnode; //No nodes above this number for this genome

    int first_output; //Number of first output node

    totalnodes=i+o+nmax;
    matrixdim=totalnodes*totalnodes;
    cm=new bool[matrixdim]; //Dimension the connection matrix
    maxnode=i+n;

    first_output=totalnodes-o+1;

    //For creating the new genes
    GenePtr newgene;
    NNodePtr in_node;
    NNodePtr out_node;

    //Retrieves the nodes pointed to by connection genes
    vector<NNodePtr>::iterator node_iter;

    //Step through the connection matrix, randomly assigning bits
    cmp=cm;
    for (count=0; count<matrixdim; count++)
    {
        if (randfloat()<linkprob)
            *cmp=true;
        else
            *cmp=false;
        cmp++;
    }

    //Create a dummy trait (this is for future expansion of the system)
    TraitPtr newtrait(new Trait(1,0,0,0,0,0,0,0,0,0));
    traits.push_back(newtrait);

    //Build the input nodes
    for (ncount=1; ncount<=i; ncount++)
    {
        NNodePtr newnode;
        if (ncount<i)
            newnode.reset(new NNode(SENSOR,ncount,INPUT));
        else
            newnode.reset(new NNode(SENSOR,ncount,BIAS));

        newnode->nodetrait=newtrait;

        //Add the node to the list of nodes
        nodes.push_back(newnode);
    }

    //Build the hidden nodes
    for (ncount=i+1; ncount<=i+n; ncount++)
    {
        NNodePtr newnode(new NNode(NEURON,ncount,HIDDEN));
        newnode->nodetrait=newtrait;
        //Add the node to the list of nodes
        nodes.push_back(newnode);
    }

    //Build the output nodes
    for (ncount=first_output; ncount<=totalnodes; ncount++)
    {
        NNodePtr newnode(new NNode(NEURON,ncount,OUTPUT));
        newnode->nodetrait=newtrait;
        //Add the node to the list of nodes
        nodes.push_back(newnode);
    }

    //cout<<"Built nodes"<<endl;

    //Connect the nodes 
    ccount=1; //Start the connection counter

    //Step through the connection matrix, creating connection genes
    cmp=cm;
    count=0;
    for (col=1; col<=totalnodes; col++)
        for (row=1; row<=totalnodes; row++)
        {
            //Only try to create a link if it is in the matrix
            //and not leading into a sensor

            if ((*cmp==true)&&(col>i)&&((col<=maxnode)||(col>=first_output))
                &&((row<=maxnode)||(row>=first_output)))
            {
                //If it isn't recurrent, create the connection no matter what
                if (col>row)
                {

                    //Retrieve the in_node
                    node_iter=nodes.begin();
                    while ((*node_iter)->node_id!=row)
                        node_iter++;

                    in_node=(*node_iter);

                    //Retrieve the out_node
                    node_iter=nodes.begin();
                    while ((*node_iter)->node_id!=col)
                        node_iter++;

                    out_node=(*node_iter);

                    //Create the gene
                    new_weight=randposneg()*randfloat();
                    GenePtr
                        newgene(new Gene(newtrait,new_weight, in_node, out_node, false, count, new_weight));

                    //Add the gene to the genome
                    genes.push_back(newgene);
                }
                else if (r)
                {
                    //Create a recurrent connection

                    //Retrieve the in_node
                    node_iter=nodes.begin();
                    while ((*node_iter)->node_id!=row)
                        node_iter++;

                    in_node=(*node_iter);

                    //Retrieve the out_node
                    node_iter=nodes.begin();
                    while ((*node_iter)->node_id!=col)
                        node_iter++;

                    out_node=(*node_iter);

                    //Create the gene
                    new_weight=randposneg()*randfloat();
                    GenePtr
                        newgene(new Gene(newtrait,new_weight, in_node, out_node, true,count,new_weight));

                    //Add the gene to the genome
                    genes.push_back(newgene);

                }

            }

            count++; //increment gene counter	    
            cmp++;
        }

    delete [] cm;

}

Genome::Genome(S32 num_in, S32 num_out, S32 num_hidden, S32 type)
    : genome_id(0)
{

    //Temporary lists of nodes
    vector<NNodePtr> inputs;
    vector<NNodePtr> outputs;
    vector<NNodePtr> hidden;
    NNodePtr bias; //Remember the bias

    vector<NNodePtr>::iterator curnode1; //Node iterator1
    vector<NNodePtr>::iterator curnode2; //Node iterator2
    vector<NNodePtr>::iterator curnode3; //Node iterator3

    //For creating the new genes
    NNodePtr newnode;

    int count;
    int ncount;

    //Create a dummy trait (this is for future expansion of the system)
    TraitPtr newtrait(new Trait(1,0,0,0,0,0,0,0,0,0));
    traits.push_back(newtrait);

    //Adjust hidden number
    if (type==0)
        num_hidden=0;
    else if (type==1)
        num_hidden=num_in*num_out;

    //Create the inputs and outputs

    //Build the input nodes
    for (ncount=1; ncount<=num_in; ncount++)
    {
        NNodePtr newnode;
        if (ncount<num_in)
            newnode.reset(new NNode(SENSOR,ncount,INPUT));
        else
        {
            newnode.reset(new NNode(SENSOR,ncount,BIAS));
            bias=newnode;
        }

        //newnode->nodetrait=newtrait;

        //Add the node to the list of nodes
        nodes.push_back(newnode);
        inputs.push_back(newnode);
    }

    //Build the hidden nodes
    for (ncount=num_in+1; ncount<=num_in+num_hidden; ncount++)
    {
        NNodePtr newnode(new NNode(NEURON,ncount,HIDDEN));
        //Add the node to the list of nodes
        nodes.push_back(newnode);
        hidden.push_back(newnode);
    }

    //Build the output nodes
    for (ncount=num_in+num_hidden+1; ncount<=num_in+num_hidden+num_out; ncount++)
    {
        NNodePtr newnode(new NNode(NEURON,ncount,OUTPUT));
        //Add the node to the list of nodes
        nodes.push_back(newnode);
        outputs.push_back(newnode);
    }

    //Create the links depending on the type
    if (type==0)
    {
        //Just connect inputs straight to outputs

        count=1;

        //Loop over the outputs
        for (curnode1=outputs.begin(); curnode1!=outputs.end(); ++curnode1)
        {
            //Loop over the inputs
            for (curnode2=inputs.begin(); curnode2!=inputs.end(); ++curnode2)
            {
                //Connect each input to each output
                GenePtr newgene(new Gene(newtrait,0, *curnode2, *curnode1, false, count,0));

                //Add the gene to the genome
                genes.push_back(newgene);

                count++;

            }

        }

    } //end type 0
    //A split link from each input to each output
    else if (type==1)
    {
        count=1; //Start the gene number counter

        curnode3=hidden.begin(); //One hidden for ever input-output pair
        //Loop over the outputs
        for (curnode1=outputs.begin(); curnode1!=outputs.end(); ++curnode1)
        {
            //Loop over the inputs
            for (curnode2=inputs.begin(); curnode2!=inputs.end(); ++curnode2)
            {

                //Connect Input to hidden
                GenePtr newgene(new Gene(newtrait,0, *curnode2, *curnode1, false,count,0));
                //Add the gene to the genome
                genes.push_back(newgene);

                count++; //Next gene

                //Connect hidden to output
                newgene.reset(new Gene(newtrait,0, *curnode3, *curnode1, false,count,0));
                //Add the gene to the genome
                genes.push_back(newgene);

                ++curnode3; //Next hidden node
                count++; //Next gene

            }
        }

    }//end type 1
    //Fully connected 
    else if (type==2)
    {
        count=1; //Start gene counter at 1


        //Connect all inputs to all hidden nodes
        for (curnode1=hidden.begin(); curnode1!=hidden.end(); ++curnode1)
        {
            //Loop over the inputs
            for (curnode2=inputs.begin(); curnode2!=inputs.end(); ++curnode2)
            {
                //Connect each input to each hidden
                GenePtr newgene(new Gene(newtrait,0, *curnode2, *curnode1,false,count,0));

                //Add the gene to the genome
                genes.push_back(newgene);

                count++;

            }
        }

        //Connect all hidden units to all outputs
        for (curnode1=outputs.begin(); curnode1!=outputs.end(); ++curnode1)
        {
            //Loop over the inputs
            for (curnode2=hidden.begin(); curnode2!=hidden.end(); ++curnode2)
            {
                //Connect each input to each hidden
                GenePtr newgene(new Gene(newtrait,0, *curnode2, *curnode1,false,count,0));

                //Add the gene to the genome
                genes.push_back(newgene);

                count++;

            }
        }

        //Connect the bias to all outputs
        for (curnode1=outputs.begin(); curnode1!=outputs.end(); ++curnode1)
        {
            GenePtr newgene(new Gene(newtrait,0, bias, *curnode1,false,count,0));

            //Add the gene to the genome
            genes.push_back(newgene);

            count++;
        }

        //Recurrently connect the hidden nodes
        for (curnode1=hidden.begin(); curnode1!=hidden.end(); ++curnode1)
        {
            //Loop Over all Hidden
            for (curnode2=hidden.begin(); curnode2!=hidden.end(); ++curnode2)
            {
                //Connect each hidden to each hidden
                GenePtr newgene(new Gene(newtrait,0, *curnode2, *curnode1,true,count,0));

                //Add the gene to the genome
                genes.push_back(newgene);

                count++;

            }

        }

    }//end type 2

}

GenomePtr Genome::new_Genome_load(const std::string& filename)
{
    S32 id;

    string curword;

    ifstream iFile(filename.c_str());

    //Make sure it worked
    if (!iFile)
    {
        cerr<<"Can't open "<<filename<<" for input"<<endl;
        return GenomePtr();
    }

    iFile>>curword;

    //Bypass initial comment
    if (curword == "/*")
    {
        iFile>>curword;
        while (curword.find("*/") != string::npos)
        {
            iFile>>curword;
        }

        iFile>>curword;
    }

    iFile>>id;

    GenomePtr newgenome(new Genome(id,iFile));

    iFile.close();

    return newgenome;
}

Genome::~Genome()
{
    traits.clear();
    nodes.clear();
    genes.clear();
}

NetworkPtr Genome::genesis(S32 id)
{
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;
    TraitPtr curtrait;
    LinkPtr curlink;
    LinkPtr newlink;

    F64 maxweight=0.0; //Compute the maximum weight for adaptation purposes
    F64 weight_mag; //Measures absolute value of weights

    //Inputs and outputs will be collected here for the network
    //All nodes are collected in an all_list- 
    //this will be used for later safe destruction of the net
    vector<NNodePtr> inlist;
    vector<NNodePtr> outlist;
    vector<NNodePtr> all_list;

    //Gene translation variables
    NNodePtr inode;
    NNodePtr onode;

    //The new network
    NetworkPtr newnet;

    //Create the nodes
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        NNodePtr newnode(new NNode((*curnode)->type,(*curnode)->node_id,(*curnode)->gen_node_label,(*curnode)->ftype));

        //Derive the node parameters from the trait pointed to
        curtrait=(*curnode)->nodetrait;
        newnode->derive_trait(curtrait);

        //Check for input or output designation of node
        if (((*curnode)->gen_node_label)==INPUT)
            inlist.push_back(newnode);
        if (((*curnode)->gen_node_label)==BIAS)
            inlist.push_back(newnode);
        if (((*curnode)->gen_node_label)==OUTPUT)
            outlist.push_back(newnode);

        //Keep track of all nodes, not just input and output
        all_list.push_back(newnode);

        //Have the node specifier point to the node it generated
        (*curnode)->analogue=newnode;

    }

    //Create the links by iterating through the genes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        //Only create the link if the gene is enabled
        if (((*curgene)->enable)==true)
        {
            curlink=(*curgene)->lnk;
            inode=curlink->get_in_node()->analogue;
            onode=curlink->get_out_node()->analogue;
            //NOTE: This line could be run through a recurrency check if desired
            // (no need to in the current implementation of NEAT)
            newlink.reset(new Link(curlink->weight,inode,onode,curlink->is_recurrent));

            (onode->incoming).push_back(newlink);
            (inode->outgoing).push_back(newlink);

            //Derive link's parameters from its Trait pointer
            curtrait=(curlink->linktrait);

            newlink->derive_trait(curtrait);

            //Keep track of maximum weight
            if (newlink->weight>0)
                weight_mag=newlink->weight;
            else
                weight_mag=-newlink->weight;
            if (weight_mag>maxweight)
                maxweight=weight_mag;
        }
    }

    //Create the new network
    newnet.reset(new Network(inlist,outlist,all_list,id));

    //Attach genotype and phenotype together
    newnet->genotype = shared_from_this();
    phenotype=newnet;

    newnet->maxweight=maxweight;

    return newnet;

}

void Genome::Lamarck()
{
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;
    LinkPtr curlink;
    LinkPtr netlink;

    //Gene translation variables
    NNodePtr inode;
    NNodePtr onode;

    //Reset the linkcount variable in the nodes.  We will use this
    //variable in the next loop to access the links in the same
    //order they were created and stored in the network.
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        (*curnode)->analogue->linkcount = 0;
    }

    //Get the link weights by iterating through the genes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        //Only consider the link if the gene is enabled
        if (((*curgene)->enable)==true)
        {
            curlink=(*curgene)->lnk;
            onode=curlink->get_out_node()->analogue;
            netlink = onode->incoming[onode->linkcount++];
            curlink->weight = netlink->weight;
        }
    }
}

bool Genome::verify()
{
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;
    vector<GenePtr>::iterator curgene2;
    NNodePtr inode;
    NNodePtr onode;

    bool disab;

    S32 last_id;

    //Check each gene's nodes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        inode=((*curgene)->lnk)->get_in_node();
        onode=((*curgene)->lnk)->get_out_node();

        //Look for inode
        curnode=nodes.begin();
        while ((curnode!=nodes.end())&&((*curnode)!=inode))
            ++curnode;

        if (curnode==nodes.end())
        {
            //cout<<"MISSING iNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
            //cin>>pause;
            return false;
        }

        //Look for onode
        curnode=nodes.begin();
        while ((curnode!=nodes.end())&&((*curnode)!=onode))
            ++curnode;

        if (curnode==nodes.end())
        {
            //cout<<"MISSING oNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
            //cin>>pause;
            return false;
        }

    }

    //Check for NNodes being out of order
    last_id=0;
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        if ((*curnode)->node_id<last_id)
        {
            //cout<<"ALERT: NODES OUT OF ORDER in "<<this<<endl;
            //cin>>pause;
            return false;
        }

        last_id=(*curnode)->node_id;
    }

    //Make sure there are no duplicate genes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {

        for (curgene2=genes.begin(); curgene2!=genes.end(); ++curgene2)
        {
            if (((*curgene)!=(*curgene2))&&((((*curgene)->lnk)->is_recurrent)==(((*curgene2)->lnk)->is_recurrent))&&((((((*curgene2)->lnk)->get_in_node())->node_id)==((((*curgene)->lnk)->get_in_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((*curgene)->lnk)->get_out_node())->node_id))))
            {
                //cout<<"ALERT: DUPLICATE GENES: "<<(*curgene)<<(*curgene2)<<endl;
                //cout<<"INSIDE GENOME: "<<this<<endl;

                //cin>>pause;
            }

        }
    }

    //Check for 2 disables in a row
    //Note:  Again, this is not necessarily a bad sign
    if (nodes.size()>=500)
    {
        disab=false;
        for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
        {
            if ((((*curgene)->enable)==false)&&(disab==true))
            {
                //cout<<"ALERT: 2 DISABLES IN A ROW: "<<this<<endl;
            }
            if (((*curgene)->enable)==false)
                disab=true;
            else
                disab=false;
        }
    }

    //cout<<"GENOME OK!"<<endl;

    return true;
}

void Genome::print_to_file(std::ofstream &outFile)
{
    vector<TraitPtr>::iterator curtrait;
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;

    outFile<<"genomestart "<<genome_id<<endl;

    //Output the traits
    for (curtrait=traits.begin(); curtrait!=traits.end(); ++curtrait)
    {
        (*curtrait)->trait_id=static_cast<int>(curtrait-traits.begin()+1);
        (*curtrait)->print_to_file(outFile);
    }

    //Output the nodes
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        (*curnode)->print_to_file(outFile);
    }

    //Output the genes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        (*curgene)->print_to_file(outFile);
    }

    outFile<<"genomeend "<<genome_id<<endl;
}

void Genome::print_to_filename(const std::string& filename)
{
    ofstream oFile(filename.c_str());
    print_to_file(oFile);
    oFile.close();
}

S32 Genome::get_last_node_id()
{
    assert(nodes.size() > 0);
    assert(*(nodes.end() - 1));
    return ((*(nodes.end() - 1))->node_id)+1;
}

F64 Genome::get_last_gene_innovnum()
{
    assert(nodes.size() > 0);
    return ((*(genes.end() - 1))->innovation_num)+1;
}

GenomePtr Genome::duplicate(S32 new_id)
{
    //Collections for the new Genome
    vector<TraitPtr> traits_dup;
    vector<NNodePtr> nodes_dup;
    vector<GenePtr> genes_dup;

    //Iterators for the old Genome
    vector<TraitPtr>::iterator curtrait;
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;

    TraitPtr assoc_trait; //Trait associated with current item

    NNodePtr inode; //For forming a gene 
    NNodePtr onode; //For forming a gene
    TraitPtr traitptr;

    //Duplicate the traits
    for (curtrait=traits.begin(); curtrait!=traits.end(); ++curtrait)
    {
        TraitPtr newtrait(new Trait(*curtrait));
        traits_dup.push_back(newtrait);
    }

    //Duplicate NNodes
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        //First, find the trait that this node points to
        if (!(*curnode)->nodetrait)
            assoc_trait.reset();
        else
        {
            curtrait=traits_dup.begin();
            while (((*curtrait)->trait_id)!=(((*curnode)->nodetrait)->trait_id))
                ++curtrait;
            assoc_trait=(*curtrait);
        }

        NNodePtr newnode(new NNode(*curnode,assoc_trait));

        (*curnode)->dup=newnode; //Remember this node's old copy
        //    (*curnode)->activation_count=55;
        nodes_dup.push_back(newnode);
    }

    //Duplicate Genes
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        //First find the nodes connected by the gene's link

        inode=((*curgene)->lnk)->get_in_node()->dup;
        onode=((*curgene)->lnk)->get_out_node()->dup;

        //Get a pointer to the trait expressed by this gene
        traitptr=((*curgene)->lnk)->linktrait;
        if (!traitptr)
            assoc_trait.reset();
        else
        {
            curtrait=traits_dup.begin();
            while (((*curtrait)->trait_id)!=(traitptr->trait_id))
                ++curtrait;
            assoc_trait=(*curtrait);
        }

        GenePtr newgene(new Gene(*curgene,assoc_trait,inode,onode));
        genes_dup.push_back(newgene);

    }

    //Finally, return the genome
    GenomePtr newgenome(new Genome(new_id,traits_dup,nodes_dup,genes_dup));

    return newgenome;

}

void Genome::mutate_random_trait()
{
    vector<TraitPtr>::iterator thetrait; //Trait to be mutated
    S32 traitnum;

    //Choose a random traitnum
    traitnum=randint(0, static_cast<S32>(traits.size())-1);

    //Retrieve the trait and mutate it
    thetrait=traits.begin();
    (*(thetrait[traitnum])).mutate();

    //TRACK INNOVATION? (future possibility)

}

void Genome::mutate_link_trait(S32 times)
{
    S32 traitnum;
    S32 genenum;
    vector<GenePtr>::iterator thegene; //Link to be mutated
    vector<TraitPtr>::iterator thetrait; //Trait to be attached
    S32 count;
    S32 loop;

    for (loop=1; loop<=times; loop++)
    {

        //Choose a random traitnum
        traitnum=randint(0, static_cast<S32>(traits.size())-1);

        //Choose a random linknum
        genenum=randint(0, static_cast<S32>(genes.size())-1);

        //set the link to point to the new trait
        thegene=genes.begin();
        for (count=0; count<genenum; count++)
            ++thegene;

        //Do not alter frozen genes
        if (!((*thegene)->frozen))
        {
            thetrait=traits.begin();

            ((*thegene)->lnk)->linktrait=thetrait[traitnum];

        }
        //TRACK INNOVATION- future use
        //(*thegene)->mutation_num+=randposneg()*randfloat()*linktrait_mut_sig;

    }
}

void Genome::mutate_node_trait(S32 times)
{
    S32 traitnum;
    S32 nodenum;
    vector<NNodePtr>::iterator thenode; //Link to be mutated
    vector<GenePtr>::iterator thegene; //Gene to record innovation
    vector<TraitPtr>::iterator thetrait; //Trait to be attached
    S32 count;
    S32 loop;

    for (loop=1; loop<=times; loop++)
    {

        //Choose a random traitnum
        traitnum=randint(0, static_cast<S32>(traits.size())-1);

        //Choose a random nodenum
        nodenum=randint(0, static_cast<S32>(nodes.size())-1);

        //set the link to point to the new trait
        thenode=nodes.begin();
        for (count=0; count<nodenum; count++)
            ++thenode;

        //Do not mutate frozen nodes
        if (!((*thenode)->frozen))
        {

            thetrait=traits.begin();

            (*thenode)->nodetrait=thetrait[traitnum];

        }
        //TRACK INNOVATION! - possible future use
        //for any gene involving the mutated node, perturb that gene's
        //mutation number
        //for(thegene=genes.begin();thegene!=genes.end();++thegene) {
        //  if (((((*thegene)->lnk)->get_in_node())==(*thenode))
        //  ||
        //  ((((*thegene)->lnk)->get_out_node())==(*thenode)))
        //(*thegene)->mutation_num+=randposneg()*randfloat()*nodetrait_mut_sig;
        //}
    }
}

void Genome::mutate_link_weights(F64 power, F64 rate, mutator mut_type)
{
    vector<GenePtr>::iterator curgene;
    F64 num; //counts gene placement
    F64 gene_total;
    F64 powermod; //Modified power by gene number
    //The power of mutation will rise farther into the genome
    //on the theory that the older genes are more fit since
    //they have stood the test of time

    F64 randnum;
    F64 randchoice; //Decide what kind of mutation to do on a gene
    F64 endpart; //Signifies the last part of the genome
    F64 gausspoint;
    F64 coldgausspoint;

    bool severe; //Once in a while really shake things up

    // ------------------------------------------------------ 

    if (randfloat()>0.5)
        severe=true;
    else
        severe=false;

    //Go through all the Genes and perturb their link's weights
    num=0.0;
    gene_total=(double) static_cast<S32>(genes.size());
    endpart=gene_total*0.8;
    powermod=1.0;

    //Loop on all genes  (ORIGINAL METHOD)
    for (curgene=genes.begin(); curgene!=genes.end(); curgene++)
    {

        //The following if determines the probabilities of doing cold gaussian
        //mutation, meaning the probability of replacing a link weight with
        //another, entirely random weight.  It is meant to bias such mutations
        //to the tail of a genome, because that is where less time-tested genes
        //reside.  The gausspoint and coldgausspoint represent values above
        //which a random float will signify that kind of mutation.  

        //Don't mutate weights of frozen links
        if (!((*curgene)->frozen))
        {

            if (severe)
            {
                gausspoint=0.3;
                coldgausspoint=0.1;
            }
            else if ((gene_total>=10.0)&&(num>endpart))
            {
                gausspoint=0.5; //Mutate by modification % of connections
                coldgausspoint=0.3; //Mutate the rest by replacement % of the time
            }
            else
            {
                //Half the time don't do any cold mutations
                if (randfloat()>0.5)
                {
                    gausspoint=1.0-rate;
                    coldgausspoint=1.0-rate-0.1;
                }
                else
                {
                    gausspoint=1.0-rate;
                    coldgausspoint=1.0-rate;
                }
            }

            randnum=randposneg()*randfloat()*power*powermod;
            if (mut_type==GAUSSIAN)
            {
                randchoice=randfloat();
                if (randchoice>gausspoint)
                    ((*curgene)->lnk)->weight+=randnum;
                else if (randchoice>coldgausspoint)
                    ((*curgene)->lnk)->weight=randnum;
            }
            else if (mut_type==COLDGAUSSIAN)
                ((*curgene)->lnk)->weight=randnum;

            //Cap the weights at 20.0 (experimental)
            if (((*curgene)->lnk)->weight > 3.0)
                ((*curgene)->lnk)->weight = 3.0;
            else if (((*curgene)->lnk)->weight < -3.0)
                ((*curgene)->lnk)->weight = -3.0;

            //Record the innovation
            //(*curgene)->mutation_num+=randnum;
            (*curgene)->mutation_num=((*curgene)->lnk)->weight;

            num+=1.0;

        }

    } //end for loop


}

void Genome::mutate_toggle_enable(S32 times)
{
    S32 genenum;
    S32 count;
    vector<GenePtr>::iterator thegene; //Gene to toggle
    vector<GenePtr>::iterator checkgene; //Gene to check
    S32 genecount;

    for (count=1; count<=times; count++)
    {

        //Choose a random genenum
        genenum=randint(0, static_cast<S32>(genes.size())-1);

        //find the gene
        thegene=genes.begin();
        for (genecount=0; genecount<genenum; genecount++)
            ++thegene;

        //Toggle the enable on this gene
        if (((*thegene)->enable)==true)
        {
            //We need to make sure that another gene connects out of the in-node
            //Because if not a section of network will break off and become isolated
            checkgene=genes.begin();
            while ( checkgene != genes.end() &&
                    (*checkgene) &&
                    ( (*checkgene)->lnk->get_in_node().get() != (*thegene)->lnk->get_in_node().get() 
                      || (*checkgene)->enable == false 
                      || (*checkgene)->innovation_num == (*thegene)->innovation_num ) )
                {
                    ++checkgene;
                }

            //Disable the gene if it's safe to do so
            if (checkgene!=genes.end())
                (*thegene)->enable=false;
        }
        else
            (*thegene)->enable=true;
    }
}

void Genome::mutate_gene_reenable()
{
    vector<GenePtr>::iterator thegene; //Gene to enable

    thegene=genes.begin();

    //Search for a disabled gene
    while ((thegene!=genes.end())&&((*thegene)->enable==true))
        ++thegene;

    //Reenable it
    if (thegene!=genes.end())
        if (((*thegene)->enable)==false)
            (*thegene)->enable=true;

}

bool Genome::mutate_add_node(vector<InnovationPtr> &innovs, S32 &curnode_id,
                             F64 &curinnov)
{
    vector<GenePtr>::iterator thegene; //random gene containing the original link
    S32 genenum; //The random gene number
    NNodePtr in_node; //Here are the nodes connected by the gene
    NNodePtr out_node;
    LinkPtr thelink; //The link inside the random gene

    vector<InnovationPtr>::iterator theinnov; //For finding a historical match
    bool done=false;

    GenePtr newgene1; //The new Genes
    GenePtr newgene2;
    NNodePtr newnode; //The new NNode
    TraitPtr traitptr; //The original link's trait

    F64 oldweight; //The weight of the original link

    S32 trycount; //Take a few tries to find an open node
    bool found;

    //First, find a random gene already in the genome  
    trycount=0;
    found=false;

    //Split next link with a bias towards older links
    //NOTE: 7/2/01 - for robots, went back to random split
    //        because of large # of inputs
    if (randfloat()>1.0)
    {
        thegene=genes.begin();
        while (((thegene!=genes.end())&&(!((*thegene)->enable)))||((thegene!=genes.end())&&((*thegene)->lnk->get_in_node()->gen_node_label==BIAS)))
            ++thegene;

        //Now randomize which node is chosen at this point
        //We bias the search towards older genes because 
        //this encourages splitting to distribute evenly
        while (((thegene!=genes.end())&&(randfloat()<0.3))||((thegene
            !=genes.end())&&((*thegene)->lnk->get_in_node()->gen_node_label==BIAS)))
        {
            ++thegene;
        }

        if ((!(thegene==genes.end()))&&((*thegene)->enable))
        {
            found=true;
        }
    }
    //In this else:
    //Alternative random gaussian choice of genes NOT USED in this
    //version of NEAT
    //NOTE: 7/2/01 now we use this after all
    else
    {
        while ((trycount<20)&&(!found))
        {

            //Choose a random genenum
            //randmult=gaussrand()/4;
            //if (randmult>1.0) randmult=1.0;

            //This tends to select older genes for splitting
            //genenum=(int) floor((randmult*(static_cast<S32>(genes.size())-1.0))+0.5);

            //This old totally random selection is bad- splitting
            //inside something recently splitted adds little power
            //to the system (should use a gaussian if doing it this way)
            genenum=randint(0, static_cast<S32>(genes.size())-1);

            //find the gene
            thegene=genes.begin();
            for (S32 genecount=0; genecount<genenum; genecount++)
                ++thegene;

            //If either the gene is disabled, or it has a bias input, try again
            if (!(((*thegene)->enable==false)||((((*thegene)->lnk)->get_in_node()->gen_node_label)==BIAS)))
                found=true;

            ++trycount;

        }
    }

    //If we couldn't find anything so say goodbye
    if (!found)
        return false;

    //Disabled the gene
    (*thegene)->enable=false;

    //Extract the link
    thelink=(*thegene)->lnk;
    oldweight=(*thegene)->lnk->weight;

    //Extract the nodes
    in_node=thelink->get_in_node();
    out_node=thelink->get_out_node();

    //Check to see if this innovation has already been done   
    //in another genome
    //Innovations are used to make sure the same innovation in
    //two separate genomes in the same generation receives
    //the same innovation number.
    theinnov=innovs.begin();

    while (!done)
    {

        if (theinnov==innovs.end())
        {

            //The innovation is totally novel

            //Get the old link's trait
            traitptr=thelink->linktrait;

            //Create the new NNode
            //By convention, it will point to the first trait
            newnode.reset(new NNode(NEURON,curnode_id++,HIDDEN));
            newnode->nodetrait=(*(traits.begin()));

            //Create the new Genes
            if (thelink->is_recurrent)
            {
                newgene1.reset(new Gene(traitptr,1.0,in_node,newnode,true,curinnov,0));
                newgene2.reset(new Gene(traitptr,oldweight*0.3,newnode,out_node,false,curinnov+1,0));
                curinnov+=2.0;
            }
            else
            {
                newgene1.reset(new Gene(traitptr,1.0,in_node,newnode,false,curinnov,0));
                newgene2.reset(new Gene(traitptr,oldweight*0.3,newnode,out_node,false,curinnov+1,0));
                curinnov+=2.0;
            }

            //Add the innovations (remember what was done)
            InnovationPtr
                p(new Innovation(in_node->node_id,out_node->node_id,curinnov-2.0,curinnov-1.0,newnode->node_id,(*thegene)->innovation_num));
            innovs.push_back(p);

            done=true;
        }

        // We check to see if an innovation already occured that was:
        //   -A new node
        //   -Stuck between the same nodes as were chosen for this mutation
        //   -Splitting the same gene as chosen for this mutation 
        //   If so, we know this mutation is not a novel innovation
        //   in this generation
        //   so we make it match the original, identical mutation which occured
        //   elsewhere in the population by coincidence 
        else if (((*theinnov)->innovation_type==NEWNODE)&&((*theinnov)->node_in_id==(in_node->node_id))&&((*theinnov)->node_out_id==(out_node->node_id))&&((*theinnov)->old_innov_num==(*thegene)->innovation_num))
        {

            //Here, the innovation has been done before

            //Get the old link's trait
            traitptr=thelink->linktrait;

            //Create the new NNode
            newnode.reset(new NNode(NEURON,(*theinnov)->newnode_id,HIDDEN));
            //By convention, it will point to the first trait
            //Note: In future may want to change this
            newnode->nodetrait=(*(traits.begin()));

            //Create the new Genes
            if (thelink->is_recurrent)
            {
                newgene1.reset(new Gene(traitptr,1.0,in_node,newnode,true,(*theinnov)->innovation_num1,0));
                newgene2.reset(new Gene(traitptr,oldweight*0.3,newnode,out_node,false,(*theinnov)->innovation_num2,0));
            }
            else
            {
                newgene1.reset(new Gene(traitptr,1.0,in_node,newnode,false,(*theinnov)->innovation_num1,0));
                newgene2.reset(new Gene(traitptr,oldweight*0.3,newnode,out_node,false,(*theinnov)->innovation_num2,0));
            }

            done=true;
        }
        else
            ++theinnov;
    }

    //Now add the new NNode and new Genes to the Genome
    //genes.push_back(newgene1);   //Old way to add genes- may result in genes becoming out of order
    //genes.push_back(newgene2);
    add_gene(genes, newgene1); //Add genes in correct order
    add_gene(genes, newgene2);
    node_insert(nodes, newnode);

    return true;

}

bool Genome::mutate_add_link(vector<InnovationPtr> &innovs, F64 &curinnov,
                             S32 tries)
{

    S32 nodenum1, nodenum2; //Random node numbers
    vector<NNodePtr>::iterator thenode1, thenode2; //Random node iterators
    S32 nodecount; //Counter for finding nodes
    S32 trycount; //Iterates over attempts to find an unconnected pair of nodes
    NNodePtr nodep1; //Pointers to the nodes
    NNodePtr nodep2; //Pointers to the nodes
    vector<GenePtr>::iterator thegene; //Searches for existing link
    bool found=false; //Tells whether an open pair was found
    vector<InnovationPtr>::iterator theinnov; //For finding a historical match
    S32 recurflag; //Indicates whether proposed link is recurrent
    GenePtr newgene; //The new Gene

    S32 traitnum; //Random trait finder
    vector<TraitPtr>::iterator thetrait;

    F64 newweight; //The new weight for the new link

    bool done;
    bool do_recur;
    bool loop_recur;
    S32 first_nonsensor;

    //These are used to avoid getting stuck in an infinite loop checking
    //for recursion
    //Note that we check for recursion to control the frequency of
    //adding recurrent links rather than to prevent any paricular
    //kind of error
    S32 thresh=static_cast<S32>(nodes.size() * nodes.size());
    S32 count=0;

    //Make attempts to find an unconnected pair
    trycount=0;

    //Decide whether to make this recurrent
    if (randfloat()<recur_only_prob)
        do_recur=true;
    else
        do_recur=false;

    //Find the first non-sensor so that the to-node won't look at sensors as
    //possible destinations
    first_nonsensor=0;
    thenode1=nodes.begin();
    while (((*thenode1)->get_type())==SENSOR)
    {
        first_nonsensor++;
        ++thenode1;
    }

    //Here is the recurrent finder loop- it is done separately
    if (do_recur)
    {

        while (trycount<tries)
        {

            //Some of the time try to make a recur loop
            if (randfloat()>0.5)
            {
                loop_recur=true;
            }
            else
                loop_recur=false;

            if (loop_recur)
            {
                nodenum1=randint(first_nonsensor,
                                 static_cast<S32>(nodes.size())-1);
                nodenum2=nodenum1;
            }
            else
            {
                //Choose random nodenums
                nodenum1=randint(0, static_cast<S32>(nodes.size())-1);
                nodenum2=randint(first_nonsensor,
                                 static_cast<S32>(nodes.size())-1);
            }

            //Find the first node
            thenode1=nodes.begin();
            for (nodecount=0; nodecount<nodenum1; nodecount++)
                ++thenode1;

            //Find the second node
            thenode2=nodes.begin();
            for (nodecount=0; nodecount<nodenum2; nodecount++)
                ++thenode2;

            nodep1=(*thenode1);
            nodep2=(*thenode2);

            //See if a recur link already exists  ALSO STOP AT END OF GENES!!!!
            thegene=genes.begin();
            while ((thegene!=genes.end()) &&((nodep2->type)!=SENSOR)&& //Don't allow SENSORS to get input
                (!((((*thegene)->lnk)->get_in_node().get()==nodep1.get())&&(((*thegene)->lnk)->get_out_node().get()==nodep2.get())&&((*thegene)->lnk)->is_recurrent)))
            {
                ++thegene;
            }

            if (thegene!=genes.end())
                trycount++;
            else
            {
                count=0;
                recurflag=phenotype.lock()->is_recur(nodep1->analogue,
                                              nodep2->analogue, count, thresh);

                //ADDED: CONSIDER connections out of outputs recurrent
                if (((nodep1->type)==OUTPUT)||((nodep2->type)==OUTPUT))
                    recurflag=true;

                //Exit if the network is faulty (contains an infinite loop)
                //NOTE: A loop doesn't really matter
                //if (count>thresh) {
                //  cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<endl;
                //  return false;
                //}

                //Make sure it finds the right kind of link (recur)
                if (!(recurflag))
                    trycount++;
                else
                {
                    trycount=tries;
                    found=true;
                }

            }

        }
    }
    else
    {
        //Loop to find a nonrecurrent link
        while (trycount<tries)
        {

            //cout<<"TRY "<<trycount<<endl;

            //Choose random nodenums
            nodenum1=randint(0, static_cast<S32>(nodes.size())-1);
            nodenum2=randint(first_nonsensor, static_cast<S32>(nodes.size())-1);

            //Find the first node
            thenode1=nodes.begin();
            for (nodecount=0; nodecount<nodenum1; nodecount++)
                ++thenode1;

            //cout<<"RETRIEVED NODE# "<<(*thenode1)->node_id<<endl;

            //Find the second node
            thenode2=nodes.begin();
            for (nodecount=0; nodecount<nodenum2; nodecount++)
                ++thenode2;

            nodep1=(*thenode1);
            nodep2=(*thenode2);

            //See if a link already exists  ALSO STOP AT END OF GENES!!!!
            thegene=genes.begin();
            while ((thegene!=genes.end()) &&((nodep2->type)!=SENSOR)&& //Don't allow SENSORS to get input
                (!((((*thegene)->lnk)->get_in_node().get()==nodep1.get())&&(((*thegene)->lnk)->get_out_node().get()==nodep2.get())&&(!(((*thegene)->lnk)->is_recurrent)))))
            {
                ++thegene;
            }

            if (thegene!=genes.end())
                trycount++;
            else
            {

                count=0;
                recurflag=phenotype.lock()->is_recur(nodep1->analogue,
                                                     nodep2->analogue, count, thresh);

                //ADDED: CONSIDER connections out of outputs recurrent
                if (((nodep1->type)==OUTPUT)||((nodep2->type)==OUTPUT))
                    recurflag=true;

                //Exit if the network is faulty (contains an infinite loop)
                if (count>thresh)
                {
                    //cout<<"LOOP DETECTED DURING A RECURRENCY CHECK"<<endl;
                    //return false;
                }

                //Make sure it finds the right kind of link (recur or not)
                if (recurflag)
                    trycount++;
                else
                {
                    trycount=tries;
                    found=true;
                }

            }

        } //End of normal link finding loop
    }

    //Continue only if an open link was found
    if (found)
    {

        //Check to see if this innovation already occured in the population
        theinnov=innovs.begin();

        //If it was supposed to be recurrent, make sure it gets labeled that way
        if (do_recur)
            recurflag=1;

        done=false;

        while (!done)
        {

            //The innovation is totally novel
            if (theinnov==innovs.end())
            {

                Assert(phenotype.lock());

                //Useful for debugging
                //cout<<"nodep1 id: "<<nodep1->node_id<<endl;
                //cout<<"nodep1: "<<nodep1<<endl;
                //cout<<"nodep1 analogue: "<<nodep1->analogue<<endl;
                //cout<<"nodep2 id: "<<nodep2->node_id<<endl;
                //cout<<"nodep2: "<<nodep2<<endl;
                //cout<<"nodep2 analogue: "<<nodep2->analogue<<endl;
                //cout<<"recurflag: "<<recurflag<<endl;

                //NOTE: Something like this could be used for time delays,
                //      which are not yet supported.  However, this does not
                //      have an application with recurrency.
                //If not recurrent, randomize recurrency
                //if (!recurflag) 
                //  if (randfloat()<recur_prob) recurflag=1;

                //Choose a random trait
                traitnum=randint(0, static_cast<S32>(traits.size())-1);
                thetrait=traits.begin();

                //Choose the new weight
                //newweight=(gaussrand())/1.5;  //Could use a gaussian
                newweight=randposneg()*randfloat()*1.0; //used to be 10.0

                //Create the new gene
                newgene.reset(new Gene(((thetrait[traitnum])),newweight,nodep1,nodep2,recurflag != 0,curinnov,newweight));

                //Add the innovation
                InnovationPtr
                    p(new Innovation(nodep1->node_id,nodep2->node_id,curinnov,newweight,traitnum));
                innovs.push_back(p);

                curinnov=curinnov+1.0;

                done=true;
            }
            //OTHERWISE, match the innovation in the innovs list
            else if ( (*theinnov)->innovation_type ==NEWLINK && 
                      (*theinnov)->node_in_id == nodep1->node_id && 
                      (*theinnov)->node_out_id == nodep2->node_id && 
                      (*theinnov)->recur_flag == (recurflag != 0) )
            {

                thetrait=traits.begin();

                //Create new gene
                newgene.reset(new Gene(
                    thetrait[(*theinnov)->new_traitnum],
                    (*theinnov)->new_weight,
                    nodep1, nodep2, 
                    recurflag != 0,
                    (*theinnov)->innovation_num1,
                    0));

                done=true;

            }
            else
            {
                //Keep looking for a matching innovation from this generation
                ++theinnov;
            }
        }

        //Now add the new Genes to the Genome
        //genes.push_back(newgene);  //Old way- this can result in the genes being out of order
        add_gene(genes, newgene); //Adds the gene in correct order

        return true;
    }
    else
    {
        return false;
    }

}

void Genome::mutate_add_sensor(vector<InnovationPtr> &innovs, double &curinnov)
{

    vector<NNodePtr> sensors;
    vector<NNodePtr> outputs;
    NNodePtr node;
    NNodePtr sensor;
    NNodePtr output;
    GenePtr gene;

    double newweight = 0.0;
    GenePtr newgene;

    bool found;

    bool done;

    size_t outputConnections;

    vector<TraitPtr>::iterator thetrait;
    int traitnum;

    vector<InnovationPtr>::iterator theinnov; //For finding a historical match

    //Find all the sensors and outputs
    for (size_t i = 0; i < nodes.size(); i++)
    {
        node=nodes[i];

        if ((node->type) == SENSOR)
            sensors.push_back(node);
        else if (node->gen_node_label == OUTPUT)
            outputs.push_back(node);
    }

    // eliminate from contention any sensors that are already connected
    for (size_t i = 0; i < sensors.size(); i++)
    {
        sensor=sensors[i];

        outputConnections=0;

        for (size_t j = 0; j < genes.size(); j++)
        {
            gene=genes[j];

            if ((gene->lnk)->get_out_node()->gen_node_label == OUTPUT)
                outputConnections++;

        }

        if (outputConnections == outputs.size())
        {
            sensors.erase(sensors.begin() + i);
            --i;
        }

    }

    //If all sensors are connected, quit
    if (sensors.size() == 0)
        return;

    //Pick randomly from remaining sensors
    sensor=sensors[randint(0, static_cast<S32>(sensors.size())-1)];

    //Add new links to chosen sensor, avoiding redundancy
    for (size_t i = 0; i < outputs.size(); i++)
    {
        output=outputs[i];

        found=false;
        for (size_t j = 0; j < genes.size(); j++)
        {
            gene=genes[j];
            if ((gene->lnk->get_in_node().get()==sensor.get())&&(gene->lnk->get_out_node().get()==output.get()))
                found=true;
        }

        //Record the innovation
        if (!found)
        {
            theinnov=innovs.begin();
            done=false;

            while (!done)
            {
                //The innovation is novel
                if (theinnov==innovs.end())
                {

                    //Choose a random trait
                    traitnum=randint(0, static_cast<S32>(traits.size())-1);
                    thetrait=traits.begin();

                    //Choose the new weight
                    //newweight=(gaussrand())/1.5;  //Could use a gaussian
                    newweight=randposneg()*randfloat()*3.0; //used to be 10.0

                    //Create the new gene
                    newgene.reset(new Gene(((thetrait[traitnum])),
                        newweight,sensor,output,false,
                        curinnov,newweight));

                    //Add the innovation
                    InnovationPtr
                        p(new Innovation(sensor->node_id,output->node_id,curinnov,newweight,traitnum));
                    innovs.push_back(p);

                    curinnov=curinnov+1.0;

                    done=true;
                } //end novel innovation case
                //OTHERWISE, match the innovation in the innovs list
                else if (((*theinnov)->innovation_type==NEWLINK)&&((*theinnov)->node_in_id==(sensor->node_id))&&((*theinnov)->node_out_id==(output->node_id))&&((*theinnov)->recur_flag==false))
                {

                    thetrait=traits.begin();

                    //Create new gene
                    newgene.reset(new Gene(((thetrait[(*theinnov)->new_traitnum])),
                        (*theinnov)->new_weight,sensor,output,
                        false,(*theinnov)->innovation_num1,0));

                    done=true;

                } //end prior innovation case
                //Keep looking for matching innovation
                else
                    ++theinnov;

            } //end while

            //genes.push_back(newgene);
            add_gene(genes, newgene); //adds the gene in correct order

        } //end case where the gene didn't previously exist
    }

}

//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome::add_gene(vector<GenePtr> &glist, GenePtr g)
{
    vector<GenePtr>::iterator curgene;

    F64 inum=g->innovation_num;

    curgene=glist.begin();
    while ((curgene!=glist.end())&&(((*curgene)->innovation_num)<inum))
    {
        ++curgene;

    }

    glist.insert(curgene, g);

}

void Genome::node_insert(vector<NNodePtr> &nlist, NNodePtr n)
{
    vector<NNodePtr>::iterator curnode;

    S32 id=n->node_id;

    curnode=nlist.begin();
    while ((curnode!=nlist.end())&&(((*curnode)->node_id)<id))
        ++curnode;

    nlist.insert(curnode, n);

}

GenomePtr Genome::mate_multipoint(GenomePtr g, S32 genomeid, F64 fitness1,
                                  F64 fitness2, bool interspec_flag)
{
    //The baby Genome will contain these new Traits, NNodes, and Genes
    vector<TraitPtr> newtraits;
    vector<NNodePtr> newnodes;
    vector<GenePtr> newgenes;
    GenomePtr new_genome;

    vector<GenePtr>::iterator curgene2; //Checks for link duplication

    //iterators for moving through the two parents' traits
    vector<TraitPtr>::iterator p1trait;
    vector<TraitPtr>::iterator p2trait;
    TraitPtr newtrait;

    //iterators for moving through the two parents' genes
    vector<GenePtr>::iterator p1gene;
    vector<GenePtr>::iterator p2gene;
    F64 p1innov; //Innovation numbers for genes inside parents' Genomes
    F64 p2innov;
    GenePtr chosengene; //Gene chosen for baby to inherit
    S32 traitnum; //Number of trait new gene points to
    NNodePtr inode; //NNodes connected to the chosen Gene
    NNodePtr onode;
    NNodePtr new_inode;
    NNodePtr new_onode;
    vector<NNodePtr>::iterator curnode; //For checking if NNodes exist already 
    S32 nodetraitnum; //Trait number for a NNode

    bool disable; //Set to true if we want to disabled a chosen gene

    disable=false;
    GenePtr newgene;

    bool p1better; //Tells if the first genome (this one) has better fitness or not

    bool skip;

    //First, average the Traits from the 2 parents to form the baby's Traits
    //It is assumed that trait lists are the same length
    //In the future, may decide on a different method for trait mating
    p2trait=(g->traits).begin();
    for (p1trait=traits.begin(); p1trait!=traits.end(); ++p1trait)
    {
        TraitPtr newtrait(new Trait(*p1trait,*p2trait)); //Construct by averaging
        newtraits.push_back(newtrait);
        ++p2trait;
    }

    //Figure out which genome is better
    //The worse genome should not be allowed to add extra structural baggage
    //If they are the same, use the smaller one's disjoint and excess genes only
    if (fitness1>fitness2)
        p1better=true;
    else if (fitness1==fitness2)
    {
        if (genes.size() < g->genes.size() )
            p1better=true;
        else
            p1better=false;
    }
    else
        p1better=false;

    //NEW 3/17/03 Make sure all sensors and outputs are included
    for (curnode=(g->nodes).begin(); curnode!=(g->nodes).end(); ++curnode)
    {
        if ((((*curnode)->gen_node_label)==INPUT)||(((*curnode)->gen_node_label)==BIAS)||(((*curnode)->gen_node_label)==OUTPUT))
        {
            if (!((*curnode)->nodetrait))
                nodetraitnum=0;
            else
                nodetraitnum=(((*curnode)->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

            //Create a new node off the sensor or output
            new_onode.reset(new NNode((*curnode),newtraits[nodetraitnum]));

            //Add the new node
            node_insert(newnodes, new_onode);

        }

    }

    //Now move through the Genes of each parent until both genomes end
    p1gene=genes.begin();
    p2gene=(g->genes).begin();
    while (!((p1gene==genes.end())&&(p2gene==(g->genes).end())))
    {

        skip=false; //Default to not skipping a chosen gene

        if (p1gene==genes.end())
        {
            chosengene=*p2gene;
            ++p2gene;
            if (p1better)
                skip=true; //Skip excess from the worse genome
        }
        else if (p2gene==(g->genes).end())
        {
            chosengene=*p1gene;
            ++p1gene;
            if (!p1better)
                skip=true; //Skip excess from the worse genome
        }
        else
        {
            //Extract current innovation numbers
            p1innov=(*p1gene)->innovation_num;
            p2innov=(*p2gene)->innovation_num;

            if (p1innov==p2innov)
            {
                if (randfloat()<0.5)
                {
                    chosengene=*p1gene;
                }
                else
                {
                    chosengene=*p2gene;
                }

                //If one is disabled, the corresponding gene in the offspring
                //will likely be disabled
                if ((((*p1gene)->enable)==false)||(((*p2gene)->enable)==false))
                    if (randfloat()<0.75)
                        disable=true;

                ++p1gene;
                ++p2gene;

            }
            else if (p1innov<p2innov)
            {
                chosengene=*p1gene;
                ++p1gene;

                if (!p1better)
                    skip=true;

            }
            else if (p2innov<p1innov)
            {
                chosengene=*p2gene;
                ++p2gene;
                if (p1better)
                    skip=true;
            }
        }

        //Uncomment this line to let growth go faster (from both parents excesses)
        skip=false;

        //For interspecies mating, allow all genes through:
        if (interspec_flag)
            skip=false;

        //Check to see if the chosengene conflicts with an already chosen gene
        //i.e. do they represent the same link    
        curgene2=newgenes.begin();
        while ((curgene2!=newgenes.end())&&(!((((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent))))&&(!((((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(!((((*curgene2)->lnk)->is_recurrent)))&&(!((((chosengene)->lnk)->is_recurrent))))))
        {
            ++curgene2;
        }

        if (curgene2!=newgenes.end())
            skip=true; //Links conflicts, abort adding

        if (!skip)
        {

            //Now add the chosengene to the baby

            //First, get the trait pointer
            if (!chosengene->lnk->linktrait)
                traitnum=(*(traits.begin()))->trait_id;
            else
                traitnum=(((chosengene->lnk)->linktrait)->trait_id)-(*(traits.begin()))->trait_id; //The subtracted number normalizes depending on whether traits start counting at 1 or 0

            //Next check for the nodes, add them if not in the baby Genome already
            inode=(chosengene->lnk)->get_in_node();
            onode=(chosengene->lnk)->get_out_node();

            //Check for inode in the newnodes list
            if (inode->node_id<onode->node_id)
            {
                //inode before onode

                //Checking for inode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;

                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //(normalized trait number for new NNode)

                    //old buggy version:
                    // if (!(onode->nodetrait)) nodetraitnum=((*(traits.begin()))->trait_id);
                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-((*(traits.begin()))->trait_id);

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));
                    node_insert(newnodes, new_inode);

                }
                else
                {
                    new_inode=(*curnode);

                }

                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode

                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_onode);

                }
                else
                {
                    new_onode=(*curnode);
                }

            }
            //If the onode has a higher id than the inode we want to add it first
            else
            {
                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));
                    //newnodes.push_back(new_onode);
                    node_insert(newnodes, new_onode);

                }
                else
                {
                    new_onode=(*curnode);

                }

                //Checking for inode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_inode);

                }
                else
                {
                    new_inode=(*curnode);

                }

            } //End NNode checking section- NNodes are now in new Genome

            //Add the Gene
            GenePtr newgene(new Gene(chosengene,newtraits[traitnum],new_inode,new_onode));
            if (disable)
            {
                newgene->enable=false;
                disable=false;
            }
            newgenes.push_back(newgene);
        }

    }

    new_genome.reset(new Genome(genomeid,newtraits,newnodes,newgenes));

    //Return the baby Genome
    return (new_genome);

}

GenomePtr Genome::mate_multipoint_avg(GenomePtr g, S32 genomeid, F64 fitness1,
                                      F64 fitness2, bool interspec_flag)
{
  //DEBUG cout << "** MATING: " << endl << *this << endl << "** with " << *g << endl;

    //The baby Genome will contain these new Traits, NNodes, and Genes
    vector<TraitPtr> newtraits;
    vector<NNodePtr> newnodes;
    vector<GenePtr> newgenes;

    //iterators for moving through the two parents' traits
    vector<TraitPtr>::iterator p1trait;
    vector<TraitPtr>::iterator p2trait;

    vector<GenePtr>::iterator curgene2; //Checking for link duplication

    //iterators for moving through the two parents' genes
    vector<GenePtr>::iterator p1gene;
    vector<GenePtr>::iterator p2gene;
    F64 p1innov; //Innovation numbers for genes inside parents' Genomes
    F64 p2innov;
    GenePtr chosengene; //Gene chosen for baby to inherit
    S32 traitnum; //Number of trait new gene points to
    NNodePtr inode; //NNodes connected to the chosen Gene
    NNodePtr onode;
    NNodePtr new_inode;
    NNodePtr new_onode;

    vector<NNodePtr>::iterator curnode; //For checking if NNodes exist already 
    S32 nodetraitnum; //Trait number for a NNode

    GenePtr newgene;

    bool skip;

    bool p1better; //Designate the better genome

    //First, average the Traits from the 2 parents to form the baby's Traits
    //It is assumed that trait lists are the same length
    //In future, could be done differently
    p2trait=(g->traits).begin();
    for (p1trait=traits.begin(); p1trait!=traits.end(); ++p1trait)
    {
        TraitPtr newtrait(new Trait(*p1trait,*p2trait)); //Construct by averaging
        newtraits.push_back(newtrait);
        ++p2trait;
    }

    //This Gene is used to hold the average of the two genes to be averaged
    //Set up the avgene
    GenePtr avgene(new Gene());

    //NEW 3/17/03 Make sure all sensors and outputs are included
    for (curnode=(g->nodes).begin(); curnode!=(g->nodes).end(); ++curnode)
    {
        if ((((*curnode)->gen_node_label)==INPUT)||(((*curnode)->gen_node_label)==OUTPUT)||(((*curnode)->gen_node_label)==BIAS))
        {
            if (!((*curnode)->nodetrait))
                nodetraitnum=0;
            else
                nodetraitnum=(((*curnode)->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

            //Create a new node off the sensor or output
            new_onode.reset(new NNode((*curnode),newtraits[nodetraitnum]));

            //Add the new node
            node_insert(newnodes, new_onode);

        }

    }

    //Figure out which genome is better
    //The worse genome should not be allowed to add extra structural baggage
    //If they are the same, use the smaller one's disjoint and excess genes only
    if (fitness1>fitness2)
        p1better=true;
    else if (fitness1==fitness2)
    {
        if (genes.size() < g->genes.size() )
            p1better=true;
        else
            p1better=false;
    }
    else
        p1better=false;

    //Now move through the Genes of each parent until both genomes end
    p1gene=genes.begin();
    p2gene=(g->genes).begin();
    while (!((p1gene==genes.end())&&(p2gene==(g->genes).end())))
    {

        avgene->enable=true; //Default to enabled

        skip=false;

        if (p1gene==genes.end())
        {
            chosengene=*p2gene;
            ++p2gene;

            if (p1better)
                skip=true;

        }
        else if (p2gene==(g->genes).end())
        {
            chosengene=*p1gene;
            ++p1gene;

            if (!p1better)
                skip=true;
        }
        else
        {
            //Extract current innovation numbers
            p1innov=(*p1gene)->innovation_num;
            p2innov=(*p2gene)->innovation_num;

            if (p1innov==p2innov)
            {
                //Average them into the avgene
                if (randfloat()>0.5)
                    (avgene->lnk)->linktrait=((*p1gene)->lnk)->linktrait;
                else
                    (avgene->lnk)->linktrait=((*p2gene)->lnk)->linktrait;

                //WEIGHTS AVERAGED HERE
                (avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;

                if (randfloat()>0.5)
                    avgene->lnk->set_in_node((*p1gene)->lnk->get_in_node());
                else
                    avgene->lnk->set_in_node((*p2gene)->lnk->get_in_node());

                if (randfloat()>0.5)
                    avgene->lnk->set_out_node((*p1gene)->lnk->get_out_node());
                else
                    avgene->lnk->set_out_node((*p2gene)->lnk->get_out_node());

                if (randfloat()>0.5)
                    avgene->lnk->is_recurrent=(*p1gene)->lnk->is_recurrent;
                else
                    avgene->lnk->is_recurrent=(*p2gene)->lnk->is_recurrent;

                avgene->innovation_num=(*p1gene)->innovation_num;
                avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

                if ((((*p1gene)->enable)==false)||(((*p2gene)->enable)==false))
                    if (randfloat()<0.75)
                        avgene->enable=false;

                chosengene=avgene;
                ++p1gene;
                ++p2gene;
            }
            else if (p1innov<p2innov)
            {
                chosengene=*p1gene;
                ++p1gene;

                if (!p1better)
                    skip=true;
            }
            else if (p2innov<p1innov)
            {
                chosengene=*p2gene;
                ++p2gene;

                if (p1better)
                    skip=true;
            }
        }

        //THIS LINE MUST BE DELETED TO SLOW GROWTH
        skip=false;

        //For interspecies mating, allow all genes through:
        if (interspec_flag)
            skip=false;

        //Check to see if the chosengene conflicts with an already chosen gene
        //i.e. do they represent the same link    
        curgene2=newgenes.begin();
        while ((curgene2!=newgenes.end()))

        {

            if (((((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent)))||((((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&(!((((*curgene2)->lnk)->is_recurrent)))&&(!((((chosengene)->lnk)->is_recurrent)))))
            {
                skip=true;

            }
            ++curgene2;
        }

        if (!skip)
        {

            //Now add the chosengene to the baby

            //First, get the trait pointer
	  if (!chosengene->lnk->linktrait) {
	    traitnum=(*(traits.begin()))->trait_id;
	  } else {
	    //The subtracted number normalizes depending on whether traits start counting at 1 or 0
	    traitnum=chosengene->lnk->linktrait->trait_id - (*(traits.begin()))->trait_id;
	  }

            //Next check for the nodes, add them if not in the baby Genome already
            inode=(chosengene->lnk)->get_in_node();
            onode=(chosengene->lnk)->get_out_node();

            //Check for inode in the newnodes list
            if (inode->node_id<onode->node_id)
            {

                //Checking for inode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;

                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode

                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-((*(traits.begin()))->trait_id);

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_inode);
                }
                else
                {
                    new_inode=(*curnode);

                }

                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode

                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;
                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_onode);
                }
                else
                {
                    new_onode=(*curnode);
                }
            }
            //If the onode has a higher id than the inode we want to add it first
            else
            {
                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_onode);
                }
                else
                {
                    new_onode=(*curnode);
                }

                //Checking for inode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_inode);
                }
                else
                {
                    new_inode=(*curnode);

                }

            } //End NNode checking section- NNodes are now in new Genome

            //Add the Gene
            GenePtr newgene(new Gene(chosengene,newtraits[traitnum],new_inode,new_onode));

            newgenes.push_back(newgene);

        } //End if which checked for link duplicationb

    }

    //Return the baby Genome
    return GenomePtr(new Genome(genomeid,newtraits,newnodes,newgenes));

}

GenomePtr Genome::mate_singlepoint(GenomePtr g, S32 genomeid)
{
    //The baby Genome will contain these new Traits, NNodes, and Genes
    vector<TraitPtr> newtraits;
    vector<NNodePtr> newnodes;
    vector<GenePtr> newgenes;

    //iterators for moving through the two parents' traits
    vector<TraitPtr>::iterator p1trait;
    vector<TraitPtr>::iterator p2trait;
    TraitPtr newtrait;

    vector<GenePtr>::iterator curgene2; //Check for link duplication

    //iterators for moving through the two parents' genes
    vector<GenePtr>::iterator p1gene;
    vector<GenePtr>::iterator p2gene;
    vector<GenePtr>::iterator stopper; //To tell when finished
    vector<GenePtr>::iterator p2stop;
    vector<GenePtr>::iterator p1stop;
    F64 p1innov; //Innovation numbers for genes inside parents' Genomes
    F64 p2innov;
    GenePtr chosengene; //Gene chosen for baby to inherit
    S32 traitnum; //Number of trait new gene points to
    NNodePtr inode; //NNodes connected to the chosen Gene
    NNodePtr onode;
    NNodePtr new_inode;
    NNodePtr new_onode;
    vector<NNodePtr>::iterator curnode; //For checking if NNodes exist already 
    S32 nodetraitnum; //Trait number for a NNode

    //This Gene is used to hold the average of the two genes to be averaged
    GenePtr avgene;

    S32 crosspoint; //The point in the Genome to cross at
    S32 genecounter; //Counts up to the crosspoint
    bool skip; //Used for skipping unwanted genes

    //First, average the Traits from the 2 parents to form the baby's Traits
    //It is assumed that trait lists are the same length
    p2trait=(g->traits).begin();
    for (p1trait=traits.begin(); p1trait!=traits.end(); ++p1trait)
    {
        TraitPtr newtrait(new Trait(*p1trait,*p2trait)); //Construct by averaging
        newtraits.push_back(newtrait);
        ++p2trait;
    }

    //Set up the avgene
    avgene.reset(new Gene());

    //Decide where to cross  (p1gene will always be in smaller Genome)
    if (genes.size() < g->genes.size())
    {
        crosspoint=randint(0, static_cast<S32>(genes.size()-1));
        p1gene=genes.begin();
        p2gene=(g->genes).begin();
        stopper=(g->genes).end();
        p1stop=genes.end();
        p2stop=(g->genes).end();
    }
    else
    {
        crosspoint = randint(static_cast<S32>(0), static_cast<S32>(g->genes.size() - 1));
        p2gene=genes.begin();
        p1gene= g->genes.begin();
        stopper=genes.end();
        p1stop= g->genes.end();
        p2stop=genes.end();
    }

    genecounter=0; //Ready to count to crosspoint

    skip=false; //Default to not skip a Gene
    //Note that we skip when we are on the wrong Genome before
    //crossing

    //Now move through the Genes of each parent until both genomes end
    while (p2gene!=stopper)
    {

        avgene->enable=true; //Default to true

        if (p1gene==p1stop)
        {
            chosengene=*p2gene;
            ++p2gene;
        }
        else if (p2gene==p2stop)
        {
            chosengene=*p1gene;
            ++p1gene;
        }
        else
        {
            //Extract current innovation numbers

            //if (p1gene==g->genes.end()) cout<<"WARNING p1"<<endl;
            //if (p2gene==g->genes.end()) cout<<"WARNING p2"<<endl;

            p1innov=(*p1gene)->innovation_num;
            p2innov=(*p2gene)->innovation_num;

            if (p1innov==p2innov)
            {

                //Pick the chosengene depending on whether we've crossed yet
                if (genecounter<crosspoint)
                {
                    chosengene=*p1gene;
                }
                else if (genecounter>crosspoint)
                {
                    chosengene=*p2gene;
                }
                //We are at the crosspoint here
                else
                {

                    //Average them into the avgene
                    if (randfloat()>0.5)
                        (avgene->lnk)->linktrait=((*p1gene)->lnk)->linktrait;
                    else
                        (avgene->lnk)->linktrait=((*p2gene)->lnk)->linktrait;

                    //WEIGHTS AVERAGED HERE
                    (avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;

                    if (randfloat()>0.5)
                        avgene->lnk->set_in_node((*p1gene)->lnk->get_in_node());
                    else
                        avgene->lnk->set_in_node((*p2gene)->lnk->get_in_node());

                    if (randfloat()>0.5)
                        avgene->lnk->set_out_node((*p1gene)->lnk->get_out_node());
                    else
                        avgene->lnk->set_out_node((*p2gene)->lnk->get_out_node());

                    if (randfloat()>0.5)
                        (avgene->lnk)->is_recurrent=((*p1gene)->lnk)->is_recurrent;
                    else
                        (avgene->lnk)->is_recurrent=((*p2gene)->lnk)->is_recurrent;

                    avgene->innovation_num=(*p1gene)->innovation_num;
                    avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

                    if ((((*p1gene)->enable)==false)||(((*p2gene)->enable)==false))
                        avgene->enable=false;

                    chosengene=avgene;
                }

                ++p1gene;
                ++p2gene;
                ++genecounter;
            }
            else if (p1innov<p2innov)
            {
                if (genecounter<crosspoint)
                {
                    chosengene=*p1gene;
                    ++p1gene;
                    ++genecounter;
                }
                else
                {
                    chosengene=*p2gene;
                    ++p2gene;
                }
            }
            else if (p2innov<p1innov)
            {
                ++p2gene;
                skip=true; //Special case: we need to skip to the next iteration
                //becase this Gene is before the crosspoint on the wrong Genome
            }
        }

        //Check to see if the chosengene conflicts with an already chosen gene
        //i.e. do they represent the same link    
        curgene2=newgenes.begin();

        while ((curgene2!=newgenes.end())&&(!((((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent))))&&(!((((((*curgene2)->lnk)->get_in_node())->node_id)==((((chosengene)->lnk)->get_out_node())->node_id))&&(((((*curgene2)->lnk)->get_out_node())->node_id)==((((chosengene)->lnk)->get_in_node())->node_id))&&(!((((*curgene2)->lnk)->is_recurrent)))&&(!((((chosengene)->lnk)->is_recurrent))))))
        {

            ++curgene2;
        }

        if (curgene2!=newgenes.end())
            skip=true; //Link is a duplicate

        if (!skip)
        {
            //Now add the chosengene to the baby

            //First, get the trait pointer
            if (!(chosengene->lnk)->linktrait)
                traitnum=(*(traits.begin()))->trait_id;
            else
                traitnum=(((chosengene->lnk)->linktrait)->trait_id)-(*(traits.begin()))->trait_id; //The subtracted number normalizes depending on whether traits start counting at 1 or 0

            //Next check for the nodes, add them if not in the baby Genome already
            inode=(chosengene->lnk)->get_in_node();
            onode=(chosengene->lnk)->get_out_node();

            //Check for inode in the newnodes list
            if (inode->node_id<onode->node_id)
            {
                //cout<<"inode before onode"<<endl;
                //Checking for inode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;

                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode

                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-((*(traits.begin()))->trait_id);

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));

                    node_insert(newnodes, new_inode);
                }
                else
                {
                    new_inode=(*curnode);
                }

                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode

                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));
                    node_insert(newnodes, new_onode);

                }
                else
                {
                    new_onode=(*curnode);
                }
            }
            //If the onode has a higher id than the inode we want to add it first
            else
            {
                //Checking for onode's existence
                curnode=newnodes.begin();
                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=onode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(onode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((onode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_onode.reset(new NNode(onode,newtraits[nodetraitnum]));
                    node_insert(newnodes, new_onode);
                }
                else
                {
                    new_onode=(*curnode);
                }

                //Checking for inode's existence
                curnode=newnodes.begin();

                while ((curnode!=newnodes.end())&&((*curnode)->node_id!=inode->node_id))
                    ++curnode;
                if (curnode==newnodes.end())
                {
                    //Here we know the node doesn't exist so we have to add it
                    //normalized trait number for new NNode
                    if (!(inode->nodetrait))
                        nodetraitnum=0;
                    else
                        nodetraitnum=((inode->nodetrait)->trait_id)-(*(traits.begin()))->trait_id;

                    new_inode.reset(new NNode(inode,newtraits[nodetraitnum]));
                    //newnodes.push_back(new_inode);
                    node_insert(newnodes, new_inode);
                }
                else
                {
                    new_inode=(*curnode);
                }

            } //End NNode checking section- NNodes are now in new Genome

            //Add the Gene
            GenePtr p(new Gene(chosengene,newtraits[traitnum],new_inode,new_onode));
            newgenes.push_back(p);

        } //End of if (!skip)

        skip=false;

    }

    //Return the baby Genome
    return GenomePtr(new Genome(genomeid,newtraits,newnodes,newgenes));

}

F64 Genome::compatibility(GenomePtr g)
{
	assert(g);
	
    //iterators for moving through the two potential parents' Genes
    vector<GenePtr>::iterator p1gene;
    vector<GenePtr>::iterator p2gene;

    //Innovation numbers
    F64 p1innov;
    F64 p2innov;

    //Intermediate value
    F64 mut_diff;

    //Set up the counters
    F64 num_disjoint=0.0;
    F64 num_excess=0.0;
    F64 mut_diff_total=0.0;
    F64 num_matching=0.0; //Used to normalize mutation_num differences

    F64 max_genome_size; //Size of larger Genome

    //Get the length of the longest Genome for percentage computations
    if (genes.size() < g->genes.size())
        max_genome_size=(g->genes).size();
    else
        max_genome_size=static_cast<S32>(genes.size());

    //Now move through the Genes of each potential parent 
    //until both Genomes end
    p1gene=genes.begin();
    p2gene=(g->genes).begin();
    while (!((p1gene==genes.end())&&(p2gene==(g->genes).end())))
    {

        if (p1gene==genes.end())
        {
            ++p2gene;
            num_excess+=1.0;
        }
        else if (p2gene==(g->genes).end())
        {
            ++p1gene;
            num_excess+=1.0;
        }
        else
        {
            //Extract current innovation numbers
            p1innov=(*p1gene)->innovation_num;
            p2innov=(*p2gene)->innovation_num;

            if (p1innov==p2innov)
            {
                num_matching+=1.0;
                mut_diff=((*p1gene)->mutation_num)-((*p2gene)->mutation_num);
                if (mut_diff<0.0)
                    mut_diff=0.0-mut_diff;
                //mut_diff+=trait_compare((*p1gene)->lnk->linktrait,(*p2gene)->lnk->linktrait); //CONSIDER TRAIT DIFFERENCES
                mut_diff_total+=mut_diff;

                ++p1gene;
                ++p2gene;
            }
            else if (p1innov<p2innov)
            {
                ++p1gene;
                num_disjoint+=1.0;
            }
            else if (p2innov<p1innov)
            {
                ++p2gene;
                num_disjoint+=1.0;
            }
        }
    } //End while

    //Return the compatibility number using compatibility formula
    //Note that mut_diff_total/num_matching gives the AVERAGE
    //difference between mutation_nums for any two matching Genes
    //in the Genome

    //Normalizing for genome size
    //return (disjoint_coeff*(num_disjoint/max_genome_size)+
    //  excess_coeff*(num_excess/max_genome_size)+
    //  mutdiff_coeff*(mut_diff_total/num_matching));


    //Look at disjointedness and excess in the absolute (ignoring size)

    //cout<<"COMPAT: size = "<<max_genome_size<<" disjoint = "<<num_disjoint<<" excess = "<<num_excess<<" diff = "<<mut_diff_total<<"  TOTAL = "<<(disjoint_coeff*(num_disjoint/1.0)+excess_coeff*(num_excess/1.0)+mutdiff_coeff*(mut_diff_total/num_matching))<<endl;

    return (disjoint_coeff*(num_disjoint/1.0)+excess_coeff*(num_excess/1.0)
        +mutdiff_coeff*(mut_diff_total/num_matching));
}

F64 Genome::trait_compare(TraitPtr t1, TraitPtr t2)
{

    S32 id1=t1->trait_id;
    S32 id2=t2->trait_id;
    S32 count;
    F64 params_diff=0.0; //Measures parameter difference

    //See if traits represent different fundamental types of connections
    if ((id1==1)&&(id2>=2))
    {
        return 0.5;
    }
    else if ((id2==1)&&(id1>=2))
    {
        return 0.5;
    }
    //Otherwise, when types are same, compare the actual parameters
    else
    {
        if (id1>=2)
        {
            for (count=0; count<=2; count++)
            {
                params_diff+=fabs(t1->params[count]-t2->params[count]);
            }
            return params_diff/4.0;
        }
        else
            return 0.0; //For type 1, params are not applicable
    }

}

S32 Genome::extrons()
{
    vector<GenePtr>::iterator curgene;
    S32 total=0;

    for (curgene=genes.begin(); curgene!=genes.end(); curgene++)
    {
        if ((*curgene)->enable)
            ++total;
    }

    return total;
}

void Genome::randomize_traits()
{

    S32 numtraits = static_cast<S32>(traits.size());
    S32 traitnum; //number of selected random trait
    vector<NNodePtr>::iterator curnode;
    vector<GenePtr>::iterator curgene;
    vector<TraitPtr>::iterator curtrait;

    //Go through all nodes and randomize their trait pointers
    for (curnode=nodes.begin(); curnode!=nodes.end(); ++curnode)
    {
        traitnum=randint(1, numtraits); //randomize trait
        (*curnode)->trait_id=traitnum;

        curtrait=traits.begin();
        while (((*curtrait)->trait_id)!=traitnum)
            ++curtrait;
        (*curnode)->nodetrait=(*curtrait);

        //if ((*curtrait)==0) cout<<"ERROR: Random trait empty"<<endl;

    }

    //Go through all connections and randomize their trait pointers
    for (curgene=genes.begin(); curgene!=genes.end(); ++curgene)
    {
        traitnum=randint(1, numtraits); //randomize trait
        (*curgene)->lnk->trait_id=traitnum;

        curtrait=traits.begin();
        while (((*curtrait)->trait_id)!=traitnum)
            ++curtrait;
        (*curgene)->lnk->linktrait=(*curtrait);

        //if ((*curtrait)==0) cout<<"ERROR: Random trait empty"<<endl;
    }

}

// For the Sensor Registry
vector<string> Genome::getSensorNames() const
{
    vector<NNodePtr>::const_iterator curnode;
    vector<string> names;

    //Go through all nodes and get their Sensor stuff
    for (curnode = nodes.begin(); curnode != nodes.end(); ++curnode)
    {
        string name = (*curnode)->getSensorName();
        if (!name.empty())
        {
            names.push_back(name);
        }
    }

    return names;
}

vector<string> Genome::getSensorArgs() const
{
    vector<NNodePtr>::const_iterator curnode;
    vector<string> args;

    //Go through all nodes and get their Sensor stuff
    for (curnode = nodes.begin(); curnode != nodes.end(); ++curnode)
    {
        string arg = (*curnode)->getSensorArgs();
        if (!arg.empty())
        {
            args.push_back(arg);
        }
    }

    return args;
}

//Calls special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer 
//num_hidden is only used in type 2
//Saves to filename argument
GenomePtr NEAT::new_Genome_auto(S32 num_in, S32 num_out, S32 num_hidden,
                                S32 type, const std::string& filename)
{
    GenomePtr g(new Genome(num_in,num_out,num_hidden,type));

    //print_Genome_tofile(g,"auto_genome");
    print_Genome_tofile(g, filename);

    return g;
}

void NEAT::print_Genome_tofile(GenomePtr g, const std::string& filename)
{

    ofstream oFile(filename.c_str());

    //Make sure	it worked
    if (!oFile)
    {
        cerr<<"Can't open "<<filename<<" for output"<<endl;
        return;
    }
    g->print_to_file(oFile);

    oFile.close();
}
