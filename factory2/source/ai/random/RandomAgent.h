#ifndef _OPENNERO_AI_RANDOM_RANDOMAGENT_H_
#define _OPENNERO_AI_RANDOM_RANDOMAGENT_H_

#include "ai/AI.h"
#include "ai/AgentBrain.h"

namespace OpenNero
{

	/// @cond
    BOOST_SHARED_DECL(RandomAITemplate);
    BOOST_SHARED_DECL(RandomAgent);
    /// @endcond

    /// Random agent for testing
    class RandomAgent : public AgentBrain
    {
    public:
        RandomAgent();

        /// called right before the agent is born
        bool initialize(const AgentInitInfo& init);

        /// start a new episode
        Actions start(const TimeType& time, const Observations& observations);

        /// act based on time, sensor arrays, and last reward
        Actions act(const TimeType& time, const Observations& observations, const Reward& reward);

        /// end an episode
        bool end(const TimeType& time, const Reward& reward);

        /// called right before the agent dies
        bool destroy();

        /// load random brain from template
        bool LoadFromTemplate(ObjectTemplatePtr t, const SimEntityData& data);
    private:
        AgentInitInfo _init; ///< agent initialization information
        Actions get_random_action(); ///< return random action based on _init
    };

}

#endif /*RANDOMAGENT_H_*/
