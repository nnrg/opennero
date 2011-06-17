#include "core/Common.h"
#include "Environment.h"

#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    using namespace boost::python;

    /// Constructor
    Environment::~Environment()
    {
        // do nothing here
    }

    /// get the information needed to create an agent suitable for this world
    AgentInitInfo PyEnvironment::get_agent_info(AgentBrainPtr agent)
    {
        AgentInitInfo result;
        TryOverride("get_agent_info", result, agent);
        return result;
    }

    /// perform agent actions in the environment and return their sensors
    Reward PyEnvironment::step(AgentBrainPtr agent, Actions action)
    {
        Reward result(0);
        TryOverride("step", result, agent, action);
        return result;
    }

    /// sense the agent's current environment
    Observations PyEnvironment::sense(AgentBrainPtr agent, Observations& observations)
    {
        Observations result;
        TryOverride("sense", result, agent, observations);
        return result;
    }

    /// is the episode over for the specified agent?
    bool PyEnvironment::is_episode_over(AgentBrainPtr agent)
    {
        bool result(false);
        TryOverride("is_episode_over", result, agent);
        return result;
    }

    /// cleanup the world on close
    void PyEnvironment::cleanup()
    {
        TryOverride("cleanup");
    }

    /// reset the environment to its initial state
    void PyEnvironment::reset(AgentBrainPtr agent)
    {
        bool result = false;
        TryOverride("reset", result, agent);
        // ignore result
    }
}






