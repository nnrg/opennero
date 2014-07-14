#ifndef _OPENNERO_AI_RTNEAT_SCOREHELPER_H_
#define _OPENNERO_AI_RTNEAT_SCOREHELPER_H_

#include "core/Preprocessor.h"
#include "ai/AI.h"

namespace OpenNero
{
    using namespace std;


    /// Holdings scoring information
    class ScoreHelper {
    private:

        size_t m_SampleSize;
        Reward m_Zero;
        Reward m_Total;
        Reward m_SumOfSquares;
        Reward m_Average;
        Reward m_StandardDeviation;
        Reward m_Min;
        Reward m_Max;

    public:
    
        ScoreHelper(const RewardInfo& zero);
        ~ScoreHelper();

        void reset();
        void doCalculations();
        void calculateAverages();
        void calculateStandardDeviations();

        /// add a reward sample
        void addSample(Reward sample);

        /// average scores in all dimensions
        const Reward& getAverage() const { return m_Average; }
        
        /// standard deviation of scores in all dimensions
        const Reward& getStandardDeviation() const { return m_StandardDeviation; }
        
        /// the number of samples
        size_t getSampleSize() const { return m_SampleSize; }

        /// get the relative (scaled) Z-scores along the dimensions
        Reward getRelativeScore(Reward absoluteScore) const;
        
        const Reward& getMin() const { return m_Min; }
        
        const Reward& getMax() const { return m_Max; }
    };
    
    class Stats
    {
    public:
        static U32 s_RunningAverageSampleSize;

    private:
        
        /// Number of trials processed over the unit's lifetime
        U32 m_NumLifetimeTrials;
        
        /// Zero stats (for resetting)
        Reward m_ZeroStats;
        
        /// Fields for holding stat accumulations
        Reward m_Stats;
        
        /// Lifetime averages of stat accumulations
        Reward m_LifetimeAverage;
        
        /// output a stats instance to stream
        friend std::ostream& operator<<(std::ostream& os, const Stats& stats);
        
    public:
        /// Constructor
        explicit Stats(const RewardInfo& info);
        
        /// Destructor
        ~Stats() {}
        
        /// Reset all stats
        void resetAll();
        
        /// start next trial
        void startNextTrial();
        
        /// predict what stats would be w/o death
        void predictStats(int timeAlive, int fullLife );
        
        // Stat-tallying methods
        void tally(Reward sample);
        
        U32 GetNumTrials() const { return m_NumLifetimeTrials; }

        /// Stat-retrieval methods
        const Reward& getStats() const { return m_LifetimeAverage; }
    };
    
    
}

#endif // _OPENNERO_AI_RTNEAT_SCOREHELPER_H_
