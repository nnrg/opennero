#include "ai/rtneat/advice.h"
#include "ai/rtneat/rtNEAT.h"
#include "advice/advice_rep.h"
#include "rtneat/nnode.h"
#include "rtneat/gene.h"
#include "scripting/scriptIncludes.h"

/// yacc/lex parsing function defined in advice/advice.y
extern Rules* yyParseAdvice(const char* advice, U32 numSensors, U32 numActions, Agent::Type type,
                            const FeatureVectorInfo& sensorBoundsNetwork, const FeatureVectorInfo& sensorBoundsAdvice);

namespace OpenNero
{
    using namespace NEAT;

    Advice::Advice(const char* advice, RTNEAT& rtneat, S32 numInputs, S32 numOutputs, bool newOutputs,
                   const FeatureVectorInfo& inputBoundsNetwork, const FeatureVectorInfo& inputBoundsAdvice) :
            mRTNEAT(rtneat), mNumInputs(numInputs), mNumOutputs(numOutputs), mGenome(), mNewOutputs(newOutputs)
    {
        // Parse the advice.
        Rules* parse_result = yyParseAdvice(advice, mNumInputs, mNumOutputs, Agent::eEvolved, inputBoundsNetwork, inputBoundsAdvice);
        if (parse_result == NULL) {
            return;
        }

        PopulationPtr population = mRTNEAT.get_population();

        // Build a genome with only the input and output nodes.  The node ids are the same as
        // those initialized by rtneat.
        std::vector<NNodePtr> nodes;
        S32 ncount;
        for (ncount=1; ncount<=mNumInputs; ncount++) {
            if (ncount<mNumInputs) {
                nodes.push_back(NNodePtr(new NNode(SENSOR,ncount,INPUT)));
            }
            else {
                nodes.push_back(NNodePtr(new NNode(SENSOR,ncount,BIAS)));
            }
        }
        for (ncount=mNumInputs+1; ncount<=mNumInputs+mNumOutputs; ncount++) {
            nodes.push_back(NNodePtr(new NNode(NEURON,ncount,OUTPUT)));
        }

        // If flag for new outputs is set, then construct new output nodes; otherwise both
        // old and new output nodes are the same.
        for (ncount=mNumInputs+1; ncount<=mNumInputs+mNumOutputs; ncount++) {
            NNodePtr newout;
            if (mNewOutputs) {
                newout = NNodePtr(new NNode(NEURON,population->cur_node_id++,OUTPUT));
            }
            else {
                // make copy to prevent invalidation of reference when vector is resized.
                newout = nodes[ncount-1];
            }
            nodes.push_back(newout);
        }

        mGenome.reset(new Genome(0, std::vector<TraitPtr>(), nodes, std::vector<GenePtr>(), std::vector<FactorPtr>()));

        parse_result->print();
        printf("\n");

        // Note that buildRepresentation() is called below with ionode = biasnode, which has the logical value true.
        // ionode is a vector element passed by reference, so pass a copy to prevent invalidation of reference when
        // vector is resized.
        NNodePtr ionode = mGenome->nodes[mNumInputs-1];
        std::vector<NNodePtr> variables(nodes);  // nodes that have variable names associated with them.
        parse_result->buildRepresentation(population, mGenome->nodes[mNumInputs-1], mGenome, variables, ionode);
        delete parse_result;
    }


    void Advice::splice_advice_pop() {
        // Splice the genome constructed above into the genomes of the population.
        PopulationPtr population = mRTNEAT.get_population();
        std::vector<OrganismPtr>& organisms = population->organisms;
        for (S32 i = 0; i < organisms.size(); i++) {
            splice_advice_org_helper(organisms[i]);
        }
    }


    void Advice::splice_advice_org_helper(OrganismPtr org) {
        // splice advice genome on to the original genome
        splice_genome(mGenome, org->gnome);
        //org->gnome->print_to_filename("genome.txt");
        org->update_phenotype();
    }


    void Advice::splice_advice_org(PyOrganismPtr org) {
        splice_advice_org_helper(org->mOrganism);
    }


    void Advice::splice_genome(GenomePtr src, GenomePtr dst) {
        // Copy and add any new nodes to the destination genome.
        S32 beginCopy = mNewOutputs ? (mNumInputs+mNumOutputs) : (mNumInputs+2*mNumOutputs);
        for (S32 i = beginCopy; i < src->nodes.size(); i++) {
            NNodePtr newnode(new NNode(src->nodes[i], TraitPtr()));
            src->nodes[i]->dup = newnode;  // Used in copying genes below.
            dst->nodes.push_back(newnode);
        }

        // Find the index where old output nodes begin in the dst genome.
        S32 beginOldOut = mNumInputs;
        while (dst->nodes[beginOldOut]->gen_node_label != OUTPUT) beginOldOut++;

        // Copy and add the new genes to the destination genome.
        for (S32 i = 0; i < src->genes.size(); i++) {
            // Find the nodes connected by the gene's link and translate them to the
            // nodes that they refer to in the destination genome.
            S32 inode_id = src->genes[i]->lnk->get_in_node()->node_id;
            S32 onode_id = src->genes[i]->lnk->get_out_node()->node_id;
            NNodePtr inode;
            if (inode_id <= mNumInputs) {
                inode = dst->nodes[inode_id-1];
            }
            else if (inode_id <= mNumInputs+mNumOutputs) {
                inode = dst->nodes[beginOldOut+inode_id-mNumInputs-1];
            }
            else {
                inode = src->genes[i]->lnk->get_in_node()->dup;
            }
            NNodePtr onode;
            if (onode_id <= mNumInputs) {
                onode = dst->nodes[onode_id-1];
            }
            else if (onode_id <= mNumInputs+mNumOutputs) {
                onode = dst->nodes[beginOldOut+onode_id-mNumInputs-1];
            }
            else {
                onode = src->genes[i]->lnk->get_out_node()->dup;
            }

            // Create and add new gene to the destination genome.
            GenePtr newgene(new Gene(src->genes[i], dst->traits[0], inode, onode));
            dst->genes.push_back(newgene);
        }

        if (mNewOutputs) {
            // Change old outputs to hidden.
            for (S32 i = 0; i < mNumOutputs; i++) {
                dst->nodes[beginOldOut+i]->gen_node_label = HIDDEN;
            }
        }
    }
}
