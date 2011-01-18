#ifndef _OPENNERO_AI_RANDOM_RANDOMAI_H_
#define _OPENNERO_AI_RANDOM_RANDOMAI_H_

#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/Environment.h"
#include "ai/random/RandomAgent.h"
#include "ai/random/RandomEnvironment.h"
#include "math/Random.h"

namespace OpenNero
{

	/// @cond
    BOOST_SHARED_DECL(RandomAITemplate);
    BOOST_SHARED_DECL(RandomAgent);
    /// @endcond

    /// template for creating a random AI object from an XML file
    class RandomAITemplate : public AIObjectTemplate
    {
    public:
        /// copy constructor
        RandomAITemplate(const RandomAITemplate& objTempl);
        /// constructor for a template given a factory, propMap, and a prefix
        RandomAITemplate(SimFactoryPtr factory, const PropertyMap& propMap, const std::string& prefix);
        virtual ~RandomAITemplate();

        // prefix of this template used in the XML file
        static const std::string TemplateType()
        {
            return "Template.AI.Random";
        }

        /// get info about AI's sensors
        SensorInfo getSensorInfo() const { return mSensorInfo; }
        /// get info about AI's actions
        ActionInfo getActionInfo() const { return mActionInfo; }
        /// get info about AI's rewards
        RewardInfo getRewardInfo() const { return mRewardInfo; }

        AIObjectPtr CreateObject(EnvironmentPtr env, SimEntityPtr ent);
    private:
        SensorInfo mSensorInfo;
        ActionInfo mActionInfo;
        RewardInfo mRewardInfo;
    };
}

#endif /*RANDOMAI_H_*/
