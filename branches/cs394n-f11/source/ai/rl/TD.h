#ifndef _OPENNERO_AI_RL_TD_H_
#define _OPENNERO_AI_RL_TD_H_

#include "core/Common.h"
#include "ai/AgentBrain.h"
#include "Approximator.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL(Approximator);
    BOOST_PTR_DECL(TDBrain);
    /// @endcond

    /// A TD reinforcement learning agent
    class TDBrain : public AgentBrain
    {
    protected:
        double mGamma;   ///< reward discount factor (between 0 and 1)
        double mAlpha;   ///< learning rate (between 0 and 1)
        double mEpsilon; ///< parameter for the epsilon-greedy policy (between 0 and 1)
        AgentInitInfo mInfo; ///< initialization info
        std::vector< Actions > action_list; ///< list of possible actions
        ApproximatorPtr mApproximator; ///< function approximator we are using
        Actions action;      ///< previous action taken
        Observations state;  ///< previous state
        Actions new_action;  ///< new action
    	// predicts reinforcement for current round
    	virtual double predict(const Observations& new_state) = 0;
    public:
        /// constructor
        /// @param gamma reward discount factor (between 0 and 1)
    	/// @param alpha learning rate (between 0 and 1)
        /// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
    	TDBrain(double gamma, double alpha, double epsilon)
        : AgentBrain()
        , mGamma(gamma)
        , mAlpha(alpha)
        , mEpsilon(epsilon)
        , mInfo()
        , mApproximator()
        , action()
        , state()
        , new_action()
        {}

        /// copy constructor
    	TDBrain(const TDBrain& agent)
        : AgentBrain()
        , mGamma(agent.mGamma)
        , mAlpha(agent.mAlpha)
        , mEpsilon(agent.mEpsilon)
        , mInfo(agent.mInfo)
        , mApproximator(agent.mApproximator->copy())
        , action(agent.action)
        , state(agent.state)
        , new_action(agent.new_action)
        {}

        /// destructor
        virtual ~TDBrain() {}

        /// called right before the agent is born
        virtual bool initialize(const AgentInitInfo& init);

        /// called for agent to take its first step
        virtual Actions start(const TimeType& time, const Observations& observations);

        /// act based on time, sensor arrays, and last reward
        virtual Actions act(const TimeType& time, const Observations& observations, const Reward& reward);

        /// called to tell agent about its last reward
        virtual bool end(const TimeType& time, const Reward& reward);

        /// called right before the agent dies
        virtual bool destroy();
        
        /// Set the gamma parameter
        /// @param gamma reward discount factor (between 0 and 1)
        void setGamma(double gamma) { mGamma = gamma; }
        
        /// Get the gamma parameter
        /// @return reward discount factor (between 0 and 1)
        double getGamma() { return mGamma; }
        
        /// Set learning rate
        /// @param alpha learning rate (between 0 and 1)
        void setAlpha(double alpha) { mAlpha = alpha; }
        
        /// Get learning rate
        /// @return learning rate (between 0 and 1)
        double getAlpha() { return mAlpha; }

        /// Epsilon parameter for the epsilon-greedy policy
        /// @param epsilon prob. of selecting a random action instead of the greedy one
        void setEpsilon(double epsilon) { mEpsilon = epsilon; }
        
        /// Epsilon parameter for the epsilon-greedy policy
        /// @return prob. of selecting a random action instead of the greedy one
        double getEpsilon() { return mEpsilon; }

        /// select action according to policy
        double epsilon_greedy(const Observations& new_state);

        /// load this object from a template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data ) 
			{ return false; /* TODO: implement when we have better template */ }
    };

} // namespace OpenNero

#endif // _OPENNERO_AI_RL_TD_H_
