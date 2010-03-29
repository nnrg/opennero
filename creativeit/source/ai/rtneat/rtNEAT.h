/// @file
/// A Python interface for the rtNEAT learning algorithm.

#ifndef _OPENNERO_AI_RTNEAT_RTNEAT_H_
#define _OPENNERO_AI_RTNEAT_RTNEAT_H_

#include "core/Preprocessor.h"
#include "rtneat/population.h"
#include "scripting/scripting.h"
#include "ai/AI.h"
#include <string>
#include <set>
#include <queue>

namespace OpenNero
{
    using namespace NEAT;
    using namespace std;

    typedef queue<OrganismPtr> OrganismQueue;

    /// @cond
    BOOST_SHARED_DECL(RTNEAT);
    BOOST_SHARED_DECL(PyNetwork);
    BOOST_SHARED_DECL(PyOrganism);
    /// @endcond

    /// An interface for the RTNEAT learning algorithm
    class RTNEAT : public AI {
        PopulationPtr mPopulation;        ///< population of organisms on the field
        OrganismQueue mEvalQueue;         ///< queue of organisms to be evaluated
        size_t mOffspringCount;           ///< number of reproductions so far
    public:
        /// Constructor
        /// @param filename name of the file with the initial population genomes
        /// @param param_file file with RTNEAT parameters to load
        /// @param population_size size of the population to construct
        RTNEAT(const std::string& filename, const std::string& param_file, size_t population_size);

        /// Constructor
        /// @param param_file RTNEAT parameter file
        /// @param inputs number of inputs
        /// @param outputs number of outputs
        /// @param population_size size of the population to construct
        /// @param noise variance of the Gaussian used to assign initial weights
        RTNEAT(const std::string& param_file, size_t inputs, size_t outputs, size_t population_size, F32 noise);

        /// Destructor
        ~RTNEAT();

        /// get the next organism to be evaluated
        PyOrganismPtr next_organism(float prob);

        /// save the current population to a file
        bool save_population(const std::string& population_file);

        /// load a population from a file
        bool load_population(const std::string& population_file);
        
        /// get the list of IDs in the currently scored population
        boost::python::list get_population_ids();

        /// load info about this AI from the object template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data) { return true; }
    };

}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
