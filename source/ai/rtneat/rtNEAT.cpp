#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/SimContext.h"
#include "game/Kernel.h"
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
    /// @param filename name of the file with the initial population genomes
    /// @param param_file file with RTNEAT parameters to load
    /// @param population_size size of the population to construct
    /// @param reward_info the specifications for the multidimensional reward
    /// @param generational if true then run generational NEAT; otherwise run realtime NEAT
    RTNEAT::RTNEAT(const std::string& param_file,
                   const std::string& filename,
                   size_t population_size,
                   size_t time_between_evolutions)
        : mPopulation()
        , mOffspringCount(population_size)
        , mSpawnTickCount(0)
        , mEvolutionTickCount(0)
        , mTotalUnitsDeleted(0)
        , mTimeBetweenEvolutions(time_between_evolutions)
    {
        NEAT::load_neat_params(Kernel::findResource(param_file));
        NEAT::pop_size = population_size;
        std::string pop_fname = Kernel::findResource(filename);
        mPopulation.reset(new Population(pop_fname, population_size));
        AssertMsg(mPopulation, "initial population creation failed");
        mOffspringCount = mPopulation->organisms.size();
        AssertMsg(mOffspringCount == population_size, "population has " << mOffspringCount << " organisms instead of " << population_size);
    }

    /// Constructor
    /// @param param_file RTNEAT parameter file
    /// @param inputs number of inputs
    /// @param outputs number of outputs
    /// @param population_size size of the population to construct
    /// @param noise variance of the Gaussian used to assign initial weights
    /// @param reward_info the specifications for the multidimensional reward
    /// @param generational if true then run generational NEAT; otherwise run realtime NEAT
    RTNEAT::RTNEAT(const std::string& param_file,
                   size_t inputs,
                   size_t outputs,
                   size_t population_size,
                   F32 noise,
                   size_t time_between_evolutions)
        : mPopulation()
        , mOffspringCount(0)
        , mSpawnTickCount(0)
        , mEvolutionTickCount(0)
        , mTotalUnitsDeleted(0)
        , mTimeBetweenEvolutions(time_between_evolutions)
    {
        NEAT::load_neat_params(Kernel::findResource(param_file));
        NEAT::pop_size = population_size;
        GenomePtr genome(new Genome(inputs, outputs, 0, 0));
        mPopulation.reset(new Population(genome, population_size, noise));
        AssertMsg(mPopulation, "initial population creation failed");
        mOffspringCount = mPopulation->organisms.size();
        AssertMsg(mOffspringCount == population_size, "population has " << mOffspringCount << " organisms instead of " << population_size);
    }

    /// Destructor
    RTNEAT::~RTNEAT()
    {
    }

    /// load the population from a file
    bool RTNEAT::load_population(const std::string& pop_file)
    {
        std::string fname = Kernel::findResource(pop_file, false);
        mPopulation.reset(new Population(fname));
        return true;
    }

    /// save a population to a file
    std::string RTNEAT::save_population(const std::string& pop_file)
    {
        // try looking for the filename as is
        std::string fname = pop_file;
        std::ofstream output(fname.c_str());
        if (!output) {
            // try again with our findResource method
           std::string fname = Kernel::findResource(pop_file, false);
           output.open(fname.c_str());
        }
        if (!output) {
            LOG_ERROR("Could not open file: " << fname);
            return "";
        }
        else
        {
            LOG_F_MSG("ai.rtneat", "Saving population to file: " << fname);
            //output << mPopulation;
            mPopulation->print_to_file(output);
            output.close();
            return fname;
        }
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

    PyOrganismPtr RTNEAT::reproduce_one()
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
        return PyOrganismPtr(new PyOrganism(new_org));
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

    py::list RTNEAT::get_organisms()
    {
        py::list l;
        std::vector<OrganismPtr>::const_iterator iter;
        for (iter=mPopulation->organisms.begin();iter!=mPopulation->organisms.end();++iter)
        {
            PyOrganismPtr p(new PyOrganism(*iter));
            l.append(p);
        }
        return l;
    }

    /// the id of the species of the organism
    int PyOrganism::GetSpeciesId() const
    {
        return mOrganism->species.lock()->id;
    }
    /// load sensor values into the network
    void PyNetwork::load_sensors(py::list l)
    {
        std::vector<double> sensors;
        for (py::ssize_t i = 0; i < py::len(l); ++i)
            {
                sensors.push_back(py::extract<double>(l[i]));
            }
        mNetwork->load_sensors(sensors);
    }

    /// load error values into the network
    void PyNetwork::load_errors(py::list l)
    {
        std::vector<double> errors;
        for (py::ssize_t i = 0; i < py::len(l); ++i)
            {
                errors.push_back(py::extract<double>(l[i]));
            }
        mNetwork->load_errors(errors);
    }

    /// get output values from the network
    py::list PyNetwork::get_outputs()
    {
        py::list l;
        std::vector<NNodePtr>::const_iterator iter;
        for (iter = mNetwork->outputs.begin(); iter != mNetwork->outputs.end(); ++iter)
            {
                l.append((*iter)->get_active_out());
            }
        return l;
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
