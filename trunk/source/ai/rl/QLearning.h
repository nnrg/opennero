#ifndef _OPENNERO_AI_RL_QLEARNING_H_
#define _OPENNERO_AI_RL_QLEARNING_H_

#include "core/Common.h"
#include "ai/rl/TD.h"

namespace OpenNero
{
    /// @cond
	BOOST_PTR_DECL(QLearningBrain);
    /// @endcond

	/// A Q-Learning reinforcement learning agent
    class QLearningBrain : public TDBrain
    {
    protected:
    	// predicts reinforcement for current round
    	virtual double predict(const Observations& new_state);
	public:
		/// constructor
		/// @param gamma reward discount factor (between 0 and 1)
    	/// @param alpha learning rate (between 0 and 1)
		/// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        /// @param actions number of bins for quantizing continuous action dimensions
        /// @param states number of bins for quantizing continuous state space dimensions
        QLearningBrain(double gamma, double alpha, double epsilon, int actions, int states, int tiles, int weights)
        : TDBrain(gamma, alpha, epsilon, actions, states, tiles, weights)
		{}

		/// constructor
		/// @param gamma reward discount factor (between 0 and 1)
    	/// @param alpha learning rate (between 0 and 1)
		/// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        QLearningBrain(double gamma, double alpha, double epsilon)
        : TDBrain(gamma, alpha, epsilon)
		{}

		/// copy constructor
		QLearningBrain(const QLearningBrain& agent)
		: TDBrain(agent)
		{}

		/// destructor
		virtual ~QLearningBrain() {}
    };
}


#endif // _OPENNERO_AI_RL_QLEARNING_H_
