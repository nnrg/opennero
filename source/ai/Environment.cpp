#include "core/Common.h"
#include "Environment.h"
#include "Task.h"
#include "Run.h"

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

    bool PyEnvironment::is_active(AgentBrainPtr agent)
    {
        bool result(false);
        TryOverride("is_active", result, agent);
        return result;
    }

    /// sense the agent's current environment
    Sensors PyEnvironment::sense(AgentBrainPtr agent)
    {
        Sensors result;
        TryOverride("sense", result, agent);
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

    /// @cond
    BOOST_PTR_DECL(PyEnvironment);
    BOOST_PTR_DECL(PyTask);
    /// @endcond

    /// Export World-specific script components
    PYTHON_BINDER( Environment )
    {
        // export the interface to python so that we can override its methods there
        class_<PyEnvironment, boost::noncopyable, PyEnvironmentPtr >("Environment", "Abstract base class for implementing an environment")
            .def("get_agent_info", pure_virtual(&Environment::get_agent_info), "Get the blueprint for creating new agents")
            .def("sense", pure_virtual(&Environment::sense), "sense the agent's current environment" )
            .def("is_episode_over", pure_virtual(&Environment::is_episode_over), "is the episode over for the specified agent?")
            .def("is_active", pure_virtual(&Environment::is_active), "is the agent active and should it act")
            .def("step", pure_virtual(&Environment::step), "Get a step for an agent")
            .def("cleanup", pure_virtual(&Environment::cleanup), "Clean up when the environment is removed")
            .def("reset", pure_virtual(&Environment::reset), "reset the environment to its initial state");

        implicitly_convertible<PyEnvironmentPtr, EnvironmentPtr >();

        class_<PyTask, boost::noncopyable, PyTaskPtr>("Task", "A base class for representing the reward structure of a particular task", init<string>())
            .def("start", &Task::start, "start the task")
            .def("step", &Task::step, "step the task")
            .def("stop", &Task::stop, "stop the task");

        implicitly_convertible<PyTaskPtr, TaskPtr>();

        class_<Run, boost::noncopyable, RunPtr>("Run", "Run is an abstract class that represents an learning experiment or a sequence of episodes", no_init);

        class_<SimpleRun, boost::noncopyable, bases<Run>, SimpleRunPtr>("SimpleRun", "A SimpleRun combines a set of agents with an environment and a task for them to perform", init<EnvironmentPtr, AgentSet, TaskPtr>("update the environment e with agents a during task t"));

        class_<SequenceRun, boost::noncopyable, bases<Run>, SequenceRunPtr>("SequenceRun", "A SequenceRun consits of some number of runs executed in sequence", init<>());

        class_<RepeatRun, boost::noncopyable, bases<Run>, RepeatRunPtr>("RepeatRun", "A RepeatRun allows a Run to be repeated some number of times", init<RunPtr, size_t>("repeat the run for n times"));
    }
}






