//--------------------------------------------------------
// OpenNero : SimEntity
//  a 'thing' that can be simulated
//--------------------------------------------------------

#include "core/Common.h"
#include <iostream>
#include "core/Log.h"
#include "core/Bitstream.h"
#include "game/SimEntity.h"
#include "game/SimContext.h"
#include "render/SceneObject.h"
#include "audio/AudioObject.h"
#if NERO_BUILD_PHYSICS
#include "physics/Physics.h"
#include "physics/PhysicsObject.h"
#endif // NERO_BUILD_PHYSICS
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/AIManager.h"
#include "utils/Performance.h"

namespace OpenNero
{
	/// default constructor
	/// SimEntityData 
	SimEntity::SimEntity(const SimEntityData& data, const std::string& templateName) :		
		mAIObject(),
#if NERO_BUILD_AUDIO
		mAudioObject(),
#endif /// NERO_BUILD_AUDIO
#if NERO_BUILD_PHYSICS
		mPhysicsObject(),
#endif // NERO_BUILD_PHYSICS
		mSceneObject(),
		mSharedData(data),
		mCreationTemplate(templateName)
	{
		// do nothing
	}

    SimEntity::~SimEntity() {}

    SimEntityPtr SimEntity::CreateSimEntity(
        SimEntityData& data, 
        const std::string& templateName, 
        SimContextPtr context)
    {
        SimEntityPtr ent(new SimEntity(data, templateName));
        InitializeSimEntity(ent, data, templateName, context);
        LOG_F_DEBUG("game", "created new object: " << ent);
        return ent;
    }

    // This method is supposed to refine and extend the current data in the entity
    void SimEntity::InitializeSimEntity(
        SimEntityPtr ent, 
        SimEntityData& data, 
        const std::string& templateName, 
        SimContextPtr context)
    {
        // object template
        ObjectTemplatePtr objTemp;

        objTemp = context->getObjectTemplate<SceneObjectTemplate>(templateName);
        SceneObjectPtr sceneObj;
        if (objTemp)
        {
            sceneObj.reset(new SceneObject(ent));
            if (sceneObj->LoadFromTemplate(objTemp, data) )
            {
                ent->SetSceneObject(sceneObj);
            }
#if NERO_BUILD_PHYSICS
			IPhysicsObjectPtr physicsObj = IPhysicsEngine::instance().createObject(ent);
            if (physicsObj->LoadFromTemplate(objTemp, data))
            {
                ent->SetPhysicsObject(physicsObj);
            }
#endif // NERO_BUILD_PHYSICS
        }
#if NERO_BUILD_AUDIO
        // audio template
        objTemp
            = context->getObjectTemplate<AudioObjectTemplate>(templateName);
        if (objTemp )
        {
            AudioObjectPtr audioObj(new AudioObject());
            if (audioObj->LoadFromTemplate(objTemp, data) )
            {
                ent->SetAudioObject(audioObj);
            }
        }
#endif // NERO_BUILD_AUDIO
        AIObjectTemplatePtr aiTemplate = context->getObjectTemplate<AIObjectTemplate>(templateName);
        if (aiTemplate)
        {
            LOG_F_DEBUG( "ai", "Creating AI Object");
            EnvironmentPtr env = AIManager::instance().GetEnvironment();
            AssertMsg(env, "Environment is not set up when creating an AI agent!");
            AIObjectPtr aiObj = aiTemplate->CreateObject(env, ent);
            if (aiObj)
            {
                aiObj->LoadFromTemplate(aiTemplate, data);
                ent->SetAIObject(aiObj);
            }
        }
        ent->SetCreationTemplate( templateName );
    }

    void SimEntity::ProcessTick(float32_t incAmt)
    {
        NERO_PERF_EVENT_SCOPED( SimEntity_ProcessTick );

        // go through components and tick them
        // TODO : If we knew exactly what a client and a server object should
        // have in terms of components this could be optimized

        if (mSceneObject)
        {
            // perform client-side interpolation based on current velocity
            // TODO: replace with interpolation between current and future pose
            //Vector3f vel = GetVelocity();
            //Vector3f pos = GetPosition();
            //Vector3f move = incAmt * vel;
            //if (move.getLengthSQ() > 0) 
            //{
            //    SetPosition( pos + move );
            //}
            mSceneObject->ProcessTick(incAmt);
        }

#if NERO_BUILD_AUDIO
        if (mAudioObject )
        {   
            mAudioObject->ProcessTick(incAmt);
        }
#endif // NERO_BUILD_AUDIO

        if (mAIObject && AIManager::const_instance().IsEnabled())
        {   
            mAIObject->ProcessTick(incAmt);
        }
#if NERO_BUILD_PHYSICS
        if (mPhysicsObject && IPhysicsEngine::instance().IsEnabled())
        {  
            mPhysicsObject->ProcessTick(incAmt);
        }
#endif // NERO_BUILD_PHYSICS
    }

#if NERO_BUILD_AUDIO
    // Windows thinks it owns everything...
    #ifdef PlaySound
    #undef PlaySound
    #endif

    /// play a sound of a given type name
    /// @param soundName the name of the sound (ie: "jump", "shoot", "onDeath", ... )
    /// @return true if the sound got played
    bool SimEntity::PlaySound(const std::string& soundName)
    {
        if ( !mAudioObject )
            return false;

        return mAudioObject->PlaySound(soundName);
    }
#endif // NERO_BUILD_AUDIO

    void SimEntity::SetCreationTemplate( const std::string& creationTemplate )
    {
        mCreationTemplate = creationTemplate;
    }

    std::string SimEntity::GetCreationTemplate() const
    {
        return mCreationTemplate;
    }

    /**
     * Get the SimId. const ref just in case we decide
     * to expand the SimId structure to be more informative
     * @return the simId
     */
    SimId SimEntity::GetSimId() const
    {
        return mSharedData.GetId();
    }

    /// set the scene object associated with this
    void SimEntity::SetSceneObject(SceneObjectPtr obj)
    {
        if (mSceneObject != obj )
        {
            mSceneObject = obj;
            if (mSceneObject )
            {
                mSceneObject->SetSharedData( &mSharedData );
            }
        }
    }

#if NERO_BUILD_AUDIO
    /// Set the audio object
    /// @param obj the audio object to use
    void SimEntity::SetAudioObject(AudioObjectPtr obj)
    {
        mAudioObject = obj;
        if (mAudioObject )
        {
            mAudioObject->SetSharedData( &mSharedData );
        }
    }
#endif // NERO_BUILD_AUDIO

    void SimEntity::SetAIObject(AIObjectPtr obj)
    {
        mAIObject = obj;
        if (mAIObject )
        {
            mAIObject->SetSharedData( &mSharedData );
        }
    }

#if NERO_BUILD_PHYSICS
    void SimEntity::SetPhysicsObject(IPhysicsObjectPtr obj)
    {
        mPhysicsObject = obj;
        if (mPhysicsObject )
        {
            mPhysicsObject->SetSharedData( &mSharedData );
        }
    }
#endif // NERO_BUILD_PHYSICS

    /// make sure you explicitly call this method in the assignment operator in
    /// the derived class
    SimEntity& SimEntity::operator=( const SimEntity& obj )
    {
        mSharedData = obj.GetData();
        SetSceneObject( obj.mSceneObject );
        return *this;
    }

    /// @param ray The ray that is collided with this entity.
    /// @param irr Handle to Irrlicht objects.
    /// @param outCollisionPoint The collision point of the ray with this entity.
    /// @return true if collision occurs; false otherwise.
    bool SimEntity::GetCollisionPoint(const Line3f& ray, const IrrHandles& irr, Vector3f& outCollisionPoint) const
    {
        // get collider
        ISceneCollisionManager* collider = irr.mpSceneManager->getSceneCollisionManager();
        Assert(collider);
        // get scene node
        ISceneNode* sceneNode = mSceneObject->mSceneNode;
        if (!sceneNode)
        {
            return false;
        }
        Triangle3f resultTriangle; // ignored
        ISceneNode* node = collider->getSceneNodeAndCollisionPointFromRay(ray, outCollisionPoint, resultTriangle, 0, node);
        return (node != NULL);
    }

    const Vector3f& SimEntity::GetPosition() const
    {
        return mSharedData.GetPosition();
    }

    const Vector3f& SimEntity::GetRotation() const
    {
        return mSharedData.GetRotation();
    }

    const Vector3f& SimEntity::GetVelocity() const
    {
        return mSharedData.GetVelocity();
    }

    const Vector3f& SimEntity::GetScale() const
    {
        return mSharedData.GetScale();
    }
    
    const std::string& SimEntity::GetLabel() const
    {
        return mSharedData.GetLabel();
    }

    const SColor& SimEntity::GetColor() const
    {
        return mSharedData.GetColor();
    }

    void SimEntity::SetPosition( const Vector3f& pos )
    {
        mSharedData.SetPosition(pos);
    }

    void SimEntity::SetRotation( const Vector3f& rot )
    {
        mSharedData.SetRotation(rot);
    }

    void SimEntity::SetVelocity( const Vector3f& vel )
    {
        mSharedData.SetVelocity(vel);
    }

    void SimEntity::SetScale( const Vector3f& scale )
    {
        mSharedData.SetScale(scale);
    }

    void SimEntity::SetLabel(const std::string& label)
    {
        mSharedData.SetLabel(label);
    }

    void SimEntity::SetColor(const SColor& color)
    {
        mSharedData.SetColor(color);
    }

    /// output SimEntity to stream
    std::ostream& operator<<(std::ostream& stream, const SimEntityPtr& ent)
    {
        if (!ent)
        {
            stream << "SimEntityPtr()";
            return stream;
        }
        stream << ent->mSharedData;
        return stream;
    }
    
} //end OpenNero
