#include <cfloat>
#include <vector>
#include <algorithm>
#include <iostream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "ai/AI.h"
#include "TD.h"

namespace OpenNero
{

    /// called right before the agent is born
    bool TDBrain::initialize(const AgentInitInfo& init)
    {
        mInfo = init;
        this->fitness = mInfo.reward.getInstance();

        int bins = action_bins;

        if (num_tiles > 0)
        {
            AssertMsg(action_bins == 0, "action_bins must be 0 for num_tiles > 0");
            AssertMsg(state_bins == 0, "state_bins must be 0 for num_tiles > 0");
            mApproximator.reset(
                new TilesApproximator(mInfo, num_tiles, num_weights));
            bins = 7; // XXX force bins > 0 to discretize action_list below
        }
        else
        {
            AssertMsg(action_bins > 0, "action_bins > 0 for num_tiles == 0");
            AssertMsg(state_bins > 0, "action_bins > 0 for num_tiles == 0");
            mApproximator.reset(
                new TableApproximator(mInfo, action_bins, state_bins));
        }

        // Similar to FeatureVectorInfo::enumerate (from AI.cpp).
        //
        // We want to enumerate all possible actions in a discrete way, so that
        // we can store them in a policy table somehow. So, for each action
        // dimension, if it's discrete we just enumerate the integral values for
        // that dimension, and if it's continuous, we split it into "bins"
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
            const double inc = info.isDiscrete(i) ? 1.0f : (hi - lo) / (bins - 1);
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
		AssertMsg(reward.size() == 1, "multi-objective rewards not supported");
        // select new action and estimate its value
        double new_Q = epsilon_greedy(new_state);
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
            new_action = mInfo.actions.getRandom();
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

    /// serialize this brain to a text string
    std::string TDBrain::to_string() const
    {
        std::ostringstream oss;
        boost::archive::text_oarchive oa(oss);
        oa << *this;
        return oss.str();
    }

    /// deserialize this brain from a text string
    void TDBrain::from_string(const std::string& s)
    {
        try {
            std::istringstream iss(s);
            boost::archive::text_iarchive ia(iss);
            ia >> *this;
        } catch (boost::archive::archive_exception const& e) {
            LOG_F_ERROR("ai.rl", "unable to load agent because of error, " << e.what());
        }
    }
}

BOOST_CLASS_EXPORT(OpenNero::TDBrain)

