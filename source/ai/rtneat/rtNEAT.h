/// @file
/// A Python interface for the rtNEAT learning algorithm.

#ifndef _OPENNERO_AI_RTNEAT_RTNEAT_H_
#define _OPENNERO_AI_RTNEAT_RTNEAT_H_

#include "core/Preprocessor.h"
#include "rtneat/population.h"
#include "scripting/scripting.h"
#include "ai/AI.h"
#include <string>
#include <iostream>
#include <boost/python.hpp>

namespace OpenNero
{
    using namespace NEAT;
    using namespace std;
    namespace py = boost::python;

    /// @cond
    BOOST_SHARED_DECL(RTNEAT);
    /// @endcond

    /// An interface for the RTNEAT learning algorithm
    class RTNEAT : public AI {
        PopulationPtr mPopulation;        ///< population of organisms
        size_t mOffspringCount;           ///< number of reproductions so far
		size_t mSpawnTickCount;           ///< number of spawn ticks
		size_t mEvolutionTickCount;       ///< number of evolution ticks
        size_t mTotalUnitsDeleted;        ///< total units deleted
        size_t mUnitsToDeleteBeforeFirstJudgment; ///< number of units to delete before judging
        size_t mTimeBetweenEvolutions;    ///< time (in ticks) between rounds of evolution

    public:
        /// Constructor
        /// @param param_file RTNEAT parameter file
        /// @param inputs number of inputs
        /// @param outputs number of outputs
        /// @param population_size size of the population to construct
        /// @param noise variance of the Gaussian used to assign initial weights
        /// @param time_between_evolutions time between evolution steps
        RTNEAT(const std::string& param_file,
               PopulationPtr population,
               size_t min_lifetime,
               size_t time_between_evolutions);

        /// Destructor
        ~RTNEAT();

        /// Called every step by the OpenNERO system
        virtual void ProcessTick( float32_t incAmt );

        /// set the lifetime so that we can ensure that the units have been alive
        /// at least that long before evaluating them
        void set_lifetime(size_t lifetime);

        /// Reproduces a new organism to replace a previously killed one
        OrganismPtr reproduce_one();    

        /// load info about this AI from the object template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data) { return true; }

	private:
		/// evolution step that potentially kills a low-fitness organism
		void remove_worst();
    };
}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
