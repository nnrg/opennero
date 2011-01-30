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
        , mWaitingBrainList()
		, mBrainList()
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
			PyOrganismPtr brain(new PyOrganism(mPopulation->organisms[i]));
            mWaitingBrainList.push(brain);
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
        , mWaitingBrainList()
		, mBrainList()
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
			PyOrganismPtr brain(new PyOrganism(mPopulation->organisms[i]));
            mWaitingBrainList.push(brain);
        }
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
            PyOrganismPtr brain(new PyOrganism(mWaitingBrainList.front()));
            mAgentsToOrganisms[agent] = brain;
            return brain;
        }

    }
    
    /// release the organism that was being used by the agent
    void RTNEAT::release_organism(AgentBrainPtr agent)
    {
        AgentToOrganismMap::const_iterator found;
        found = mAgentsToOrganisms.find(agent);
        Assert(found != mAgentsToOrganisms.end());
        mWaitingBrainList.push(found->second);
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
    
    void RTNEAT::ProcessTick( float32_t incAmt )
    {
        
    }
    
    void RTNEAT::evaluateAll()
    {
        // Zero out the score helper
        mScoreHelper->reset();

        for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != mBrainList.end(); ++iter) {
            if ((*iter)->GetOrganism()->time_alive >= NEAT::time_alive_minimum) {
                if ( (!((*iter)->GetOrganism()->time_alive % NEAT::time_alive_minimum)) && (*iter)->GetOrganism()->time_alive > 0 ) {

                    //temp hack. evaluateBrains() is called
                    //more often than NeroBrain::think for some reason
                    (*iter)->GetOrganism()->time_alive++; 												   
                    (*iter)->m_Stats.startNextTrial();
                }
                mScoreHelper->addAccuracyOfShotsSample((*iter)->m_Stats.getAccuracyOfShots());
                mScoreHelper->addEnemyHitsSample((*iter)->m_Stats.getEnemyHits());
                mScoreHelper->addFriendHitsSample((*iter)->m_Stats.getFriendHits());
                mScoreHelper->addHitsTakenSample((*iter)->m_Stats.getHitsTaken());
                mScoreHelper->addWeaponFiresSample((*iter)->m_Stats.getWeaponFires());
                mScoreHelper->addTravelDistanceSample((*iter)->m_Stats.getTravelDistance());
                mScoreHelper->addDistanceFromEnemiesSample((*iter)->m_Stats.getDistanceFromEnemies());
                mScoreHelper->addDistanceFromFriendsSample((*iter)->m_Stats.getDistanceFromFriends());
                mScoreHelper->addDistanceFromFlagSample((*iter)->m_Stats.getDistanceFromFlag());
                mScoreHelper->addRangeFromEnemySample((*iter)->m_Stats.getRangeFromEnemy());
                mScoreHelper->addRangeFromFriendsSample((*iter)->m_Stats.getRangeFromFriends());
            }
        }

        mScoreHelper->doCalculations();

        //Con::printf("Population accuracy average: %f", m_ScoreHelper->getAccuracyOfShotsAverage());
        F32 minAbsoluteScore = 0; // min of 0, min abs score
        F32 maxAbsoluteScore = -F32_MAX; // max raw score
        PyOrganismPtr champ; // brain with best raw score
        F32 accuracyOfShotsScore;
        F32 enemyHitsScore;
        F32 friendHitsScore;
        F32 hitsTakenScore;
        F32 weaponFiresScore;
        F32 travelDistanceScore;
        F32 distanceFromEnemiesScore;
        F32 distanceFromFriendsScore;
        F32 distanceFromFlagScore;
        F32 rangeFromEnemyScore;
        F32 rangeFromFriendsScore;

        for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != m_rainList.end(); ++iter) {
            if ((*iter)->GetOrganism()->time_alive >= NEAT::time_alive_minimum) {
                accuracyOfShotsScore = mScoreHelper->getAccuracyOfShotsRelativeScore((*iter)->m_Stats.getAccuracyOfShots()) * s_AccuracyOfShotsWeight;
                enemyHitsScore = mScoreHelper->getEnemyHitsRelativeScore((*iter)->m_Stats.getEnemyHits()) * s_EnemyHitsWeight;
                friendHitsScore = mScoreHelper->getFriendHitsRelativeScore((*iter)->m_Stats.getFriendHits()) * s_FriendHitsWeight;
                hitsTakenScore = mScoreHelper->getHitsTakenRelativeScore((*iter)->m_Stats.getHitsTaken()) * s_HitsTakenWeight;
                weaponFiresScore = mScoreHelper->getWeaponFiresRelativeScore((*iter)->m_Stats.getWeaponFires()) * s_WeaponFiresWeight;
                travelDistanceScore = mScoreHelper->getTravelDistanceRelativeScore((*iter)->m_Stats.getTravelDistance()) * s_TravelDistanceWeight;
                distanceFromEnemiesScore = mScoreHelper->getDistanceFromEnemiesRelativeScore((*iter)->m_Stats.getDistanceFromEnemies()) * s_DistanceFromEnemiesWeight;
                distanceFromFriendsScore = mScoreHelper->getDistanceFromFriendsRelativeScore((*iter)->m_Stats.getDistanceFromFriends()) * s_DistanceFromFriendsWeight;
                distanceFromFlagScore = mScoreHelper->getDistanceFromFlagRelativeScore((*iter)->m_Stats.getDistanceFromFlag()) * s_DistanceFromFlagWeight;
                rangeFromEnemyScore = mScoreHelper->getRangeFromEnemyRelativeScore((*iter)->m_Stats.getRangeFromEnemy()) * s_RangeFromEnemyWeight;
                rangeFromFriendsScore = mScoreHelper->getRangeFromFriendsRelativeScore((*iter)->m_Stats.getRangeFromFriends()) * s_RangeFromFriendsWeight;

                (*iter)->mAbsoluteScore = accuracyOfShotsScore +
                    enemyHitsScore +
                    friendHitsScore +
                    hitsTakenScore +
                    weaponFiresScore +
                    travelDistanceScore +
                    distanceFromEnemiesScore +
                    distanceFromFriendsScore +
                    distanceFromFlagScore +
                    rangeFromEnemyScore +
                    rangeFromFriendsScore;

                if ((*iter)->mAbsoluteScore < minAbsoluteScore)
                    minAbsoluteScore = (*iter)->mAbsoluteScore;
				if ((*iter)->mAbsoluteScore > maxAbsoluteScore)
					maxAbsoluteScore = (*iter)->mAbsoluteScore;
            }
        }

        if (minAbsoluteScore < 0) {
            for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != mBrainList.end(); ++iter) {
                if ((*iter)->GetOrganism()->time_alive >= NEAT::time_alive_minimum) {
                    F32 modifiedFitness = (*iter)->mAbsoluteScore - minAbsoluteScore;
                    if (modifiedFitness < 0)
                        modifiedFitness = 0;


                    if (!((*iter)->GetOrganism()->smited)) 
					{
                        (*iter)->GetOrganism()->fitness = modifiedFitness;
					}
                    else 
                    { 
                        (*iter)->GetOrganism()->fitness = 0.01 * modifiedFitness;
                    }
                }
            }
        }
        else
        {
            for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != mBrainList.end(); ++iter) {
                if ((*iter)->GetOrganism()->time_alive >= NEAT::time_alive_minimum) {

                    if (!((*iter)->GetOrganism()->smited)) 
                        (*iter)->GetOrganism()->fitness = (*iter)->mAbsoluteScore;
                    else 
                    { 
                        (*iter)->GetOrganism()->fitness = 0.01 * (*iter)->mAbsoluteScore;
                    }
                }
            }
        }
    }
    
    void RTNEAT::evolveAll()
    {
        // Remove the worst organism
        OrganismPtr deadorg = mPopulation->remove_worst();

        //We can try to keep the number of species constant at this number
        S32 num_species_target=4;
        S32 compat_adjust_frequency = mBrainList.size()/10;
        if (compat_adjust_frequency < 1)
            compat_adjust_frequency = 1;

        vector<PyOrganismPtr>::iterator curorg;
        SpeciesPtr new_species;

        // Sometimes, if all organisms are beneath the minimum "time alive" threshold, no organism will be removed
        // If an organism *was* actually removed, then we can proceed with replacing it via the evolutionary process
        if (deadorg) {
            NEAT::Organism *new_org = 0;

            // Estimate all species' fitnesses
            for (vector<SpeciesPtr>::iterator curspec = (mPopulation->species).begin(); curspec != (mPopulation->species).end(); ++curspec) {
                (*curspec)->estimate_average();

                // Calculate an average based upon the actual scores (not the adjusted, non-negative scores that are
                // being passed to organisms' fitness fields) so that we can display an average that makes sense from
                // evaluation to evaluation
                F32 scoreavg = 0;
                S32 samplesize = 0;
                vector<OrganismPtr>::iterator curorg = mPopulation->organisms.begin();
                for ( ; curorg != mPopulation->organisms.end(); ++curorg) {
                    if ((*curorg)->species == (*curspec)) {
                        vector<PyOrganismPtr>::iterator curbrain = mBrainList.begin();
                        for ( ; curbrain != mBrainList.end(); ++curbrain) {
                            if ( (*curbrain)->GetOrganism() == (*curorg) && (*curbrain)->GetOrganism()->time_alive >= NEAT::time_alive_minimum) {
                                scoreavg += (*curbrain)->mAbsoluteScore;
                                ++samplesize;                            
                            }
                        }
                    }
                }
                if (samplesize > 0)
                    scoreavg /= (F32)samplesize;

                LOG_F_DEBUG("ai", "Species " << (*curspec)->id << 
					              " size: " << (*curspec)->organisms.size() << 
								  " elig. size: " << samplesize <<
								  " avg. score: " << scoreavg);
            }

            // Print out info about the organism that was killed off
            for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != mBrainList.end(); ++iter) {
                if ((*iter)->GetOrganism() == deadorg) {
                    LOG_F_DEBUG("ai", "Org to kill: score = " << (*iter)->m_AbsoluteScore);
                    break;
                }
            }

            //m_Population->memory_pool->isEmpty();
            if(Platform::getRandom()<=s_MilestoneProbability && !m_Population->memory_pool->isEmpty())// && meets probability requirement)
                {
                    // Reproduce an organism with the same traits as the "memory pool".
                    new_org = (m_Population->memory_pool)->reproduce_one(m_OffspringCount, m_Population, m_Population->species);
                }
            else
                // Reproduce a single new organism to replace the one killed off.
                new_org = (m_Population->choose_parent_species())->reproduce_one(m_OffspringCount,m_Population,m_Population->species, 0,0);
            ++mOffspringCount;

            //Every m_BrainList.size() reproductions, reassign the population to new species
            if (mOffspringCount % compat_adjust_frequency == 0) {

                S32 num_species = m_Population->species.size();
                F64 compat_mod=0.1;  //Modify compat thresh to control speciation

                // This tinkers with the compatibility threshold, which normally would be held constant
                if (num_species < num_species_target)
                    NEAT::compat_threshold -= compat_mod;
                else if (num_species > num_species_target)
                    NEAT::compat_threshold += compat_mod;

                if (NEAT::compat_threshold < 0.3) 
                    NEAT::compat_threshold = 0.3;

                //Go through entire population, reassigning organisms to new species
                for (curorg = mPopulation->organisms.begin(); curorg != mPopulation->organisms.end(); ++curorg) {
                    mPopulation->reassign_species(*curorg);
                }
            }

            // Iterate through all of the Brains, find the one whose Organism was killed off, and link that Brain
            // to the newly created Organism, effectively doing a "hot swap" of the Organisms in that Brain.  
            for (vector<PyOrganismPtr>::iterator iter = mBrainList.begin(); iter != mBrainList.end(); ++iter) {
                if ((*iter)->GetOrganism() == deadorg) {
                    (*iter)->SetOrganism(new_org);
                    (*iter)->m_Stats.resetAll();
                    for (vector<NEROBody*>::iterator iter2 = mBodyList.begin(); iter2 != mBodyList.end(); ++iter2) {
                        if ((*iter2)->getBrain() == (*iter)) {
                            deleteUnit(*iter2);
                            break;
                        }
                    }
                    break;
                }
            }
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
