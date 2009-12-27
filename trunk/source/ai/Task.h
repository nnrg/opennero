#ifndef _OPENNERO_AI_TASK_H_
#define _OPENNERO_AI_TASK_H_

#include <string>
#include "AI.h"
#include "Environment.h"

namespace OpenNero
{
    using namespace std;

    /// @cond
    BOOST_PTR_DECL(Task);
    /// @endcond

    /// A Task represents the reward structure of particular learning task
    /// It provides a way to return the reward and to perform actions before
    /// and after the task is run.
    class Task
    {
        string _name;
    public:
        /// constructor
        explicit Task(string name) : _name(name) {}

        /// virtual destructor
        virtual ~Task() {}

        /// start the task
        virtual void start() {}

        /// get the action, and return the reward
        virtual Reward step(AgentBrainPtr agent, Actions action, EnvironmentPtr env)
        {
            return env->step(agent, action); // pass to environment
        }

        /// stop the task
        virtual void stop() {}
    };
    
    /// Python wrapper for the Task
    class PyTask : public Task, public TryWrapper<Task>
    {
    public:
        /// Construtor
        PyTask(string name) : Task(name) {}
        
        void start();
        
        Reward step(AgentBrainPtr agent, Actions action, EnvironmentPtr env);
        
        void stop();
    };
}

#endif
