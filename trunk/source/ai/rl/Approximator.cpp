#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "tiles2.h"

namespace OpenNero
{
    /// constructor
    TableApproximator::TableApproximator(const AgentInitInfo& info) :
        Approximator(info)
        , table()
    {
    }

    TableApproximator::TableApproximator(const TableApproximator& a) :
        Approximator(a)
        , table(a.table)
    {
    }

    TableApproximator::~TableApproximator()
    {
    }

    /// predict the value associated with a particular feature vector
    double TableApproximator::predict(const FeatureVector& sensors, const FeatureVector& actions)
    {
        StateActionPair key(sensors,actions);
        TableMap::iterator found = table.find(key);
        if (found == table.end())
        {
            return 0;
        }
        else 
        {
            return found->second;
        }

    }

    /// update the value associated with a particular feature vector
    void TableApproximator::update(const FeatureVector& sensors, const FeatureVector& actions, double target)
    {
        StateActionPair key(sensors,actions);
        table[key] = target;
    }

    TilesApproximator::TilesApproximator(const AgentInitInfo& info) 
        : Approximator(info)
        , mAlpha(0.1)
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
    void TilesApproximator::to_tiles(const FeatureVector& sensors, const FeatureVector& actions)
    {
        size_t num_sensors = mInfo.sensors.size();
        size_t num_actions = mInfo.actions.size();
        Assert(num_sensors == sensors.size());
        Assert(num_actions == actions.size());
        // shuffle feature vector into the arrays
        for (size_t i = 0; i < ints_index.size(); ++i)
        {
            if (ints_index[i] < num_sensors)
            {
                ints[i] = (int)sensors[ints_index[i]];
            }
            else
            {
                ints[i] = (int)actions[ints_index[i] - num_sensors];
            }
        }
        for (size_t i = 0; i < floats_index.size(); ++i)
        {
            if (floats_index[i] < num_sensors)
            {
                floats[i] = (float)sensors[floats_index[i]];
            }
            else
            {
                floats[i] = (float)actions[floats_index[i] - num_sensors];
            }
        }
        // convert the input values to tiles
        GetTiles(tiles, num_tilings, num_weights, floats, floats_index.size(), ints, ints_index.size());
    }
    
    /// @param sensors sensor vector
    /// @param actions action vector
    double TilesApproximator::predict(const FeatureVector& sensors, const FeatureVector& actions)
    {
        to_tiles(sensors, actions);
        double result=0.0;
        for (size_t i = 0; i < num_tilings; ++i) 
        {
            result += weights[tiles[i]];
        }
        return result;
    }
    
    /// Adapt the tile weights for the tiles that are triggered by the given example
    /// @param sensors sensor vector
    /// @param actions action vector
    /// @param target output target
    void TilesApproximator::update(const FeatureVector& sensors, const FeatureVector& actions, double target)
    {
        // first, predict the value
        double x = predict(sensors, actions);
        // then, adapt weights towards the prediction
        for (size_t i = 0; i < num_tilings; ++i) 
        {
            weights[tiles[i]] += (float)(mAlpha/num_tilings * (target - x));
        }
    }
}
