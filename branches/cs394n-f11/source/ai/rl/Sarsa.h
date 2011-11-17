#ifndef _OPENNERO_AI_RL_SARSA_H_
#define _OPENNERO_AI_RL_SARSA_H_

#include "core/Common.h"
#include "ai/rl/TD.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL(SarsaBrain);
    /// @endcond

    /// A SARSA reinforcement learning agent
    class SarsaBrain : public TDBrain
    {
            double mLambda;                 ///< parameter for the SARSA(lambda) learning algorithm
            double cumulative_reward;       ///< cumulative reward
            size_t n_episodes;              ///< number of episodes
		protected:
			// predicts reinforcement for current round
			virtual double predict(const Observations& new_state);
        public:
            /// constructor
            /// @param gamma reward discount factor (between 0 and 1)
        	/// @param alpha learning rate (between 0 and 1)
            /// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        	/// @param lambda parameter for the SARSA(lambda) learning algorith
            /// @param actions number of bins for quantizing continuous action dimensions
            /// @param states number of bins for quantizing continuous state space dimensions
            SarsaBrain(double gamma, double alpha, double epsilon, double lambda, int actions, int states, int tiles, int weights)
            : TDBrain(gamma, alpha, epsilon, actions, states, tiles, weights)
            , mLambda(lambda)
			, cumulative_reward(0)
			, n_episodes(0)
            {}

            /// constructor
            /// @param gamma reward discount factor (between 0 and 1)
        	/// @param alpha learning rate (between 0 and 1)
            /// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        	/// @param lambda parameter for the SARSA(lambda) learning algorith
            SarsaBrain(double gamma, double alpha, double epsilon, double lambda)
            : TDBrain(gamma, alpha, epsilon)
            , mLambda(lambda)
			, cumulative_reward(0)
			, n_episodes(0)
            {}

            /// copy constructor
            SarsaBrain(const SarsaBrain& agent)
            : TDBrain(agent)
			, mLambda(agent.mLambda)
			, cumulative_reward(agent.cumulative_reward)
			, n_episodes(agent.n_episodes)
			{}

            /// destructor
            virtual ~SarsaBrain() {}

            /// called for agent to take its first step
            virtual Actions start(const TimeType& time, const Observations& o);

            /// act based on time, sensor arrays, and last reward
            virtual Actions act(const TimeType& time, const Observations& o, const Reward& reward);

            /// called to tell agent about its last reward
            virtual bool end(const TimeType& time, const Reward& reward);
    };

} // namespace OpenNero

#endif // _OPENNERO_AI_RL_SARSA_H_
