#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "tiles2.h"

namespace OpenNero
{
    /// @param info information about the agent for which this approximator is to be used
    TableApproximator::TableApproximator(const AgentInitInfo& info) :
        Approximator(info)
        , table()
    {
    }

    /// copy constructor
    TableApproximator::TableApproximator(const TableApproximator& a) :
        Approximator(a)
        , table(a.table)
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
        StateActionPair key(observation,action);
        StateActionDoubleMap::iterator found = table.find(key);
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
        StateActionPair key(observation,action);
        table[key] = target;
    }

    /// Create a new tiles approximator based on the agent description provided
    /// @param info agent interface description
    TilesApproximator::TilesApproximator(const AgentInitInfo& info) 
        : Approximator(info)
        , mAlpha(0.1f)
        , ints_index()
        , floats_index()
        , num_tilings(32)
        , num_weights(1024)
        , ints(NULL)
        , floats(NULL)
        , tiles(NULL)
        , weights(NULL)
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
        ints = new int[ints_index.size()];
        floats = new float[floats_index.size()];
        tiles = new int[num_tilings];
        weights = new float[num_weights];
        for (size_t i = 0; i < num_weights; ++i)
        {
            weights[i] = RANDOM.normalF(0,1);
        }
    }

    TilesApproximator::TilesApproximator(const TilesApproximator& a)
        : Approximator(a.mInfo)
        , mAlpha(a.mAlpha)
        , ints_index(a.ints_index)
        , floats_index(a.floats_index)
        , num_tilings(a.num_tilings)
        , num_weights(a.num_weights)
        , ints(NULL)
        , floats(NULL)
        , tiles(NULL)
        , weights(NULL)
    {
        ints = new int[ints_index.size()];
        memcpy(ints, a.ints, ints_index.size() * sizeof(int));
        floats = new float[floats_index.size()];
        memcpy(floats, a.floats, floats_index.size() * sizeof(float));
        tiles = new int[num_tilings];
        memcpy(tiles, a.tiles, num_tilings * sizeof(int));
        weights = new float[num_weights];
        memcpy(weights, a.weights, num_weights * sizeof(float));
    }

    TilesApproximator::~TilesApproximator()
    {
        SAFE_DELETE_ARRAY(ints);
        SAFE_DELETE_ARRAY(floats);
        SAFE_DELETE_ARRAY(tiles);
        SAFE_DELETE_ARRAY(weights);
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
        GetTiles(tiles, num_tilings, num_weights, floats, floats_index.size(), ints, ints_index.size());
    }
    
    /// @param observation sensor vector
    /// @param action action vector
    double TilesApproximator::predict(const FeatureVector& observation, const FeatureVector& action)
    {
        to_tiles(observation, action);
        double result=0.0;
        for (size_t i = 0; i < num_tilings; ++i) 
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
        for (size_t i = 0; i < num_tilings; ++i) 
        {
            weights[tiles[i]] += (float)(mAlpha/num_tilings * (target - x));
        }
    }
}
