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
    
    /// get the organism currently assigned to the agent
    PyOrganismPtr RTNEAT::get_organism(AgentBrainPtr agent)
    {
        AgentToOrganismMap::const_iterator found;
        found = mAgentsToOrganisms.find(agent);
        if (found != mAgentsToOrganisms.end())
        {
            return found->second;
        }
        else
        {
            PyOrganismPtr org(new PyOrganism(mEvalQueue.front()));
            mAgentsToOrganisms[agent] = org;
            return org;
        }

    }
    
    /// release the organism that was being used by the agent
    void RTNEAT::release_organism(AgentBrainPtr agent)
    {
        AgentToOrganismMap::const_iterator found;
        found = mAgentsToOrganisms.find(agent);
        Assert(found != mAgentsToOrganisms.end());
        mEvalQueue.push(found->second->GetOrganism());
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
    
    // 

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
