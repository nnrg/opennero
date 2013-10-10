/// @file
/// An interface for providing advice to rtNEAT networks.

#ifndef _OPENNERO_AI_RTNEAT_ADVICE_H_
#define _OPENNERO_AI_RTNEAT_ADVICE_H_

#include "rtneat/genome.h"
#include "ai/rtneat/rtNEAT.h"
#include "ai/AI.h"


namespace OpenNero
{
    using namespace NEAT;
    using namespace std;

    /// An interface for providing advice to rtNEAT networks.
    class Advice {
    public:
        /// Constructor to parse the given advice into its genome representation
        /// @param advice the string containing advice
        /// @param rtneat the RTNEAT instance for which advice is given
        /// @param numInputs number of network inputs
        /// @param numOutputs number of network outputs
        /// @param newOutputs new output units are used for the advice representation
        ///     if true; old outputs are used otherwise
        /// @param inputBoundsNetwork bounds on the inputs in the network
        /// @param inputBoundsAdvice bounds on the inputs in the advice language
        Advice(const char* advice, RTNEAT& rtneat, S32 numInputs, S32 numOutputs, bool newOutputs,
               const FeatureVectorInfo& inputBoundsNetwork, const FeatureVectorInfo& inputBoundsAdvice);
        
        /// Destructor
        ~Advice() {}

        /// @brief Splice the genome representation of the previously given advice into
        ///     every genome in the population
        void splice_advice_pop();

        /// @brief Splice the genome representation of the previously given advice into
        ///     the given organism
        void splice_advice_org(PyOrganismPtr org);

    private:
        RTNEAT& mRTNEAT;
        S32 mNumInputs;
        S32 mNumOutputs;
        GenomePtr mGenome;
        bool mNewOutputs;

        /// @brief Splice one genome into another
        /// @param src the source genome
        /// @param dst the destination genome
        void splice_genome(GenomePtr src, GenomePtr dst);

        /// @brief Splice the genome representation of the previously given advice into
        ///     the given organism
        void splice_advice_org_helper(OrganismPtr org);
    };

}

#endif /* _OPENNERO_AI_RTNEAT_ADVICE_H_ */
