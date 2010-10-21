//---------------------------------------------------
// Name: OpenNero : Environment
// Desc: Interfaces for the Environment in which the AI system exists.
//---------------------------------------------------
#ifndef _OPENNERO_AI_WORLD_H_
#define _OPENNERO_AI_WORLD_H_

#include <vector>
#include "scripting/scriptIncludes.h"
#include "scripting/scripting.h"
#include "AI.h"

namespace OpenNero
{
    using namespace std;

	/// @cond
    BOOST_PTR_DECL(Environment);
	BOOST_PTR_DECL(PyEnvironment);
	/// @endcond

    /// An abstract interface that represents the simulated world
    /// A world can have objects, (human) players, (embodied) agents and AIs
    class Environment
    {
    public:
        /// virtual destructor
        virtual ~Environment() = 0;

        /// get the information needed to create an agent suitable for this world
        virtual AgentInitInfo
            get_agent_info(AgentBrainPtr agent = AgentBrainPtr()) = 0;

        /// @brief perform agent actions in the environment and receive the reward
        virtual Reward step(AgentBrainPtr agent, Actions action) = 0;

        /// called to find out if the agent is active and should act
        virtual bool is_active(AgentBrainPtr agent) = 0;

        /// @brief check if the episode for the agent is over
        virtual bool is_episode_over(AgentBrainPtr agent) = 0;

        /// @brief passively sense the agent's environment
        virtual Observations sense(AgentBrainPtr agent) = 0;

        /// cleanup the world on close
        virtual void cleanup() = 0;

        /// reset the environment to its initial state
        virtual void reset(AgentBrainPtr agent) = 0;
    };

    /**
     * @brief A concrete wrapper for the Python implementations of the World interface
     * This wrapper is needed because we need a concrete C++ class to be able to call the
     * Python implementation from the C++ side. It simply calls through to its overriding methods.
     * This is only necessary for virtual interfaces overridden from Python.
     */
    class PyEnvironment : public Environment, public TryWrapper<Environment>
    {
        /// get the information needed to create an agent suitable for this world
        AgentInitInfo get_agent_info(AgentBrainPtr agent);

        /// perform agent actions in the environment and return their sensors
        Reward step(AgentBrainPtr agent, Actions action);

        /// called to find out if the agent is active and should act
        bool is_active(AgentBrainPtr agent);

        /// sense the agent's current environment
        Observations sense(AgentBrainPtr agent);

        /// is the episode over for the specified agent?
        bool is_episode_over(AgentBrainPtr agent);

        /// cleanup the world on close
        void cleanup();

        /// reset the environment to its initial state
        void reset(AgentBrainPtr agent);
    };

}

#endif
