#include "core/Common.h"
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/AIManager.h"
#include "ai/Environment.h"
#include "ai/SensorArray.h"
#include "ai/AgentBrain.h"
#include "ai/PythonAI.h"
#include "ai/random/RandomAI.h"
#include "core/IrrUtil.h"
#include "game/SimEntityData.h"
#include "math/Random.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    /**
    * Factory method to create the appropriate type of AIObjectTemplate depending on the XML data provided
    * @param factory SimFactory used to grab resources
    * @param propMap property map that contains the information from the XML file
    * @return an instance of an AIObjectTemplate child suitable for generating new AI objects
    */
    AIObjectTemplatePtr AIObjectTemplate::createTemplate(SimFactoryPtr factory, const PropertyMap& propMap)
    {
        if (propMap.hasSection(RandomAITemplate::TemplateType())) 
        {
            AIObjectTemplatePtr p(new RandomAITemplate(factory, propMap, RandomAITemplate::TemplateType()));
            return p;
        }
        if (propMap.hasSection(PythonAITemplate::TemplateType())) 
        {
            AIObjectTemplatePtr p(new PythonAITemplate(factory, propMap, PythonAITemplate::TemplateType()));
            return p;
        }
        return AIObjectTemplatePtr();
    }

    /// create a new AI object to live in the specified world
    AIObject::AIObject(EnvironmentPtr world, SimEntityPtr parent) 
        : SimEntityComponent(parent)
        , mAgentBrain()
        , mWorld(world)
        , mReward()
    {
    }

    AIObject::~AIObject()
    {
        // nada
    }

    /// get the AI move and apply it to the shared data
    void AIObject::ProcessTick(float32_t dt)
    {
        Assert(getBrain());
        if (getBrain()->step == 0) // if first step
        {
            Sensors sensors = getWorld()->sense(getBrain());
            setActions(getBrain()->start(dt, sensors));
            setReward(getWorld()->step(getBrain(), getActions()));
            getBrain()->step++;
            if (mSharedData && mSharedData->GetLabel().empty() && !getBrain()->name.empty())
            {
                mSharedData->SetLabel(getBrain()->name);
            }
        }
        else
        {
            Assert(getWorld());
            if (getWorld()->is_active(getBrain()))
            {
                if (getWorld()->is_episode_over(getBrain())) {
                    getBrain()->end(dt, getReward());
                    getWorld()->reset(getBrain());
                    getBrain()->episode++;
                    getBrain()->step = 0;
                    getBrain()->fitness = 0;
                } else {
                    Sensors sensors = getWorld()->sense(getBrain());
                    setActions(getBrain()->act(dt, sensors, getReward()));
                    setReward(getWorld()->step(getBrain(), getActions()));
                    getBrain()->step++;
                }
            }
        }
    }
    
    void AIObject::setReward(Reward reward)
    {
        mReward = reward;
        getBrain()->fitness += reward;
        AIManager::instance().Log
            (GetSharedState()->GetId(),
             getBrain()->episode,
             getBrain()->step,
             getReward(),
             getBrain()->fitness);
    }

    /// sense the agent's environment
    Sensors AIObject::Sense()
    {
        return Sensors();
    }

    inline std::ostream& operator<<(std::ostream& out, AIObject& obj)
    {
        return obj.stream(out);
    }
}
