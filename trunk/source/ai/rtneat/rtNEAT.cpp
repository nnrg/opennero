#include "core/Common.h"
#include "game/SimEntity.h"
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

    namespace {
        const size_t kNumSpeciesTarget = 5; ///< target number of species in the population
        const double kCompatMod = 0.1; ///< compatibility threshold modifier
        const double kMinCompatThreshold = 0.3; // minimum species compatibility threshold

        /// compare two organisms by fitness
        bool fitness_less(OrganismPtr a, OrganismPtr b)
        {
            return a->fitness < b->fitness;
        }
    }

    /// Constructor
    /// @param filename name of the file with the initial population genomes
    /// @param param_file file with RTNEAT parameters to load
    /// @param population_size size of the population to construct
    RTNEAT::RTNEAT(const std::string& filename, const std::string& param_file, size_t population_size)
        : mPopulation()
        , mEvalQueue()
        , mOffspringCount(0)
    {
        NEAT::load_neat_params(Kernel::findResource(param_file));
        NEAT::pop_size = population_size;
        NEAT::time_alive_minimum = 1; // organisms cannot be removed before the are evaluated at least once
        mPopulation.reset(new Population(filename, population_size));
        AssertMsg(mPopulation, "initial population creation failed");
        mOffspringCount = mPopulation->organisms.size();
        AssertMsg(mOffspringCount == population_size, "population has " << mOffspringCount << " organisms instead of " << population_size);
        for (size_t i = 0; i < mPopulation->organisms.size(); ++i)
        {
            mEvalQueue.push(mPopulation->organisms[i]);
        }
    }

    /// Constructor
    /// @param param_file RTNEAT parameter file
    /// @param inputs number of inputs
    /// @param outputs number of outputs
    /// @param population_size size of the population to construct
    /// @param noise variance of the Gaussian used to assign initial weights
    RTNEAT::RTNEAT(const std::string& param_file, size_t inputs, size_t outputs, size_t population_size, F32 noise)
        : mPopulation()
        , mEvalQueue()
        , mOffspringCount(0)
    {
        NEAT::load_neat_params(Kernel::findResource(param_file));
        NEAT::pop_size = population_size;
        NEAT::time_alive_minimum = 1; // organisms cannot be removed before the are evaluated at least once
        GenomePtr genome(new Genome(inputs, outputs, 0, 0));
        mPopulation.reset(new Population(genome, population_size, noise));
        AssertMsg(mPopulation, "initial population creation failed");
        mOffspringCount = mPopulation->organisms.size();
        AssertMsg(mOffspringCount == population_size, "population has " << mOffspringCount << " organisms instead of " << population_size);
        for (size_t i = 0; i < mPopulation->organisms.size(); ++i)
        {
            mEvalQueue.push(mPopulation->organisms[i]);
        }
    }
    
    boost::python::list RTNEAT::get_population_ids()
    {
        boost::python::list result;
        vector<OrganismPtr>::iterator org_iter;
        for (org_iter = mPopulation->organisms.begin(); 
             org_iter != mPopulation->organisms.end(); 
             ++org_iter) {
            if ((*org_iter)->time_alive > 0)
            {
                result.append((*org_iter)->gnome->genome_id);
            }
            mPopulation->reassign_species(*org_iter);
        }
        return result;
    }

    /// Destructor
    RTNEAT::~RTNEAT()
    {

    }

    PyOrganismPtr RTNEAT::next_organism(float prob)
    {
        OrganismPtr org;
        
        if (!mEvalQueue.empty())
        {
            org = mEvalQueue.front();
            mEvalQueue.pop();
        }
        else if (RANDOM.randF() < prob)
        {
            vector<OrganismPtr>::iterator most = max_element(mPopulation->organisms.begin(), mPopulation->organisms.end(), fitness_less);
            org = *most;
        }
        else
        {
            vector<OrganismPtr>::iterator least = min_element(mPopulation->organisms.begin(), mPopulation->organisms.end(), fitness_less);
            AssertMsg(least != mPopulation->organisms.end(), "lowest fitness organism not found");
            double least_fitness = (*least)->fitness;
            double max_fitness = least_fitness;
            size_t effective_pop_size = 0;
            vector<OrganismPtr>::iterator org_iter;
            for (org_iter = mPopulation->organisms.begin(); org_iter != mPopulation->organisms.end(); ++org_iter)
            {
                if ((*org_iter)->time_alive > 0) {
                    double fitness = (*org_iter)->fitness;
                    if (fitness > max_fitness) 
                    {
                        max_fitness = fitness;
                    }
                    effective_pop_size += 1;
                    (*org_iter)->fitness -= least_fitness;
                }
            }
            LOG_F_DEBUG("ai", 
                "Effective rtNEAT population of size: " << effective_pop_size <<
                ", min. fitness: " << least_fitness <<
                ", max. fitness: " << max_fitness);
            OrganismPtr removed = mPopulation->remove_worst();
            if (removed) {
                SpeciesPtr parent = mPopulation->choose_parent_species();

                // reproduce
                org = parent->reproduce_one(static_cast<S32>(mOffspringCount++), mPopulation, mPopulation->species, 0, 0);
                AssertMsg(org, "Organism did not reproduce correctly");

                size_t num_species = mPopulation->species.size(); // number of species in the population

                // adjust species boundaries to keep their number close to target
                if (num_species < kNumSpeciesTarget)
                    NEAT::compat_threshold -= kCompatMod;
                else if (num_species > kNumSpeciesTarget)
                    NEAT::compat_threshold += kCompatMod;

                if (NEAT::compat_threshold < kMinCompatThreshold)
                    NEAT::compat_threshold = kMinCompatThreshold;

                //Go through entire population, reassigning organisms to new species
                for (org_iter = mPopulation->organisms.begin(); org_iter != mPopulation->organisms.end(); ++org_iter) {
                    assert((*org_iter)->gnome);
                    if ((*org_iter)->time_alive > 0)
                    {
 
                        (*org_iter)->fitness += least_fitness;
                    }
                    mPopulation->reassign_species(*org_iter);
                } 
            }
        }

        AssertMsg(org, "No rtNEAT organism found!");

        return PyOrganismPtr(new PyOrganism(org));
    }

    /// save a population to a file
    bool RTNEAT::save_population(const std::string& pop_file)
    {
        std::string fname = Kernel::findResource(pop_file, false);
        std::ofstream output(fname.c_str());
        if (!output) {
            LOG_ERROR("Could not open file " << fname);
            return false;
        }
        else
        {
            LOG_F_DEBUG("rtNEAT", "Saving population to " << fname);
            //output << mPopulation;
            mPopulation->print_to_file(output);
            output.close();
            return true;
        }
    }

    std::ostream& operator<<(std::ostream& output, const PyNetwork& net)
    {
        output << net.mNetwork;
        return output;
    }

    std::ostream& operator<<(std::ostream& output, const PyOrganism& org)
    {
        output << org.mOrganism;
        return output;
    }
}
