#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "ai/AI.h"
#include "TD.h"
#include <cfloat>
#include <vector>
#include <algorithm>

namespace OpenNero
{

    /// called right before the agent is born
    bool TDBrain::initialize(const AgentInitInfo& init)
    {
        mInfo = init;

        action_bins = 3;
        state_bins = 5;

        // Similar to FeatureVectorInfo::enumerate (from AI.cpp).
        //
        // We want to enumerate all possible actions in a discrete way, so that
        // we can store them in a policy table somehow. So, for each action
        // dimension, if it's discrete we just enumerate the integral values for
        // that dimension, and if it's continuous, we split it into action_bins
        // different values.
        //
        // An example: Suppose some action dimension is continuous and spans the
        // closed interval [-1, 1]. Additionally suppose we want to split
        // continuous action dimensions into 5 bins. We'd like to preserve the
        // actual range of action values, but only store the 5 discrete values
        // that equally partition this space, i.e., {-1, -.5, 0, .5, 1}. So we
        // traverse the action dimension from -1 to 1 (inclusive), adding
        // (hi - lo) / (bins - 1) == (1 - -1) / 4 == 0.5 each time.
        const FeatureVectorInfo& info = init.actions;
        action_list.clear();
        action_list.push_back(info.getInstance());
        for (size_t i = 0; i < info.size(); ++i)
        {
            const double lo = info.getMin(i), hi = info.getMax(i);
            const double inc = info.isDiscrete(i) ? 1.0f : (hi - lo) / (action_bins - 1);
            std::vector< Actions > new_action_list;
            std::vector< Actions >::const_iterator iter;
            for (iter = action_list.begin(); iter != action_list.end(); ++iter)
            {
                for (double a = lo; a <= hi; a += inc)
                {
                    FeatureVector v = *iter;
                    v[i] = a;
                    new_action_list.push_back(v);
                }
            }
            action_list = new_action_list;
        }

        this->fitness = mInfo.reward.getInstance();
        mApproximator.reset(new TableApproximator(mInfo)); // initialize the function approximator
        //mApproximator.reset(new TilesApproximator(mInfo)); // initialize the function approximator
        return true;
    }

    /// called for agent to take its first step
    Actions TDBrain::start(const TimeType& time, const Observations& new_state)
    {
        epsilon_greedy(quantize_state(new_state));
        action = new_action;
        state = new_state;
        return action;
    }

    /// act based on time, sensor arrays, and last reward
    Actions TDBrain::act(const TimeType& time, const Observations& new_state, const Reward& reward)
    {
		AssertMsg(reward.size() == 1, "multi-objective rewards not supported");
        // select new action and estimate its value
        double new_Q = epsilon_greedy(quantize_state(new_state));
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
		AssertMsg(reward.size() == 1, "multi-objective rewards not supported");
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
            new_action = quantize_action(mInfo.actions.getRandom());
            double value = predict(new_state);
            return value;
        }
        // enumerate all possible actions (actions must be discrete!)
        new_action = mInfo.actions.getInstance();
        // select the greedy action in random order
        std::random_shuffle(action_list.begin(), action_list.end());
        double max_value = -DBL_MAX;
        std::vector< Actions >::const_iterator iter;
        for (iter = action_list.begin(); iter != action_list.end(); ++iter)
        {
            double value = mApproximator->predict(new_state, *iter);
            if (value > max_value)
            {
                max_value = value;
                new_action = quantize_action(*iter);
            }
        }
        // Assuming if you choose max value, you will want to update with that as your prediction
        return max_value;
    }

    /// static helper: given a feature vector from a continuous space, quantize
    /// it based on the given info for the vector, and the number of discrete
    /// (linear) bins we want for each dimension of our discretized space.
    ///
    /// This is similar in spirit to the action space enumeration in initialize
    /// above. I'll continue the example from there, i.e., that we want to map a
    /// dimension that spans [-1, 1] into 5 equal bins, with centers at the
    /// values {-1 (bin #0), -.5, 0, .5, 1 (bin #4)}.
    ///
    /// Suppose we get a continuous value of 0.3 for this dimension. This should
    /// probably fit into bin #3 (by a "closest center" metric -- similarly, 0.2
    /// should fit into bin #2). We first map [-1, 1] onto {0, 1, 2, 3, 4}, and
    /// then finally we map the integral bin number back into [-1, 1] by adding
    /// the lower bound (-1) to the appropriate multiple of "inc" == span /
    /// (bins - 1). The tricky bit here is that to map each bin's interval
    /// (e.g., for bin #3, the interval is [.25, .75)) we basically end up
    /// subtracting inc / 2 from everything, which effectively converts the bin
    /// centers into the barriers that divide them.
    ///
    /// So, an input value of 0.3 would be normalized to (0.3 - (-1 - 0.5 / 2))
    /// / (1 - -1) == 1.55 / 2 = 0.775, which maps into bin int(0.775 * 4) == 3,
    /// which maps back into the action space as -1 + 3 * 0.5 == 0.5. Similarly,
    /// an input value of 0.2 would be normalized to 0.725, which maps into bin
    /// int(0.725 * 4) == 2.
    FeatureVector quantize(const FeatureVector& continuous,
                           const FeatureVectorInfo& info,
                           const int bins)
    {
        if (bins == 0)
            return continuous;
        FeatureVector discrete(continuous);
        for (size_t i = 0; i < continuous.size(); ++i)
        {
            if (info.isDiscrete(i))
            {
                discrete[i] = int(continuous[i]);
            }
            else
            {
                double lo = info.getMin(i), hi = info.getMax(i), span = hi - lo;
                double inc = span / (bins - 1);
                double interp = (continuous[i] - (lo - inc / 2)) / span;
                discrete[i] = lo + inc * int((bins - 1) * interp);
            }
        }
        return discrete;
    }

    /// quantize a continuous action vector
    FeatureVector TDBrain::quantize_action(const FeatureVector& new_action) const
    {
        return quantize(new_action, mInfo.actions, action_bins);
    }

    /// quantize a continuous state vector
    FeatureVector TDBrain::quantize_state(const FeatureVector& new_state) const
    {
        return quantize(new_state, mInfo.sensors, state_bins);
    }

    /// called right before the agent dies
    bool TDBrain::destroy()
    {
        return true;
    }

}
