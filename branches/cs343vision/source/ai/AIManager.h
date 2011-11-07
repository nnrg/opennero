#ifndef _OPENNERO_AI_AIMANAGER_H_
#define _OPENNERO_AI_AIMANAGER_H_

#include <set>
#include "ai/AI.h"

namespace OpenNero
{
    using namespace std;

    /// @cond
    BOOST_PTR_DECL(AgentBrain);
    BOOST_PTR_DECL(Environment);
    BOOST_PTR_DECL(AI);
    /// @endcond

    /// A global manager class for the OpenNero AI framework
    class AIManager
    {
        // private constructor
        AIManager() : mEnabled(false), mEnvironment() {}

    public:
        /// singleton instance of class
        static AIManager& instance();

        /// const singleton instance of class
        static const AIManager& const_instance();

        /// enable or disable AI
        /// @param state on or off state
        void SetEnabled(bool state);

        /// return true iff AI is enabled
        bool IsEnabled() const { return mEnabled; }
        
        /// tick the AIs
        void ProcessTick( float32_t incAmt );

        /// get the currently selected AI Environment
        EnvironmentPtr GetEnvironment() const;

        /// set the currently selected AI Environment
        void SetEnvironment(EnvironmentPtr env);

        /// get the named AI instance if available
        AIPtr GetAI(const std::string& name) const;

        /// set the named AI instance
        void SetAI(const std::string& name, AIPtr ai);

        /// log the performance of AI agents
        void Log(SimId id, size_t episode, size_t step, Reward reward, Reward fitness);
        
        /// reset the ai (remove the ai systems)
        void Reset();

        /// Shutdown and clean-up the AI subsystem
        void destroy();

    private:
        bool mEnabled; ///< global "disable AI" switch
        EnvironmentPtr mEnvironment; ///< current environment
        std::map<std::string, AIPtr> mAIs; ///< AIs currently used
    };

}

#endif /*AIMANAGER_H_*/
