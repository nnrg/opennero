#ifndef _OPENNERO_AI_RL_TD_H_
#define _OPENNERO_AI_RL_TD_H_

#include <iostream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

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
        friend class boost::serialization::access;

        double mGamma;   ///< reward discount factor (between 0 and 1)
        double mAlpha;   ///< learning rate (between 0 and 1)
        double mEpsilon; ///< parameter for the epsilon-greedy policy (between 0 and 1)
        AgentInitInfo mInfo; ///< initialization info
        std::vector< Actions > action_list; ///< list of possible actions
        ApproximatorPtr mApproximator; ///< function approximator we are using
        Actions action;      ///< previous action taken
        Observations state;  ///< previous state
        Actions new_action;  ///< new action
        int action_bins; ///< number of discrete bins for action space.
        int state_bins; ///< number of discrete bins for state space.
        int num_tiles; ///< number of discrete bins for action space.
        int num_weights; ///< number of discrete bins for state space.

    	// predicts reinforcement for current round
    	virtual double predict(const Observations& new_state) = 0;
    public:
        /// constructor
        /// @param gamma reward discount factor (between 0 and 1)
    	/// @param alpha learning rate (between 0 and 1)
        /// @param epsilon parameter for the epsilon-greedy policy (between 0 and 1)
        /// @param actions number of bins for quantizing continuous action dimensions
        /// @param states number of bins for quantizing continuous state space dimensions
        TDBrain(double gamma, double alpha, double epsilon, int actions, int states, int tiles, int weights)
        : AgentBrain()
        , mGamma(gamma)
        , mAlpha(alpha)
        , mEpsilon(epsilon)
        , mInfo()
        , mApproximator()
        , action()
        , state()
        , new_action()
        , action_bins(actions)
        , state_bins(states)
        , num_tiles(tiles)
        , num_weights(weights)
        {}

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
        , action_bins(3)
        , state_bins(5)
        , num_tiles(0)
        , num_weights(0)
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
        , action_bins(agent.action_bins)
        , state_bins(agent.state_bins)
        , num_tiles(agent.num_tiles)
        , num_weights(agent.num_weights)
        {}

        /// destructor
        virtual ~TDBrain() {}

        /// called right before the agent is born
        virtual bool initialize(const AgentInitInfo& init);

        /// called for agent to take its first step
        virtual Actions start(const TimeType& time, const Observations& new_state);

        /// act based on time, sensor arrays, and last reward
        virtual Actions act(const TimeType& time, const Observations& new_state, const Reward& reward);

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

        std::string to_string() const;
        void from_string(const std::string& s);

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(mGamma);
            ar & BOOST_SERIALIZATION_NVP(mAlpha);
            ar & BOOST_SERIALIZATION_NVP(mEpsilon);
            ar & BOOST_SERIALIZATION_NVP(action_bins);
            ar & BOOST_SERIALIZATION_NVP(state_bins);
            ar & BOOST_SERIALIZATION_NVP(num_tiles);
            ar & BOOST_SERIALIZATION_NVP(num_weights);
            ar & BOOST_SERIALIZATION_NVP(mInfo);
            ar & BOOST_SERIALIZATION_NVP(action_list);
            ar & BOOST_SERIALIZATION_NVP(mApproximator);
        }
    };
} // namespace OpenNero

#endif // _OPENNERO_AI_RL_TD_H_
