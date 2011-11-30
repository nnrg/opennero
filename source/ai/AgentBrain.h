//---------------------------------------------------
// Name: OpenNero : Agent
// Desc: Interface for an AI agent acting in the world
//---------------------------------------------------

#ifndef _OPENNERO_AI_AGENT_H_
#define _OPENNERO_AI_AGENT_H_

#include "core/ONTime.h"
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "game/objects/TemplatedObject.h"
#include "scripting/scriptIncludes.h"
#include "scripting/scripting.h"

namespace OpenNero
{

	/// @cond
    BOOST_PTR_DECL(AIObject);
    BOOST_PTR_DECL(Sensor);
    /// @endcond

    using namespace boost;

    /// AI agent acting in the world
    class AgentBrain : public TemplatedObject
    {
            AIObjectWPtr mBody; ///< AIObject where this brain is attached

        public:

            std::string name; ///< name of this brain

            size_t episode; ///< episode count

            size_t step; ///< step count

            Reward fitness; ///< cumulative reward for the current episode

            mutable bool skip_flag; ///< causes the next call to act to be skipped

        public:
            /// constructor
            AgentBrain() : mBody(), name(), episode(0), step(0), fitness(0), skip_flag(false) {}

            /// destructor
            virtual ~AgentBrain() {}

            /// called right before the agent is born
            virtual bool initialize(const AgentInitInfo& init) = 0;

            /// called for agent to take its first step
            virtual Actions start(const TimeType& time, const Observations& observations) = 0;

            /// act based on time, sensor arrays, and last reward
            virtual Actions act(const TimeType& time, const Observations& observations, const Reward& reward) = 0;

            /// called to tell agent about its last reward
            virtual bool end(const TimeType& time, const Reward& reward) = 0;

            /// called right before the agent dies
            virtual bool destroy() = 0;

            /// get the current fitness of the agent
            Reward get_fitness() { return fitness; }

            /// add a sensor to this agent's body
            size_t add_sensor(SensorPtr s) { return GetBody()->add_sensor(s); }

            /// Causes the next call to act to be skipped
            void Skip() { skip_flag = true; }

            /// Should the next call to act be skipped? (clears the flag)
            bool GetSkip() const;

            /// Causes the agent to ignore collisions and to be placed exactly where specified by state
            void Teleport();

            /// set the body associated with this agent
            virtual void SetBody(AIObjectPtr body) { mBody = body; }

            /// get the body associated with this agent
            virtual AIObjectPtr GetBody() { return mBody.lock(); }

            /// get the shared simulation data for adjusting pose
            SimEntityData* GetSharedState() { return GetBody()->GetSharedState(); }
    };

    /// shared pointer to an AgentBrain
    BOOST_SHARED_DECL(AgentBrain);

    /// C++ interface for Python-side AgentBrain
    class PyAgentBrain : public AgentBrain, public TryWrapper<AgentBrain>
    {
        public:
            /// called right before the agent is born
            bool initialize(const AgentInitInfo& init);

            /// called for agent to take its first step
            Actions start(const TimeType& time, const Observations& observations);

            /// act based on time, sensor arrays, and last reward
            Actions act(const TimeType& time, const Observations& observations,
                        const Reward& reward);

            /// called to tell agent about its last reward
            bool end(const TimeType& time, const Reward& reward);

            /// called right before the agent dies
            bool destroy();

            /// use a template to initialize this object
			bool LoadFromTemplate(ObjectTemplatePtr t, const SimEntityData& data);
    };

    /// shared pointer to a PyAgentBrain
    BOOST_SHARED_DECL(PyAgentBrain);
}

#endif
