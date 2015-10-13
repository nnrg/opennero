/// @file
/// A Python interface for the rtNEAT learning algorithm.

#ifndef _OPENNERO_AI_RTNEAT_RTNEAT_H_
#define _OPENNERO_AI_RTNEAT_RTNEAT_H_

#include "core/Preprocessor.h"
#include "rtneat/population.h"
#include "scripting/scripting.h"
#include "ai/AI.h"
#include "ai/Environment.h"
#include "ai/rtneat/ScoreHelper.h"
#include <string>
#include <set>
#include <queue>
#include <iostream>
#include <boost/python.hpp>
#include <boost/bimap.hpp>

namespace OpenNero
{
    using namespace NEAT;
    using namespace std;
    namespace py = boost::python;

    /// @cond
    BOOST_SHARED_DECL(RTNEAT);
    BOOST_SHARED_DECL(PyNetwork);
    BOOST_SHARED_DECL(PyOrganism);
    BOOST_SHARED_DECL(AIObject);
    /// @endcond

    /// A bi-directional map associating AIObjects (bodies) with PyOrganisms (rtNEAT brains)
    typedef boost::bimap<AIObjectPtr, PyOrganismPtr> BrainBodyMap;

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
        /// @param filename name of the file with the initial population genomes
        /// @param param_file file with RTNEAT parameters to load
        /// @param population_size size of the population to construct
        /// @param time_between_evolutions time between evolution steps
        RTNEAT(const std::string& param_file,
               const std::string& filename,
               size_t population_size,
               size_t time_between_evolutions);

        /// Constructor
        /// @param param_file RTNEAT parameter file
        /// @param inputs number of inputs
        /// @param outputs number of outputs
        /// @param population_size size of the population to construct
        /// @param noise variance of the Gaussian used to assign initial weights
        /// @param time_between_evolutions time between evolution steps
        RTNEAT(const std::string& param_file,
               size_t inputs,
               size_t outputs,
               size_t population_size,
               F32 noise,
               size_t time_between_evolutions);

        /// Destructor
        ~RTNEAT();

        /// Called every step by the OpenNERO system
        virtual void ProcessTick( float32_t incAmt );

        /// save the current population to a file
		/// return the name of the file the population was saved to
		std::string save_population(const std::string& population_file);

        /// load a population from a file
        bool load_population(const std::string& population_file);

        /// set the lifetime so that we can ensure that the units have been alive
        /// at least that long before evaluating them
        void set_lifetime(size_t lifetime);

        /// Reproduces a new organism to replace a previously killed one
        PyOrganismPtr reproduce_one();

        /// @return the current population
        PopulationPtr get_population() { return mPopulation; }

        /// @ return list of organisms being evolved
        py::list get_organisms();       

        /// load info about this AI from the object template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data) { return true; }

	private:
		/// evolution step that potentially kills a low-fitness organism
		void remove_worst();
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

        /// operator to push to an output stream
        friend std::ostream& operator<<(std::ostream& output, const PyNetwork& net);
    };


    /// A Python wrapper for the Organism class with a simple interface for fitness and network
    class PyOrganism
    {
        OrganismPtr mOrganism;
    public:
        /// we keep our own champion flag
        bool champion;

		/// constructor for a PyOrganism
        /// @param org rtNEAT organism to wrap
        PyOrganism(OrganismPtr org) :
            mOrganism(org),
            champion(false)
        { }

        /// set the fitness of the organism
        void SetFitness(double fitness) { mOrganism->fitness = fitness; }

        /// get the fitness of the organism
        double GetFitness() const { return mOrganism->fitness; }

		/// get the genome ID of this organism
        int GetId() const { return mOrganism->gnome->genome_id; }

        /// the id of the species of the organism
        int GetSpeciesId() const;

        /// set the amount of time the organism has to live
    	void SetTimeAlive(int time_alive) { mOrganism->time_alive = time_alive; }

        /// get the amount of time that the organism has to live
        int GetTimeAlive() const { return mOrganism->time_alive; }

        /// whether the organism is marked for elimination
        bool GetEliminate() const { return mOrganism->eliminate; }

        /// save this organism to a file
        bool Save(const std::string& fname) const { return mOrganism->print_to_file(fname); }

        /// Get the organism
        OrganismPtr GetOrganism() { return mOrganism; }

        /// Set the organism
        void SetOrganism(OrganismPtr organism) { mOrganism = organism; }

        /// get network of the organism
        PyNetworkPtr GetNetwork() const { return PyNetworkPtr(new PyNetwork(mOrganism->net)); }

        /// in Lamarckian evolution, save the weights back into the genotype
        void UpdateGenotype() { mOrganism->update_genotype(); }

        friend std::ostream& operator<<(std::ostream& output, const PyOrganism& net);
    };
}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
