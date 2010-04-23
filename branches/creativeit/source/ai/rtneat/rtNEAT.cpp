#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/Kernel.h"
#include "ai/AIObject.h"
#include "ai/rtneat/rtNEAT.h"
#include "ai/rtneat/advice.h"
#include "rtneat/population.h"
#include "rtneat/network.h"
#include "math/Random.h"
#include <ostream>
#include <fstream>

namespace OpenNero
{
 
    /// @cond
    BOOST_SHARED_DECL(SimEntity);
    /// @endcond

    using namespace NEAT;
    namespace py = boost::python;

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
    
    py::list RTNEAT::get_population_ids()
    {
        py::list result;
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
    
    PyOrganismPtr RTNEAT::next_organism(PyOrganismPtr org) {
        static size_t count = 0; // count of organisms spliced with advice

        // If user has provided any advice, splice it into the organism
        // and return it; otherwise, evolve and return the next organism.
        if (mAdvice && count < mPopulation->organisms.size()) 
        {
            if (org == NULL) 
            {
                org = evolve_next_organism();
            }
            mAdvice->splice_advice_org(org);
            count++;
            if (count == mPopulation->organisms.size()) 
            {
                mAdvice.reset();
                count = 0;
            }
            return org;
        }
        else
        {
            return evolve_next_organism();
        }
    }

    PyOrganismPtr RTNEAT::evolve_next_organism()
    {
        OrganismPtr org;
        static int index = 0;   // index of current organism in population
        static OrganismPtr elite_org;   // the organism with highest fitness in the population

        if (!mEvalQueue.empty())
        {
            org = mEvalQueue.front();
            mEvalQueue.pop();
        }
        else
        {
            // Find organism with highest fitness before we change the population.
            if (index == 0) {
                vector<OrganismPtr>::iterator elite = max_element(mPopulation->organisms.begin(), mPopulation->organisms.end(), fitness_less);
                AssertMsg(elite != mPopulation->organisms.end(), "highest fitness organism not found");
                LOG_F_DEBUG("ai", "highest fitness: " << (*elite)->fitness);
                elite_org = *elite;
            }

            vector<OrganismPtr>::iterator least = min_element(mPopulation->organisms.begin(), mPopulation->organisms.end(), fitness_less);
            AssertMsg(least != mPopulation->organisms.end(), "lowest fitness organism not found");
            double least_fitness = (*least)->fitness;
            LOG_F_DEBUG("ai", "lowest fitness: " << least_fitness);
            vector<OrganismPtr>::iterator org_iter;
            for (org_iter = mPopulation->organisms.begin(); org_iter != mPopulation->organisms.end(); ++org_iter) {
                if ((*org_iter)->time_alive > 0) {
                    (*org_iter)->fitness -= least_fitness;
                }
            }

            OrganismPtr removed = mPopulation->remove_worst();
            if (removed) {
                SpeciesPtr parent = mPopulation->choose_parent_species();
                S32 generation = static_cast<S32>(mOffspringCount++);

                // use copy of elite organism for first individual in the population and reproduce
                // for the other individuals
                if (index == 0) {
                    org.reset(new Organism(0.0, elite_org->gnome->duplicate(generation), generation));
                    mPopulation->add_organism(org);
                }
                else {
                    org = parent->reproduce_one(generation, mPopulation, mPopulation->species, 0, 0);
                    AssertMsg(org, "Organism did not reproduce correctly");
                }

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
                    if ((*org_iter)->time_alive > 0) {
                        (*org_iter)->fitness += least_fitness;
                    }
                    mPopulation->reassign_species(*org_iter);
                }
            }

            index = (index+1)%mPopulation->organisms.size();
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

    using namespace boost::python;

    /// Export RTNEAT related classes and functions to Python
    PYTHON_BINDER(RTNEAT)
    {
        // export Network
        class_<PyNetwork, PyNetworkPtr>("Network", "an artificial neural network", no_init )
            .def("load_sensors", &PyNetwork::load_sensors, "load sensor values into the network")
            .def("load_errors", &PyNetwork::load_errors, "load error values into the network")
            .def("show_activation", &PyNetwork::show_activation, "print the activations of all nodes")
            .def("show_input", &PyNetwork::show_input, "print the activations of input nodes")
            .def("show_output", &PyNetwork::show_output, "print the activations of output nodes")
            .def("activate", &PyNetwork::activate, "activate the network for one or more steps until signal reaches output")
            .def("backprop", &PyNetwork::backprop, "back-propagates error in the network for one or more steps until signal reaches input")
            .def("flush", &PyNetwork::flush, "flush the network by clearing its internal state")
            .def("get_outputs", &PyNetwork::get_outputs, "get output values from the network")
            .def("print_links", &PyNetwork::print_links, "print network connections and their weights")
            .def(self_ns::str(self_ns::self));

        // export Organism
        class_<PyOrganism, PyOrganismPtr>("Organism", "a phenotype and a genotype for a neural network", no_init)
            .add_property("net", &PyOrganism::GetNetwork, "neural network (phenotype)")
            .add_property("id", &PyOrganism::GetId, "evolution-wide unique id of the organism")
            .add_property("fitness", &PyOrganism::GetFitness, &PyOrganism::SetFitness, "organism fitness (non-negative real)")
			.add_property("time_alive", &PyOrganism::GetTimeAlive, &PyOrganism::SetTimeAlive, "organism time alive (integer, non negative)")
            .def("update_genotype", &PyOrganism::UpdateGenotype, "import weights from network to genotype")
            .def("save", &PyOrganism::Save, "save the organism to file")
            .def(self_ns::str(self_ns::self));

        // export AI base class
        class_<AI, AIPtr, noncopyable>("AI", "AI algorithm", no_init);
            
        // export RTNEAT interface
        class_<RTNEAT, bases<AI>, RTNEATPtr>("RTNEAT", init<const std::string&, const std::string&, S32>())
            .def(init<const std::string&, S32, S32, S32, F32>())
            .add_property("advice", &RTNEAT::get_advice, &RTNEAT::set_advice, "user provided advice")
            .def("next_organism", &RTNEAT::next_organism, "evolve a new organism and return it")
            .def("get_population_ids", &RTNEAT::get_population_ids, "get a list of the current genome ids")
            .def("save_population", &RTNEAT::save_population, "save the population to a file");
    }
}
