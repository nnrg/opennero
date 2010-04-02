#ifndef _OPENNERO_AI_RUN_H_
#define _OPENNERO_AI_RUN_H_

#include "AI.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL(Run);
    BOOST_PTR_DECL(SimpleRun);
    BOOST_PTR_DECL(SequenceRun);
    BOOST_PTR_DECL(RepeatRun);
    BOOST_PTR_DECL(Task);
    /// @endcond

    /// Run is an abstract class that represents an learning experiment or a sequence of episodes
    class Run
    {
    public:
        /// virtual destructor
        virtual ~Run() {}

        /// update the environment with the effects of performing all the agents' actions
        virtual bool step() = 0;
    };

    /// A SimpleRun combines a set of agents with an environment and a task for them to perform
    class SimpleRun : public Run
    {
        EnvironmentWPtr _env; ///< environment we are updating
        AgentSet _agents; ///< agents that are included in this run
        TaskPtr _task; ///< task these agents are performing
    public:
        SimpleRun(EnvironmentPtr env, AgentSet agents, TaskPtr task); ///< constructor for a simple run given the environment, agents, and a task
        bool step();
    };

    /// A SequenceRun consits of some number of runs executed in sequence
    class SequenceRun : public Run
    {
        std::vector<RunPtr> _runs; ///< runs to be performed
    public:
        SequenceRun();
        void append(RunPtr run);   ///< add a run to the current sequence
        bool step();
    };

    /// A RepeatRun allows a Run to be repeated some number of times
    class RepeatRun : public Run
    {
        RunPtr _run;
        size_t _number;
        size_t _current_run;
    public:
        /// Repeat the given run a given number of times
        RepeatRun(RunPtr run, size_t number);
        bool step();
    };
}

#endif
