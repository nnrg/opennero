#include "core/Common.h"
#include "scripting/scriptIncludes.h"
#include "AgentBrain.h"
#include "ai/rl/TD.h"
#include "ai/rl/Sarsa.h"
#include "ai/rl/QLearning.h"

namespace OpenNero
{
    using namespace boost::python;

    /// called right before the agent is born
    bool PyAgentBrain::initialize(const AgentInitInfo& init_info)
    {
        bool result(false);
        TryOverride("initialize", result, init_info);
        return result;
    }

    /// called for agent to take its first step
    Actions PyAgentBrain::start(const TimeType& time, const Sensors& sensors)
    {
        Actions result;
        TryOverride("start", result, time, sensors);
        return result;
    }

    /// act based on time, sensor arrays, and last reward
    Actions PyAgentBrain::act(const TimeType& time, const Sensors& sensors, const Reward& reward)
    {
        Actions result;
        TryOverride("act", result, time, sensors, reward);
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

    /**
     * Export Agent-specific script components
     */
    PYTHON_BINDER( AgentBrain )
    {
        // export the interface to python so that we can override its methods there
        python::class_<PyAgentBrain, noncopyable, PyAgentBrainPtr>("AgentBrain", "Abstract brain for an AI agent")
            .def("initialize", pure_virtual(&AgentBrain::initialize), "Called before learning starts")
            .def("start", pure_virtual(&AgentBrain::start), "Called at the beginning of a learning episode")
            .def("act", pure_virtual(&AgentBrain::act), "Called for every step of the state-action loop")
            .def("end", pure_virtual(&AgentBrain::end), "Called at the end of a learning episode")
            .def("destroy", pure_virtual(&AgentBrain::destroy), "Called after learning ends")
            .def_readonly("step", &AgentBrain::step, "Current step count")
            .def_readonly("episode", &AgentBrain::episode, "Current episode count")
            .def_readonly("fitness", &AgentBrain::fitness, "Cumulative reward for this episode")
            .add_property("state", make_function(&AgentBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
        // export the interface to python so that we can override its methods there
        python::class_<TDBrain, noncopyable, bases<AgentBrain>, TDBrainPtr >("TDBrain", "CMAC tile coding SARSA agent", no_init )
            .def("initialize", &TDBrain::initialize, "Called before learning starts")
            .def("start", &TDBrain::start, "Called at the beginning of a learning episode")
            .def("act", &TDBrain::act, "Called for every step of the state-action loop")
            .def("end", &TDBrain::end, "Called at the end of a learning episode")
            .def("destroy", &TDBrain::destroy, "Called after learning ends")
            .add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
            .add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
            .add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
            .add_property("state", make_function(&TDBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
        // export the interface to python so that we can override its methods there
        python::class_<SarsaBrain, bases<TDBrain>, SarsaBrainPtr >("SarsaBrain", "CMAC tile coding SARSA agent", init<double, double, double, double>() )
            .def("initialize", &SarsaBrain::initialize, "Called before learning starts")
            .def("start", &SarsaBrain::start, "Called at the beginning of a learning episode")
            .def("act", &SarsaBrain::act, "Called for every step of the state-action loop")
            .def("end", &SarsaBrain::end, "Called at the end of a learning episode")
            .def("destroy", &SarsaBrain::destroy, "Called after learning ends")
            .add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
            .add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
            .add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
            .add_property("state", make_function(&SarsaBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
        // export the interface to python so that we can override its methods there
        python::class_<QLearningBrain, bases<TDBrain>, QLearningBrainPtr >("QLearningBrain", "CMAC tile coding SARSA agent", init<double, double, double>() )
            .def("initialize", &QLearningBrain::initialize, "Called before learning starts")
            .def("start", &QLearningBrain::start, "Called at the beginning of a learning episode")
            .def("act", &QLearningBrain::act, "Called for every step of the state-action loop")
            .def("end", &QLearningBrain::end, "Called at the end of a learning episode")
            .def("destroy", &QLearningBrain::destroy, "Called after learning ends")
            .add_property("epsilon", &TDBrain::getEpsilon, &TDBrain::setEpsilon)
            .add_property("alpha", &TDBrain::getAlpha, &TDBrain::setAlpha)
            .add_property("gamma", &TDBrain::getGamma, &TDBrain::setGamma)
            .add_property("state", make_function(&QLearningBrain::GetSharedState, return_value_policy<reference_existing_object>()), "Body of the agent");
		;
    }
}
