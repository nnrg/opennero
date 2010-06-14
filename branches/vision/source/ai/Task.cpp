#include "core/Common.h"
#include "Task.h"

#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    using namespace boost::python;
    
    void PyTask::start()
    {
        TryOverride("start");
    }

    Reward PyTask::step(AgentBrainPtr agent, Actions action, EnvironmentPtr env)
    {
        Reward result;
        TryOverride("step", result, agent, action, env);
        return result;
    }

    void PyTask::stop()
    {
        TryOverride("stop");
    }
}