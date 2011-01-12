#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "TD.h"
#include <cfloat>
#include <list>

namespace OpenNero
{

    /// called right before the agent is born
    bool TDBrain::initialize(const AgentInitInfo& init)
    {
        mInfo = init;
        this->fitness = mInfo.reward.getInstance();
        mApproximator.reset(new TableApproximator(mInfo)); // initialize the function approximator
        return true;
    }

    /// called for agent to take its first step
    Actions TDBrain::start(const TimeType& time, const Observations& new_state)
    {
        epsilon_greedy(new_state);
        action = new_action;
        state = new_state;
        return action;
    }

    /// act based on time, sensor arrays, and last reward
    Actions TDBrain::act(const TimeType& time, const Observations& new_state, const Reward& reward)
    {
        double new_Q = epsilon_greedy(new_state); // select new action and estimate its value
        double old_Q = mApproximator->predict(state, action);
        // Q(s_t, a_t) <- Q(s_t, a_t) + \alpha [r_{t+1} + \gamma Q(s_{t+1}, a_{t+1}) - Q(s_t, a_t)
        mApproximator->update(state, action, old_Q + mAlpha * (reward[0] + mGamma * new_Q - old_Q));
        action = new_action;
        state = new_state;
        return action;
    }

    /// called to tell agent about its last reward
    bool TDBrain::end(const TimeType& time, const Reward& reward)
    {
        // Q(s_t, a_t) <- Q(s_t, a_t) + \alpha [r_{t+1} - Q(s_t, a_t)]
        // LOG_F_DEBUG("ai", "TD FINAL UPDATE s1: " << state << ", a1: " << action << ", r: " << reward);
        double old_Q = mApproximator->predict(state, action);
        mApproximator->update(state, action, old_Q + mAlpha * (reward[0] - old_Q));
        return true;
    }

    /// select action according to the epsilon-greedy policy
    double TDBrain::epsilon_greedy(const Observations& new_state)
    {
        // with chance epsilon, select random action
        if (RANDOM.randF() < mEpsilon)
        {
        	new_action = mInfo.actions.getRandom();
            double value = predict(new_state);
            return value;
        }
        // enumerate all possible actions (actions must be discrete!)
        new_action = mInfo.actions.getInstance();
        std::list< FeatureVector > action_list = mInfo.actions.enumerate();
        // select the greedy action
        double max_value = -DBL_MAX;
        std::list< Actions >::const_iterator iter;
        for (iter = action_list.begin(); iter != action_list.end(); ++iter)
        {
            double value = mApproximator->predict(new_state, *iter);
            if (value > max_value)
            {
                max_value = value;
                new_action = *iter;
            }
        }
        // Assuming if you choose max value, you will want to update with that as your prediction
        return max_value;
    }

    /// called right before the agent dies
    bool TDBrain::destroy()
    {
        return true;
    }

}
