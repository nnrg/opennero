#include "core/Common.h"
#include "ai/rtneat/ScoreHelper.h"
#include "ai/AI.h"

namespace OpenNero {

    U32 Stats::s_RunningAverageSampleSize = 2;
    
    ScoreHelper::ScoreHelper(const RewardInfo& reward_info)
        : m_SampleSize(0)
        , m_Zero(reward_info.getInstance())
        , m_Total(m_Zero)
        , m_SumOfSquares(m_Zero)
        , m_Average(m_Zero)
        , m_StandardDeviation(m_Zero)
        , m_Min(m_Zero)
        , m_Max(m_Zero)
    {
        for (size_t i = 0; i < m_Min.size(); ++i)
        {
            m_Min[i] = FLT_MAX;
            m_Max[i] = -FLT_MAX;
        }
    }
    
    ScoreHelper::~ScoreHelper()
    {
    }
    
    void ScoreHelper::reset()
    {
        m_SampleSize = 0;
        m_Total = m_Zero;
        m_SumOfSquares = m_Zero;
        m_Average = m_Zero;
        m_StandardDeviation = m_Zero;    
        for (size_t i = 0; i < m_Min.size(); ++i)
        {
            m_Min[i] = FLT_MAX;
            m_Max[i] = -FLT_MAX;
        }
    }
    
    void ScoreHelper::doCalculations()
    {
        calculateAverages();
        calculateStandardDeviations();
    }
    
    void ScoreHelper::calculateAverages()
    {
        if (m_SampleSize > 0) {
            m_Average = m_Total / (double)m_SampleSize;
        } else {
            m_Average = m_Zero;
        }
    }
    
    void ScoreHelper::calculateStandardDeviations()
    {
        if (m_SampleSize > 0) {
            for (size_t i = 0; i < m_StandardDeviation.size(); ++i) {
                m_StandardDeviation[i] = sqrt( m_SumOfSquares[i] / m_SampleSize - m_Average[i] * m_Average[i]);
            }
        } else {
            m_StandardDeviation = m_Zero;
        }
    }
    
    /// add a reward sample
    void ScoreHelper::addSample(Reward sample)
    {
        m_Total += sample;
        for (size_t i = 0; i < sample.size(); ++i)
        {
            m_SumOfSquares[i] += (sample[i] * sample[i]);
            if (m_Min[i] > sample[i])
                m_Min[i] = sample[i];
            if (m_Max[i] < sample[i])
                m_Max[i] = sample[i];
        }
        
        ++m_SampleSize;
    }

    /// preferred generic method
    Reward ScoreHelper::getRelativeScore(Reward absoluteScore) const
    {
        Reward result(absoluteScore);
        for (size_t i = 0; i < absoluteScore.size(); ++i)
        {
            if (m_StandardDeviation[i] > 0) {
                result[i] = (absoluteScore[i] - m_Average[i]) / m_StandardDeviation[i];
            } else { 
                result[i] = m_Zero[i];
            }
        }
        return result;
    }

    /// Number of trials processed over the unit's lifetime
    Stats::Stats(const RewardInfo& info) 
        : m_NumLifetimeTrials(0)
        , m_ZeroStats(info.getInstance())
        , m_Stats(m_ZeroStats)
        , m_LifetimeAverage(m_ZeroStats)
    {
        // empty
    }
            
    /// Reset all stats
    void Stats::resetAll()
    {
        m_NumLifetimeTrials = 0;
        m_Stats = m_ZeroStats;
        m_LifetimeAverage = m_ZeroStats;
    }
    
    /// start next trial
    void Stats::startNextTrial()
    {
        ++m_NumLifetimeTrials;
        if (m_NumLifetimeTrials <= s_RunningAverageSampleSize)
        {
            m_LifetimeAverage += (m_Stats / (F32)m_NumLifetimeTrials);
        } else {
            m_LifetimeAverage += (m_Stats / (F32)s_RunningAverageSampleSize) - 
                (m_LifetimeAverage / (F32)s_RunningAverageSampleSize);
        }
        m_Stats = m_ZeroStats;
    }
    
    /// predict what stats would be w/o death
    void Stats::predictStats(int timeAlive, int fullLife )
    {
        //get the local duration of life that we've lived
        int localTimeAlive = timeAlive % fullLife;

        //we can't predict if time is zero.
        if( localTimeAlive == 0 )
            return;

        //we if have already lived a full life, no need to predict
        if( localTimeAlive >= fullLife )
            return;

        F32 predict = (F32)fullLife / localTimeAlive;

        //predict the stats
        m_Stats = m_Stats * predict;
    }
    
    // Stat-tallying methods
    void Stats::tally(Reward sample)
    {
        m_Stats += sample;
    }
    
    std::ostream& operator<<(std::ostream& os, const Stats& stats) {
        os << "<Stats trials=\"" << stats.m_NumLifetimeTrials 
           << "\" average=\"" << stats.m_LifetimeAverage
           << "\" stats=\"" << stats.m_Stats << "\" />";
        return os;
    }
    
}
