#include "core/Common.h"
#include "RandomEnvironment.h"

#include "RandomAI.h"
#include "math/Random.h"
#include "core/Log.h"
#include <sstream>

namespace OpenNero
{

    Observations RandomEnvironment::get_random_sensors() const
    {
        Observations result(mInitInfo.sensors.size());
        for (size_t i = 0; i < result.size(); ++i)
        {
            if (mInitInfo.sensors.isDiscrete(i))
            {
                result[i]
                    = RANDOM.randI( (uint32_t)(mInitInfo.sensors.getMax(i)
                        - mInitInfo.sensors.getMin(i)) )
                        + mInitInfo.sensors.getMin(i);
            }
            else
            {
                result[i]
                    = RANDOM.randD(mInitInfo.sensors.getMax(i)
                        - mInitInfo.sensors.getMin(i))
                        + mInitInfo.sensors.getMin(i);
            }
        }
        return result;
    }

    /// @brief perform agent actions in the environment and receive the reward
    Reward RandomEnvironment::step(AgentBrainPtr agent, Actions action)
    {
        return mInitInfo.reward.getRandom();
    }

    /// @brief passively sense the agent's environment
    Observations RandomEnvironment::sense(AgentBrainPtr agent, Observations& observations)
    {
        return get_random_sensors();
    }

    /// @brief cleanup the world on close
    void RandomEnvironment::cleanup()
    {
        // nothing
    }

}


