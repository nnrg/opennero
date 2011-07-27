#ifndef _OPENNERO_AI_RANDOM_RANDOMENVIRONMENT_H_
#define _OPENNERO_AI_RANDOM_RANDOMENVIRONMENT_H_

#include "ai/AI.h"
#include "ai/AgentBrain.h"
#include "ai/AIObject.h"
#include "ai/Environment.h"

namespace OpenNero
{

	/// @cond
    BOOST_SHARED_DECL(RandomAITemplate);
    BOOST_SHARED_DECL(RandomAgent);
    /// @endcond

    /// A dummy environment that generates random sensor values
    class RandomEnvironment : public Environment
    {
    private:
        AgentInitInfo mInitInfo; ///< environmental constrains: blueprint for sensors, actions and rewards

    protected:
        /// get a random sensor in compiance with the environmental constraints
        Observations get_random_sensors() const;

    public:

        /// consructor that takes in sensors and actions
        RandomEnvironment(U32 nSensors, U32 nActions) : mInitInfo(nSensors, nActions, 1)
        {
        }

        /// @brief destructor
        ~RandomEnvironment() {}

        /// @brief get the information needed to create an agent suitable for this world
        AgentInitInfo get_agent_info(AgentBrainPtr)
        {
            return mInitInfo;
        }

        /// @brief perform agent actions in the environment and receive the reward
        Reward step(AgentBrainPtr agent, Actions action);

        /// @brief passively sense the agent's environment
        Observations sense(AgentBrainPtr agent, Observations& observations);

        /// @brief cleanup the world on close
        void cleanup();
    };

}

#endif /*RANDOMENVIRONMENT_H_*/
