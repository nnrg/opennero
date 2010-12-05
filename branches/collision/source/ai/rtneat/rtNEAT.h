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
#include <iostream>
#include <boost/python.hpp>

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
        void load_sensors(py::list l)
        {
            std::vector<double> sensors;
            for (py::ssize_t i = 0; i < py::len(l); ++i)
            {
                sensors.push_back(py::extract<double>(l[i]));
            }
            mNetwork->load_sensors(sensors);
        }

        /// activate the network for one or more steps until signal reaches output
        bool activate() { return mNetwork->activate(); }

        /// get output values from the network
        py::list get_outputs()
        {
            py::list l;
            std::vector<NNodePtr>::const_iterator iter;
            for (iter = mNetwork->outputs.begin(); iter != mNetwork->outputs.end(); ++iter)
            {
                l.append((*iter)->get_active_out());
            }
            return l;
        }
        /// operator to push to an output stream
        friend std::ostream& operator<<(std::ostream& output, const PyNetwork& net);
    };

    /// A Python wrapper for the Organism class with a simple interface for fitness and network
    class PyOrganism
    {
        OrganismPtr mOrganism;
    public:
        /// constructor for a PyOrganism
        PyOrganism(OrganismPtr org) : mOrganism(org) {}
        /// set the fitness of the organism
        void SetFitness(double fitness) { 
            if (mOrganism->fitness == 0)
            mOrganism->fitness = fitness; }
        /// get the fitness of the organism
        double GetFitness() const { return mOrganism->fitness; }
		/// get the genome ID of this organism
        int GetId() const { return mOrganism->gnome->genome_id; }
        /// set the amount of time the organism has to live
    	void SetTimeAlive(int time_alive) { mOrganism->time_alive = time_alive; }
        /// get the amount of time that the organism has to live
        int GetTimeAlive() const { return mOrganism->time_alive; }
        /// get network of the organism
        PyNetworkPtr GetNetwork() const { return PyNetworkPtr(new PyNetwork(mOrganism->net)); }
        /// save this organism to a file
        bool Save(const std::string& fname) const { return mOrganism->print_to_file(fname); }
        /// operator to push to an output stream
        friend std::ostream& operator<<(std::ostream& output, const PyOrganism& net);
    };

}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
