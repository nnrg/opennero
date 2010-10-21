#include "core/Common.h"
#include "RandomAgent.h"
#include "RandomAI.h"

namespace OpenNero {
    Actions RandomAgent::get_random_action()
    {
        Actions result(_init.actions.size());
        for (size_t i = 0; i < result.size(); ++i)
        {
            if (_init.actions.isDiscrete(i))
            {
                result[i]
                    = RANDOM.randI( (uint32_t)(_init.actions.getMax(i)
                        - _init.actions.getMin(i)) ) + _init.actions.getMin(i);
            }
            else
            {
                result[i] = RANDOM.randD(_init.actions.getMax(i)
                    - _init.actions.getMin(i)) + _init.actions.getMin(i);
            }
        }
        return result;
    }

    RandomAgent::RandomAgent() :
        _init()
    {
        // nada
    }

    bool RandomAgent::initialize(const AgentInitInfo& init)
    {
        _init = init;
        return true;
    }

    Actions RandomAgent::start(const TimeType& time, const Observations& observations)
    {
        return this->get_random_action();
    }

    Actions RandomAgent::act(const TimeType& time, const Observations& observations,
                             const Reward& reward)
    {
        return this->get_random_action();
    }

    bool RandomAgent::end(const TimeType& time, const Reward& reward)
    {
        // nada
        return true;
    }

    bool RandomAgent::destroy()
    {
        // nada
        return true;
    }

    bool RandomAgent::LoadFromTemplate(ObjectTemplatePtr objTempl, const SimEntityData& data)
    {
        RandomAITemplatePtr randomAITempl = shared_static_cast<
            RandomAITemplate, ObjectTemplate>(objTempl);
        _init = AgentInitInfo(randomAITempl->getSensorInfo(),
                              randomAITempl->getActionInfo(),
                              randomAITempl->getRewardInfo());
        return true;
    }
}
