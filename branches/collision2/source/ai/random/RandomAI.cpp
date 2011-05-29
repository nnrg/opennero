#include "core/Common.h"
#include "RandomEnvironment.h"

#include "ai/random/RandomAI.h"
#include "math/Random.h"
#include "core/Log.h"
#include <sstream>

namespace OpenNero
{
    RandomAITemplate::RandomAITemplate(const RandomAITemplate& objTempl) :
        AIObjectTemplate(objTempl), mSensorInfo(objTempl.mSensorInfo),
            mActionInfo(objTempl.mActionInfo),
            mRewardInfo(objTempl.mRewardInfo)
    {
    }

    RandomAITemplate::RandomAITemplate(SimFactoryPtr factory, const PropertyMap& propMap, const std::string& prefix) :
        AIObjectTemplate(factory, propMap), 
        mSensorInfo(), 
        mActionInfo(), 
        mRewardInfo()
    {
        if (propMap.hasSection(prefix + ".SensorInfo"))
        {
            propMap.getValue(mSensorInfo, prefix + ".SensorInfo",
                             SensorInfo::Converter() );
        }
        else
        {
            LOG_WARNING("Using default sensor information");
            mSensorInfo.addContinuous(0, 1);
            mSensorInfo.addContinuous(0, 1);
            mSensorInfo.addContinuous(0, 1);
        }

        if (propMap.hasSection(prefix + ".ActionInfo"))
        {
            propMap.getValue(mActionInfo, prefix + ".ActionInfo",
                             ActionInfo::Converter() );
        }
        else
        {
            LOG_WARNING("Using default action information");
            mActionInfo.addContinuous(0, 1);
            mActionInfo.addContinuous(0, 1);
            mActionInfo.addContinuous(0, 1);
        }

        if (propMap.hasSection(prefix + ".RewardInfo"))
        {
            propMap.getValue(mRewardInfo, prefix + ".RewardInfo",
                             RewardInfo::Converter() );
        }
        else
        {
            LOG_WARNING("Using default reward information");
            mRewardInfo.addContinuous(0, 1);
        }
    }

    RandomAITemplate::~RandomAITemplate()
    {
        // nada
    }

    AIObjectPtr RandomAITemplate::CreateObject(EnvironmentPtr env, SimEntityPtr ent)
    {
        return AIObjectPtr();
    }

}

