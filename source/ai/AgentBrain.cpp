#include "core/Common.h"
#include "scripting/scriptIncludes.h"
#include "AgentBrain.h"
#include "ai/rl/TD.h"
#include "ai/rl/Sarsa.h"
#include "ai/rl/QLearning.h"

namespace OpenNero
{
    using namespace boost::python;

    /// Should the next call to act be skipped? (clears the flag)
    bool AgentBrain::GetSkip() const
    {
        if (skip_flag) {
            skip_flag = false;
            return true;
        } else {
            return false;
        }
    }

    /// Causes the agent to ignore collisions and to be placed exactly where specified by state
    void AgentBrain::Teleport()
    {
        SimEntityPtr ent = mBody.lock()->GetEntity();
        ent->UpdateImmediately();
    }

    /// called right before the agent is born
    bool PyAgentBrain::initialize(const AgentInitInfo& init_info)
    {
        // initialize (potentially multi-objective) fitness
        fitness = init_info.reward.getInstance();
        bool result(false);
        TryOverride("initialize", result, init_info);
        return result;
    }

    /// called for agent to take its first step
    Actions PyAgentBrain::start(const TimeType& time, const Observations& observations)
    {
        Actions result;
        TryOverride("start", result, time, observations);
        return result;
    }

    /// act based on time, sensor arrays, and last reward
    Actions PyAgentBrain::act(const TimeType& time, const Observations& observations, const Reward& reward)
    {
        Actions result;
        TryOverride("act", result, time, observations, reward);
        return result;
    }

    /// called to tell agent about its last reward
    bool PyAgentBrain::end(const TimeType& time, const Reward& reward)
    {
        bool result(false);
        TryOverride("end", result, time, reward);
        return result;
    }

    /// called right before the agent dies
    bool PyAgentBrain::destroy()
    {
        bool result(false);
        TryOverride("destroy", result);
        return result;
    }

    /// use a template to initialize this object
	bool PyAgentBrain::LoadFromTemplate( ObjectTemplatePtr t, const SimEntityData& data )
	{
        /// TODO: implement Python-side templates
        return false;
    }
}
