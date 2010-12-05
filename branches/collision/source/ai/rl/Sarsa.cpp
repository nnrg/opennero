#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "Sarsa.h"
#include <cfloat>
#include <list>

namespace OpenNero
{

    /// called for agent to take its first step
    Actions SarsaBrain::start(const TimeType& time, const Observations& new_state)
    {
        cumulative_reward = 0;
        return TDBrain::start(time, new_state);
    }

    /// act based on time, sensor arrays, and last reward
    Actions SarsaBrain::act(const TimeType& time, const Observations& new_state, const Reward& reward)
    {
        cumulative_reward += reward;
        return TDBrain::act(time, new_state, reward);
    }

    /// called to tell agent about its last reward
    bool SarsaBrain::end(const TimeType& time, const Reward& reward)
    {
        ++n_episodes;
        LOG_F_DEBUG("ai", "TDBrain cumulative reward: "<< cumulative_reward << " after " << n_episodes);
        return TDBrain::end(time, reward);
    }

    double SarsaBrain::predict(const Observations& new_state) {
    	return mApproximator->predict(new_state, new_action);
    }
}
