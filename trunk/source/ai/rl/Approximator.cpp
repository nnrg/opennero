#include <boost/serialization/export.hpp>

#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "tiles2.h"

namespace OpenNero
{
    /// @param info information about the agent for which this approximator is to be used
    TableApproximator::TableApproximator(const AgentInitInfo& info, const int actions, const int states) :
        Approximator(info)
        , table()
        , action_bins(actions)
        , state_bins(states)
    {
    }

    /// copy constructor
    TableApproximator::TableApproximator(const TableApproximator& a) :
        Approximator(a)
        , table(a.table)
        , action_bins(a.action_bins)
        , state_bins(a.state_bins)
    {
    }

    /// destructor
    TableApproximator::~TableApproximator()
    {
    }

    /// predict the value associated with a particular feature vector
    /// @param observation observation to update
    /// @param action action to update
    /// @return currently approximated (exact) value
    double TableApproximator::predict(const FeatureVector& observation, const FeatureVector& action)
    {
        const FeatureVector& s = quantize_state(observation);
        const FeatureVector& a = quantize_action(action);
        StateActionDoubleMap::iterator found = table.find(StateActionPair(s, a));
        if (found == table.end())
        {
            return 0;
        }
        else
        {
            return found->second;
        }

    }

    /// @param observation observation to update
    /// @param action action to update
    /// @param target new value for this state/action pair
    void TableApproximator::update(const FeatureVector& observation, const FeatureVector& action, double target)
    {
        const FeatureVector& s = quantize_state(observation);
        const FeatureVector& a = quantize_action(action);
        table[StateActionPair(s, a)] = target;
    }

    /// given a feature vector from a continuous space, quantize each component
    /// based on the range for the component and the number of discrete (linear)
    /// bins we want for each dimension of our discretized space.
    ///
    /// This is similar in spirit to the action space enumeration in initialize
    /// above. I'll continue the example from there, i.e., that we want to map a
    /// dimension that spans [-1, 1] into 5 equal bins, with centers at the
    /// values {-1 (bin #0), -.5, 0, .5, 1 (bin #4)}.
    ///
    /// Suppose we get a continuous value of 0.3 for this dimension. This should
    /// probably fit into bin #3 (by a "closest center" metric -- similarly, 0.2
    /// should fit into bin #2). We first map [-1, 1] onto {0, 1, 2, 3, 4}, and
    /// then we map the integral bin number back into [-1, 1] by adding the
    /// lower bound (-1) to the appropriate multiple of
    /// "inc" == span / (bins - 1).
    ///
    /// The tricky bit here is that to map each bin's interval (e.g., for bin
    /// #3, the interval is [.25, .75)) we basically end up subtracting inc / 2
    /// from everything, which effectively converts the bin centers into the
    /// barriers that divide them.
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
    FeatureVector TableApproximator::quantize_action(const FeatureVector& action) const
    {
        return quantize(action, mInfo.actions, action_bins);
    }

    /// quantize a continuous state vector
    FeatureVector TableApproximator::quantize_state(const FeatureVector& state) const
    {
        return quantize(state, mInfo.sensors, state_bins);
    }

    /// Create a new tiles approximator based on the agent description provided
    /// @param info agent interface description
    TilesApproximator::TilesApproximator(const AgentInitInfo& info, const int num_tiles, const int num_weights)
        : Approximator(info)
        , mAlpha(0.1f)
        , ints_index()
        , floats_index()
        , ints()
        , floats()
        , tiles()
        , weights()
    {
        LOG_F_DEBUG("ai", "TilesApproximator( "  << info << " )");
        size_t num_sensors = info.sensors.size();
        size_t num_actions = info.actions.size();
        for (size_t i = 0; i < info.sensors.size(); ++i)
        {
            if (info.sensors.isDiscrete(i))
            {
                ints_index.push_back(i);
            }
            else
            {
                floats_index.push_back(i);
            }
        }
        for (size_t i = num_sensors; i < num_sensors + num_actions; ++i)
        {
            if (info.actions.isDiscrete(i - num_sensors))
            {
                ints_index.push_back(i);
            }
            else
            {
                floats_index.push_back(i);
            }
        }
        Assert(floats_index.size() + ints_index.size() == num_sensors + num_actions);
        ints.resize(ints_index.size());
        floats.resize(floats_index.size());
        tiles.resize(num_tiles);
        weights.resize(num_weights);
        for (size_t i = 0; i < weights.size(); ++i)
        {
            weights[i] = RANDOM.normalF(0,1);
        }
    }

    TilesApproximator::TilesApproximator(const TilesApproximator& a)
        : Approximator(a.mInfo)
        , mAlpha(a.mAlpha)
        , ints_index(a.ints_index)
        , floats_index(a.floats_index)
        , ints(a.ints)
        , floats(a.floats)
        , tiles(a.tiles)
        , weights(a.weights)
    {
    }

    TilesApproximator::~TilesApproximator()
    {
    }

    /// convert feature vector into tiles
    void TilesApproximator::to_tiles(const FeatureVector& observation, const FeatureVector& action)
    {
        size_t num_sensors = mInfo.sensors.size();
        size_t num_actions = mInfo.actions.size();
        Assert(num_sensors == observation.size());
        Assert(num_actions == action.size());
        // shuffle feature vector into the arrays
        for (size_t i = 0; i < ints_index.size(); ++i)
        {
            if (ints_index[i] < num_sensors)
            {
                ints[i] = (int)observation[ints_index[i]];
            }
            else
            {
                ints[i] = (int)action[ints_index[i] - num_sensors];
            }
        }
        for (size_t i = 0; i < floats_index.size(); ++i)
        {
            if (floats_index[i] < num_sensors)
            {
                floats[i] = (float)observation[floats_index[i]];
            }
            else
            {
                floats[i] = (float)action[floats_index[i] - num_sensors];
            }
        }
        // convert the input values to tiles
        GetTiles(tiles, weights.size(), floats, ints);
    }
    
    /// @param observation sensor vector
    /// @param action action vector
    double TilesApproximator::predict(const FeatureVector& observation, const FeatureVector& action)
    {
        to_tiles(observation, action);
        double result=0.0;
        for (size_t i = 0; i < tiles.size(); ++i) 
        {
            result += weights[tiles[i]];
        }
        return result;
    }
    
    /// Adapt the tile weights for the tiles that are triggered by the given example
    /// @param observation sensor vector
    /// @param action action vector
    /// @param target output target
    void TilesApproximator::update(const FeatureVector& observation, const FeatureVector& action, double target)
    {
        double x = predict(observation, action);
        // then, adapt weights towards the prediction
        for (size_t i = 0; i < tiles.size(); ++i) 
        {
            weights[tiles[i]] += (float)(mAlpha / tiles.size() * (target - x));
        }
    }
}

BOOST_CLASS_EXPORT(OpenNero::Approximator)
BOOST_CLASS_EXPORT(OpenNero::TableApproximator)
BOOST_CLASS_EXPORT(OpenNero::TilesApproximator)
