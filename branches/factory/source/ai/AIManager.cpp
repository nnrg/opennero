#include "core/Common.h"
#include <vector>
#include "ai/AIManager.h"
#include "ai/AI.h"
#include "ai/AgentBrain.h"
#include "ai/Environment.h"
#include "core/Log.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{

    AIManager& AIManager::instance()
    {
        static AIManager me;
        return me;
    }

    const AIManager& AIManager::const_instance()
    {
        return instance();
    }

    void AIManager::SetEnabled(bool state)
    {
        if (state)
        {
            LOG_F_MSG("ai", "AI Engine enabled");
        }
        else
        {
            LOG_F_MSG("ai", "AI Engine disabled");
        }
        mEnabled = state;
    }

    /// Shutdown and clean-up the AI subsystem
    void AIManager::destroy()
    {
        SetEnabled(false);
        if (mEnvironment) {
            mEnvironment->cleanup();
            mEnvironment.reset();
        }
    }

    AIPtr AIManager::GetAI(const std::string& name) const
    {
        map<string, AIPtr>::const_iterator iter = mAIs.find(name);
        if (iter != mAIs.end()) 
        {
            return iter->second;
        }
        else
        {
            return AIPtr();
        }
    }

    void AIManager::Log(SimId id, 
                        size_t episode, 
                        size_t step, 
                        Reward reward, 
                        Reward fitness)
    {
        //stringstream ss;
        //GetStaticTimer().stamp(ss);
        //ss << " (M) [ai.tick] " << id <<
        //    "\t" << episode <<
        //    "\t" << step <<
        //    "\t" << reward <<
        //    "\t" << fitness << endl;
        //ScriptingEngine::instance().NetworkWrite(ss.str());
        LOG_F_DEBUG("ai.tick", id <<
            "\t" << episode <<
            "\t" << step <<
            "\t" << reward <<
            "\t" << fitness);
    }

    void AIManager::SetAI(const std::string& name, AIPtr ai)
    {
        mAIs[name] = ai;
    }
}
