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
        mEnvironment.reset();
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
                        double reward, 
                        double fitness)
    {
        stringstream ss;
        GetStaticTimer().stamp(ss);
        ss << " (M) [ai.tick] " << id <<
            "\t" << episode <<
            "\t" << step <<
            "\t" << reward <<
            "\t" << fitness << endl;
        ScriptingEngine::instance().NetworkWrite(ss.str());
    }

    AIPtr getAI(const std::string& name)
    {
        return AIManager::instance().GetAI(name);
    }

    void AIManager::SetAI(const std::string& name, AIPtr ai)
    {
        mAIs[name] = ai;
    }

    void setAI(const std::string& name, AIPtr ai)
    {
        AIManager::instance().SetAI(name, ai);
    }

    /// enable or disable AI
    void switch_ai(bool state)
    {
        AIManager::instance().SetEnabled(state);
    }

    /// toggle AI between on and off
    void toggle_ai()
    {
        AIManager::instance().SetEnabled(!AIManager::instance().IsEnabled());
    }

    /// enable AI
    void enable_ai()
    {
        AIManager::instance().SetEnabled(true);
    }

    /// disable AI
    void disable_ai()
    {
        AIManager::instance().SetEnabled(false);
    }
    
    /// reset environment
    void reset_ai()
    {
    }

    /// get the currently running environment
    EnvironmentPtr get_environment()
    {
        return AIManager::const_instance().GetEnvironment();
    }

    /// set the environment
    void set_environment(shared_ptr<PyEnvironment> env)
    {
        AIManager::instance().SetEnvironment(env);
    }

    /// export AI on/off toggle functions
    PYTHON_BINDER( AIManager )
    {
        // TODO: make these methods more organized
        boost::python::def("switch_ai", &switch_ai, "switch AI");
        boost::python::def("toggle_ai", &toggle_ai, "enable or disable AI");
        boost::python::def("enable_ai", &enable_ai, "enable AI");
        boost::python::def("disable_ai", &disable_ai, "disable AI");
        boost::python::def("reset_ai", &reset_ai, "reset AI");
        boost::python::def("get_environment", &get_environment, "get the current environment");
        boost::python::def("set_environment", &set_environment, "set the current environment");

		boost::python::def("get_ai", &getAI, "return AIPtr");
		boost::python::def("set_ai", &setAI,"set AI ptr");
    }
}
