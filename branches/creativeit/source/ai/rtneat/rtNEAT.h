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
    namespace py = boost::python;

    typedef queue<OrganismPtr> OrganismQueue;

    /// @cond
    BOOST_SHARED_DECL(RTNEAT);
    BOOST_SHARED_DECL(PyNetwork);
    BOOST_SHARED_DECL(PyOrganism);
    BOOST_SHARED_DECL(Advice);
    /// @endcond

    /// An interface for the RTNEAT learning algorithm
    class RTNEAT : public AI {
        PopulationPtr mPopulation;        ///< population of organisms on the field
        OrganismQueue mEvalQueue;         ///< queue of organisms to be evaluated
        size_t mOffspringCount;           ///< number of reproductions so far
        AdvicePtr mAdvice;                ///< user provided advice
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

        /// advice provided by the user
        AdvicePtr get_advice() const { return mAdvice; }
        void set_advice(AdvicePtr advice) { mAdvice = advice; }

        /// get the next organism to be evaluated
        PyOrganismPtr next_organism(PyOrganismPtr org);

        /// evolve and return the next organism to be evaluated
        PyOrganismPtr evolve_next_organism();

        /// save the current population to a file
        bool save_population(const std::string& population_file);

        /// load a population from a file
        bool load_population(const std::string& population_file);

        /// return the population pointer
        PopulationPtr get_population() { return mPopulation; }

        /// get the list of IDs in the currently scored population
        boost::python::list get_population_ids();

        /// load info about this AI from the object template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data) { return true; }
    };

    /// A Python wrapper for the Network class with a simple interface for forward prop
    class PyNetwork
    {
        NetworkPtr mNetwork;
    public:
        /// Constructor
        PyNetwork(NetworkPtr net) : mNetwork(net) {}

        /// flush the network by clearing its internal state
        void flush() { mNetwork->flush(); }

        /// load sensor values into the network
        void load_sensors(py::list l);

        /// load error values into the network
        void load_errors(py::list l);

        /// print the activations of all nodes.
        void show_activation() { mNetwork->show_activation(); }

        /// print the activations of input nodes.
        void show_input() { mNetwork->show_input(); }

        /// print the activations of output nodes.
        void show_output() { mNetwork->show_output(); }

        /// activate the network for one or more steps until signal reaches output
        bool activate() { return mNetwork->activate(); }

        /// back-propagates error in the network for one or more steps until signal reaches input
        bool backprop() { return mNetwork->backprop(); }

        /// print connections and their weights with carriage returns
        void print_links() { mNetwork->print_links(); }

        /// get output values from the network
        py::list get_outputs();

        friend std::ostream& operator<<(std::ostream& output, const PyNetwork& net);
    };
    
    /// A Python wrapper for the Organism class with a simple interface for fitness and network
    class PyOrganism
    {
        OrganismPtr mOrganism; ///< internal organism data
    public:
        /// create a Python wrapper for an organism object
        PyOrganism(OrganismPtr org) : mOrganism(org) {}
        
        /// set the current fitness of this organism
        void SetFitness(double fitness) { mOrganism->fitness = fitness; }
        
        /// get the current fitness of this organism
        double GetFitness() const { return mOrganism->fitness; }
        
        /// set the time alive of this organism
		void SetTimeAlive(int time_alive) { mOrganism->time_alive = time_alive; }
        
        /// get the time alive of this organism
        int GetTimeAlive() const { return mOrganism->time_alive; }
        
        /// return the network of this organism
        PyNetworkPtr GetNetwork() const { return PyNetworkPtr(new PyNetwork(mOrganism->net)); }
        
        /// update the genotype of this organism
        void UpdateGenotype() { mOrganism->update_genotype(); }
        
        /// get the genome ID of this organism
        int GetId() const { return mOrganism->gnome->genome_id; }
        
        /// save this organism to a file
        bool Save(const std::string& fname) const { return mOrganism->print_to_file(fname); }

        friend std::ostream& operator<<(std::ostream& output, const PyOrganism& net);
        friend class Advice;
    };

}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
