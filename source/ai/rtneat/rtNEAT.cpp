#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/SimContext.h"
#include "game/Kernel.h"
#include "ai/AIObject.h"
#include "ai/rtneat/rtNEAT.h"
#include "rtneat/population.h"
#include "rtneat/network.h"
#include "scripting/scriptIncludes.h"
#include "math/Random.h"
#include <ostream>
#include <fstream>

namespace OpenNero
{

    /// @cond
    BOOST_SHARED_DECL(SimEntity);
    /// @endcond

    using namespace NEAT;
    /// Constructor
    /// @param param_file RTNEAT parameter file
    /// @param population RTNEAT Population to manage
    /// @param min_lifetime minimum time alive before killing
    /// @param time_between_evolution number of steps between kills
    RTNEAT::RTNEAT(const std::string& param_file,
                   PopulationPtr population,
                   size_t min_lifetime,
                   size_t time_between_evolutions)
        : mPopulation(population)
        , mOffspringCount(0)
        , mSpawnTickCount(0)
        , mEvolutionTickCount(0)
        , mTotalUnitsDeleted(0)
        , mTimeBetweenEvolutions(time_between_evolutions)
    {
        NEAT::load_neat_params(Kernel::findResource(param_file));
        NEAT::time_alive_minimum = min_lifetime;
        NEAT::pop_size = mPopulation->organisms.size();
        mOffspringCount = mPopulation->organisms.size();
    }

    /// Destructor
    RTNEAT::~RTNEAT()
    {
    }

    void RTNEAT::ProcessTick( float32_t incAmt )
    {
        // Increment the spawn tick and evolution tick counters
        ++mSpawnTickCount;
        ++mEvolutionTickCount;

        // If the total number of units spawned so far exceeds the threshold value AND enough
        // ticks have passed since the last evolution, then a new evolution may commence.
        if (mEvolutionTickCount >= mTimeBetweenEvolutions)
        {
            //Judgment day!
            remove_worst();
            mEvolutionTickCount = 0;
        }
    }

    void RTNEAT::remove_worst()
    {
        // Remove the worst organism
        OrganismPtr deadorg = mPopulation->remove_worst();

        // Sometimes, if all organisms are beneath the minimum "time alive" threshold, no organism will be removed
        // If an organism *was* actually removed, then we can proceed with replacing it via the evolutionary process
        if (deadorg) {
            LOG_F_DEBUG("ai.rtneat.evolve", "deadorg: " << deadorg->gnome->genome_id);
            //Mark organism for elimination
            deadorg->eliminate = true;

            // Increment the deletion counter
            ++mTotalUnitsDeleted;
        }
    }

    OrganismPtr RTNEAT::reproduce_one()
    {
        //We can try to keep the number of species constant at this number
        U32 num_species_target=4;
        U32 compat_adjust_frequency = mPopulation->organisms.size()/10;
        if (compat_adjust_frequency < 1)
            compat_adjust_frequency = 1;

        NEAT::OrganismPtr new_org;

        // Estimate all species' fitnesses
        for (vector<SpeciesPtr>::iterator curspec = (mPopulation->species).begin(); curspec != (mPopulation->species).end(); ++curspec) {
            (*curspec)->estimate_average();
        }

        // Reproduce a single new organism to replace the one killed off.
        new_org = (mPopulation->choose_parent_species())->reproduce_one(mOffspringCount, mPopulation, mPopulation->species, 0,0);
        //}
        ++mOffspringCount;

        //Every compat_adjust_frequency reproductions, reassign the population to new species
        if (mOffspringCount % compat_adjust_frequency == 0) {

            U32 num_species = mPopulation->species.size();
            F64 compat_mod=0.1;  //Modify compat thresh to control speciation

            // This tinkers with the compatibility threshold, which normally would be held constant
            if (num_species < num_species_target)
                NEAT::compat_threshold -= compat_mod;
            else if (num_species > num_species_target)
                NEAT::compat_threshold += compat_mod;

            if (NEAT::compat_threshold < 0.3)
                NEAT::compat_threshold = 0.3;

            //Go through entire population, reassigning organisms to new species
            vector<OrganismPtr>::iterator curorg = mPopulation->organisms.begin();
            vector<OrganismPtr>::iterator orgend = mPopulation->organisms.end();
            for (; curorg != orgend; ++curorg) {
                mPopulation->reassign_species(*curorg);
            }
        }
        return new_org;
    }

    /// set the lifetime so that we can ensure that the units have been alive
    /// at least that long before evaluating them
    void RTNEAT::set_lifetime(size_t lifetime)
    {
        if (lifetime > 0) {
            NEAT::time_alive_minimum = lifetime;
            LOG_F_DEBUG("ai.rtneat",
                "time_alive_minimum: " << NEAT::time_alive_minimum);
        }
    }
}