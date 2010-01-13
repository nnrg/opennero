#ifndef _OPENNERO_AI_AIOBJECT_H_
#define _OPENNERO_AI_AIOBJECT_H_

#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include "game/objects/TemplatedObject.h"
#include "game/objects/SimEntityComponent.h"
#include "ai/AI.h"

namespace OpenNero
{
	/// @cond
	BOOST_SHARED_DECL(AIObject);
    BOOST_SHARED_DECL(AIObjectTemplate);
    BOOST_SHARED_DECL(Environment);
    BOOST_SHARED_DECL(SimEntity);
    /// @endcond

    class SimEntityData;

    /// interface for objects connecting an AgentBrain to a SimEntity body
    class AIObject : public enable_shared_from_this<AIObject>, public SimEntityComponent, public TemplatedObject
    {
    public:
        /// create an AIObject for the specified world
        /// @param world the Environment to use for this Python object
        /// @param parent the SimEntity that owns this Body
        AIObject(EnvironmentPtr world, SimEntityPtr parent);

        /// destructor
        virtual ~AIObject();

        /// get the AI move and apply it to the shared data
        virtual void ProcessTick(float32_t dt);

        /// sense the agent's environment
        virtual Sensors Sense();

        /// get initialization information for the agent
        virtual SensorInfo GetSensorInfo() = 0;

        /// display this AI object as a string
        virtual std::ostream& stream(std::ostream& out) const = 0;

        /// set the currently selected actions for this AI object
        void setActions(Actions actions) { mActions = actions; }
        /// get the most recently set actions
        Actions getActions() const { return mActions; }

        /// set the brain of this AIObject
        void setBrain(AgentBrainPtr brain) { mAgentBrain = brain; }
        /// get the current brain of this AIObject
        AgentBrainPtr getBrain() const { return mAgentBrain; }

        /// set the environment in which this AIObject operates
        void setWorld(EnvironmentPtr world) { mWorld = world; }

        /// get the environment in which this AIObject operates
        EnvironmentPtr getWorld() const { return mWorld.lock(); }

        /// set the most recent reward for this AIObject
        void setReward(Reward reward);

        /// get the most recent reward for this AIObject
        Reward getReward() const { return mReward; }

    private:

        Actions mActions; ///< last performed action
        AgentBrainPtr mAgentBrain; ///< the brain whose actions we are applying
        EnvironmentWPtr mWorld; ///< world we are acting in
        Reward mReward; ///< the reward received by the agent after performing the previous action
    };

    /// print an AI object to stream
    std::ostream& operator<<(std::ostream& out, AIObject& obj);

    /// template for loading an AI object from an XML configuration file
    class AIObjectTemplate : public ObjectTemplate
    {
    public:
        /// constructor
        AIObjectTemplate(const AIObjectTemplate& objTempl) : ObjectTemplate(objTempl) {}
        /// constructor
        AIObjectTemplate(SimFactoryPtr factory, const PropertyMap& propMap) : ObjectTemplate(factory, propMap) {}
        /// destructor
        virtual ~AIObjectTemplate() {}

        /// create an AIObject from this template
        virtual AIObjectPtr CreateObject(EnvironmentPtr env, SimEntityPtr ent) = 0;

        /// create the concrete type of template we need
        static AIObjectTemplatePtr createTemplate(SimFactoryPtr factory, const PropertyMap& propMap);

        /// return the string representation of this type
        static const std::string TemplateType()
        {
            return "AIObject";
        }
    };

}

#endif /*AIOBJECT_H_*/
