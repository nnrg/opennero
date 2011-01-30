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
    
    void RTNEAT::ProcessTick( float32_t incAmt )
    {
        
    }
    
    void RTNEAT::evaluateAll()
    {
        // Zero out the score helper
        m_ScoreHelper->reset();

        for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
            if ((*iter)->m_Organism->time_alive >= NEAT::time_alive_minimum) {
                if ( (!((*iter)->m_Organism->time_alive % NEAT::time_alive_minimum)) && (*iter)->m_Organism->time_alive > 0 ) {

                    //temp hack. evaluateBrains() is called
                    //more often than NeroBrain::think for some reason
                    (*iter)->m_Organism->time_alive++; 												   
                    (*iter)->m_Stats.startNextTrial();
                }
                m_ScoreHelper->addAccuracyOfShotsSample((*iter)->m_Stats.getAccuracyOfShots());
                m_ScoreHelper->addEnemyHitsSample((*iter)->m_Stats.getEnemyHits());
                m_ScoreHelper->addFriendHitsSample((*iter)->m_Stats.getFriendHits());
                m_ScoreHelper->addHitsTakenSample((*iter)->m_Stats.getHitsTaken());
                m_ScoreHelper->addWeaponFiresSample((*iter)->m_Stats.getWeaponFires());
                m_ScoreHelper->addTravelDistanceSample((*iter)->m_Stats.getTravelDistance());
                m_ScoreHelper->addDistanceFromEnemiesSample((*iter)->m_Stats.getDistanceFromEnemies());
                m_ScoreHelper->addDistanceFromFriendsSample((*iter)->m_Stats.getDistanceFromFriends());
                m_ScoreHelper->addDistanceFromFlagSample((*iter)->m_Stats.getDistanceFromFlag());
                m_ScoreHelper->addRangeFromEnemySample((*iter)->m_Stats.getRangeFromEnemy());
                m_ScoreHelper->addRangeFromFriendsSample((*iter)->m_Stats.getRangeFromFriends());
            }
        }

        m_ScoreHelper->doCalculations();

        //Con::printf("Population accuracy average: %f", m_ScoreHelper->getAccuracyOfShotsAverage());
        F32 minAbsoluteScore = 0; // min of 0, min abs score
        F32 maxAbsoluteScore = -F32_MAX; // max raw score
        NEROBrain* champ = NULL; // brain with best raw score
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

        for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
            if ((*iter)->m_Organism->time_alive >= NEAT::time_alive_minimum) {
                accuracyOfShotsScore = m_ScoreHelper->getAccuracyOfShotsRelativeScore((*iter)->m_Stats.getAccuracyOfShots()) * s_AccuracyOfShotsWeight;
                enemyHitsScore = m_ScoreHelper->getEnemyHitsRelativeScore((*iter)->m_Stats.getEnemyHits()) * s_EnemyHitsWeight;
                friendHitsScore = m_ScoreHelper->getFriendHitsRelativeScore((*iter)->m_Stats.getFriendHits()) * s_FriendHitsWeight;
                hitsTakenScore = m_ScoreHelper->getHitsTakenRelativeScore((*iter)->m_Stats.getHitsTaken()) * s_HitsTakenWeight;
                weaponFiresScore = m_ScoreHelper->getWeaponFiresRelativeScore((*iter)->m_Stats.getWeaponFires()) * s_WeaponFiresWeight;
                travelDistanceScore = m_ScoreHelper->getTravelDistanceRelativeScore((*iter)->m_Stats.getTravelDistance()) * s_TravelDistanceWeight;
                distanceFromEnemiesScore = m_ScoreHelper->getDistanceFromEnemiesRelativeScore((*iter)->m_Stats.getDistanceFromEnemies()) * s_DistanceFromEnemiesWeight;
                distanceFromFriendsScore = m_ScoreHelper->getDistanceFromFriendsRelativeScore((*iter)->m_Stats.getDistanceFromFriends()) * s_DistanceFromFriendsWeight;
                distanceFromFlagScore = m_ScoreHelper->getDistanceFromFlagRelativeScore((*iter)->m_Stats.getDistanceFromFlag()) * s_DistanceFromFlagWeight;
                rangeFromEnemyScore = m_ScoreHelper->getRangeFromEnemyRelativeScore((*iter)->m_Stats.getRangeFromEnemy()) * s_RangeFromEnemyWeight;
                rangeFromFriendsScore = m_ScoreHelper->getRangeFromFriendsRelativeScore((*iter)->m_Stats.getRangeFromFriends()) * s_RangeFromFriendsWeight;

                (*iter)->m_AbsoluteScore =	accuracyOfShotsScore +
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

                if ((*iter)->m_AbsoluteScore < minAbsoluteScore)
                    minAbsoluteScore = (*iter)->m_AbsoluteScore;

                F32 rawScore = GuiNEROFitnessGraph::evaluate(*iter);
                if (rawScore > maxAbsoluteScore) {
                    maxAbsoluteScore = rawScore;
                    champ = (*iter);
                }
            }
        }

        if (champ != NULL) {
            F32 score = GuiNEROFitnessGraph::evaluate(champ,true);
            GuiNERONetworkDisplay::setBrain(champ);
            NEROLog::write(NEROLog::LOG_FITNESS, score);
        }

        if (minAbsoluteScore < 0) {
            for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
                if ((*iter)->m_Organism->time_alive >= NEAT::time_alive_minimum) {
                    F32 modifiedFitness = (*iter)->m_AbsoluteScore - minAbsoluteScore;
                    if (modifiedFitness < 0)
                        modifiedFitness = 0;


                    if (!((*iter)->m_Organism->smited)) 
                        (*iter)->m_Organism->fitness = modifiedFitness;
                    else 
                    { 
                        (*iter)->m_Organism->fitness = 0.01 * modifiedFitness;
                    }
                }
            }
        }
        else
        {
            for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
                if ((*iter)->m_Organism->time_alive >= NEAT::time_alive_minimum) {

                    if (!((*iter)->m_Organism->smited)) 
                        (*iter)->m_Organism->fitness = (*iter)->m_AbsoluteScore;
                    else 
                    { 
                        (*iter)->m_Organism->fitness = 0.01 * (*iter)->m_AbsoluteScore;
                    }
                }
            }
        }
    }
    
    void RTNEAT::evolveAll()
    {
        // Remove the worst organism
        //NEAT::Organism *deadorg = m_Population->remove_worst_probabilistic();
        NEAT::Organism *deadorg = m_Population->remove_worst();

        //We can try to keep the number of species constant at this number
        S32 num_species_target=4;
        S32 compat_adjust_frequency = m_BrainList.size()/10;
        if (compat_adjust_frequency < 1)
            compat_adjust_frequency = 1;

        Vector<NEAT::Organism*>::iterator curorg;
        NEAT::Species *new_species;

        // Sometimes, if all organisms are beneath the minimum "time alive" threshold, no organism will be removed
        // If an organism *was* actually removed, then we can proceed with replacing it via the evolutionary process
        if (deadorg) {
            NEAT::Organism *new_org = 0;

            // Estimate all species' fitnesses
            for (Vector<NEAT::Species*>::iterator curspec = (m_Population->species).begin(); curspec != (m_Population->species).end(); ++curspec) {
                (*curspec)->estimate_average();

                // Calculate an average based upon the actual scores (not the adjusted, non-negative scores that are
                // being passed to organisms' fitness fields) so that we can display an average that makes sense from
                // evaluation to evaluation
                F32 scoreavg = 0;
                S32 samplesize = 0;
                Vector<NEAT::Organism*>::iterator curorg = m_Population->organisms.begin();
                for ( ; curorg != m_Population->organisms.end(); ++curorg) {
                    if ((*curorg)->species == (*curspec)) {
                        Vector<NEROBrain*>::iterator curbrain = m_BrainList.begin();
                        for ( ; curbrain != m_BrainList.end(); ++curbrain) {
                            if ( (*curbrain)->m_Organism == (*curorg) && (*curbrain)->m_Organism->time_alive >= NEAT::time_alive_minimum) {
                                scoreavg += (*curbrain)->m_AbsoluteScore;
                                ++samplesize;                            
                            }
                        }
                    }
                }
                if (samplesize > 0)
                    scoreavg /= (F32)samplesize;

                if( s_ConsoleDebug )
                    Con::printf("Species %d   size: %d   elig. size: %d   avg. score: %f", (*curspec)->id, (*curspec)->organisms.size(), samplesize, scoreavg);  //PFHACK
            }
            //Con::printf("--------------------");  //PFHACK

            // Print out info about the organism that was killed off
            for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
                if ((*iter)->m_Organism == deadorg) {
                    if( s_ConsoleDebug )
                        Con::printf("Org to kill: score = %f", (*iter)->m_AbsoluteScore);
                    break;
                }
            }

            //m_Population->memory_pool->isEmpty();
            if(Platform::getRandom()<=s_MilestoneProbability && !m_Population->memory_pool->isEmpty())// && meets probability requirement)
                {
                    // Reproduce an organism with the same traits as the "memory pool".
                    Con::printf("Using the Memory Pool");
                    new_org = (m_Population->memory_pool)->reproduce_one(m_OffspringCount, m_Population, m_Population->species);
                }
            else
                // Reproduce a single new organism to replace the one killed off.
                new_org = (m_Population->choose_parent_species())->reproduce_one(m_OffspringCount,m_Population,m_Population->species, 0,0);
            ++m_OffspringCount;

            //Con::printf("Compat threshold: %f",NEAT::compat_threshold);

            //Every m_BrainList.size() reproductions, reassign the population to new species
            if (m_OffspringCount % compat_adjust_frequency == 0) {

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
                for (curorg = (m_Population->organisms).begin(); curorg != (m_Population->organisms).end(); ++curorg) {
                    m_Population->reassign_species(*curorg);
                }
            }

            // Iterate through all of the Brains, find the one whose Organism was killed off, and link that Brain
            // to the newly created Organism, effectively doing a "hot swap" of the Organisms in that Brain.  
            for (Vector<NEROBrain*>::iterator iter = m_BrainList.begin(); iter != m_BrainList.end(); ++iter) {
                if ((*iter)->m_Organism == deadorg) {
                    (*iter)->m_Organism = new_org;
                    (*iter)->m_Stats.resetAll();
                    for (Vector<NEROBody*>::iterator iter2 = m_BodyList.begin(); iter2 != m_BodyList.end(); ++iter2) {
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
