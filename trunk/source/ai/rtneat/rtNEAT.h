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
        queue<PyOrganismPtr> mWaitingBrainList; ///< queue of organisms to be evaluated
        vector<PyOrganismPtr> mBrainList; ///< all the organisms along with their stats
        BrainBodyMap mBrainBodyMap;       ///< map from agents to organisms
        size_t mOffspringCount;           ///< number of reproductions so far
		size_t mSpawnTickCount;           ///< number of spawn ticks
		size_t mEvolutionTickCount;       ///< number of evolution ticks
        size_t mTotalUnitsDeleted;        ///< total units deleted
        size_t mUnitsToDeleteBeforeFirstJudgment; ///< number of units to delete before judging
        size_t mTimeBetweenEvolutions;    ///< time (in ticks) between rounds of evolution
        RewardInfo mRewardInfo; ///< the constraints that describe the per-step rewards
        FeatureVector mFitnessWeights; ///< fitness weights
        bool mEvolutionEnabled; ///< whether the evolution is enabled
        
    public:
        /// Constructor
        /// @param filename name of the file with the initial population genomes
        /// @param param_file file with RTNEAT parameters to load
        /// @param population_size size of the population to construct
        /// @param reward_info the specifications for the multidimensional reward
        RTNEAT(const std::string& filename, 
               const std::string& param_file, 
               size_t population_size,
               const RewardInfo& reward_info);

        /// Constructor
        /// @param param_file RTNEAT parameter file
        /// @param inputs number of inputs
        /// @param outputs number of outputs
        /// @param population_size size of the population to construct
        /// @param noise variance of the Gaussian used to assign initial weights
        /// @param reward_info the specifications for the multidimensional reward
        RTNEAT(const std::string& param_file, 
               size_t inputs, 
               size_t outputs,
               size_t population_size, 
               F32 noise,
               const RewardInfo& reward_info);

        /// Destructor
        ~RTNEAT();
        
        /// are we ready to spawn a new organism?
        bool ready();
                
        /// have we been deleted?
        bool has_organism(AgentBrainPtr agent);
        
        /// get the organism currently assigned to the agent
        PyOrganismPtr get_organism(AgentBrainPtr agent);
        
        /// release the organism that was being used by the agent
        void release_organism(AgentBrainPtr agent);
        
        /// Called every step by the OpenNERO system
        virtual void ProcessTick( float32_t incAmt );

        /// save the current population to a file
        bool save_population(const std::string& population_file);

        /// load a population from a file
        bool load_population(const std::string& population_file);
        
        /// get the weight vector
        const FeatureVector& get_weights() const { return mFitnessWeights; }
        
        /// set the i'th weight
        void set_weight(size_t i, double weight) { mFitnessWeights[i] = weight; }
        
        /// set the lifetime so that we can ensure that the units have been alive
        /// at least that long before evaluating them
        void set_lifetime(size_t lifetime);
        
        /// enable or disable evolution
        /// @{
        void enable_evolution() { mEvolutionEnabled = true; }
        void disable_evolution() { mEvolutionEnabled = false; }
        /// @}
        
        /// check if the evolution is enabled
        bool is_evolution_enabled() const { return mEvolutionEnabled; }
        
        /// load info about this AI from the object template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data) { return true; }

	private:

		/// evaluate all brains by compiling their stats
		void evaluateAll();

		/// evolution step that potentially replaces an organism with an 
		/// offspring
		void evolveAll();

		/// Delete the unit which is currently associated with the specified
		/// brain and move the brain back to waiting list.
		void deleteUnit(PyOrganismPtr brain);
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
		F32 mAbsoluteScore;
        Stats mStats;

		/// constructor for a PyOrganism
        /// @param org rtNEAT organism to wrap
        /// @param reward_info the info about the multidimensional reward
        PyOrganism(OrganismPtr org, const RewardInfo& reward_info) : 
            mOrganism(org), 
            mAbsoluteScore(0),
            mStats(reward_info)
        { }
        /// set the fitness of the organism
        void SetFitness(double fitness) { 
            if (mOrganism->fitness == 0)
            mOrganism->fitness = fitness;
        }
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
        /// the id of the species of the organism
        int GetSpeciesId() const;
        /// whether the organism is a champion
        bool IsChampion() const;
        /// save this organism to a file
        bool Save(const std::string& fname) const { return mOrganism->print_to_file(fname); }
        /// Get the organism
        OrganismPtr GetOrganism() { return mOrganism; }
        /// Set the organism
        void SetOrganism(OrganismPtr organism) { mOrganism = organism; mAbsoluteScore = 0; }
        /// operator to push to an output stream
        friend std::ostream& operator<<(std::ostream& output, const PyOrganism& net);
    };
}

#endif /* _OPENNERO_AI_RTNEAT_RTNEAT_H_ */
