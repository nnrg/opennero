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
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/AIManager.h"

namespace OpenNero
{
	/// default constructor
	/// SimEntityData 
	SimEntity::SimEntity(const SimEntityData& data, const std::string& templateName) :		
		mAIObject(),
#if NERO_BUILD_AUDIO
		mAudioObject(),
#endif /// NERO_BUILD_AUDIO
		mSceneObject(),
		mSharedData(data),
		mCreationTemplate(templateName)
	{
	}

    SimEntity::~SimEntity() {
	}

    SimEntityPtr SimEntity::CreateSimEntity(
        SimEntityData& data, 
        const std::string& templateName, 
        SimContextPtr context)
    {
        SimEntityPtr ent(new SimEntity(data, templateName));
        InitializeSimEntity(ent, data, templateName, context);
        return ent;
    }

    // This method is supposed to refine and extend the current data in the entity
    void SimEntity::InitializeSimEntity(
        SimEntityPtr ent, 
        SimEntityData& data, 
        const std::string& templateName, 
        SimContextPtr context)
    {
        {
            SceneObjectTemplatePtr objTemp = context->getObjectTemplate<SceneObjectTemplate>(templateName);
            if (objTemp)
            {
                SceneObjectPtr sceneObj;
                sceneObj.reset(new SceneObject(ent));
                if (sceneObj->LoadFromTemplate(objTemp, data) )
                {
                    ent->SetSceneObject(sceneObj);
                    ent->SetCollision(ent->GetCollision() | objTemp->mCollisionMask);
                }
                
            }
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
        {
            AIObjectTemplatePtr aiTemplate = context->getObjectTemplate<AIObjectTemplate>(templateName);
            if (aiTemplate)
            {
                EnvironmentPtr env = AIManager::instance().GetEnvironment();
                AssertMsg(env, "Environment is not set up when creating an AI agent!");
                AIObjectPtr aiObj = aiTemplate->CreateObject(env, ent);
                if (aiObj && aiObj->LoadFromTemplate(aiTemplate, data)) {
                    ent->SetAIObject(aiObj);
                }
            }
        }
        ent->SetCreationTemplate( templateName );
    }

    void SimEntity::ProcessTick(float32_t incAmt)
    {
        if (mSceneObject)
        {
            // This call will update the pose of the Irrlicht object to 
            // correspond to our mSharedData
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
            // This call is the meat and potatoes of OpenNERO proper:
            // if this object has a decision to make will ask the 
            // Environment about what it sees and tell it how it wants to 
            // act.
            mAIObject->ProcessTick(incAmt);
        }
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
                mSceneObject->SetSharedState( &mSharedData );
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
            mAudioObject->SetSharedState( &mSharedData );
        }
    }
#endif // NERO_BUILD_AUDIO

    void SimEntity::SetAIObject(AIObjectPtr obj)
    {
        mAIObject = obj;
        if (mAIObject )
        {
            mAIObject->SetSharedState( &mSharedData );
        }
    }

    /// make sure you explicitly call this method in the assignment operator in
    /// the derived class
    SimEntity& SimEntity::operator=( const SimEntity& obj )
    {
        mSharedData = obj.GetState();
        SetSceneObject( obj.mSceneObject );
        return *this;
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
    
    uint32_t SimEntity::GetType() const
    {
        return mSharedData.GetType();
    }
    
    uint32_t SimEntity::GetCollision() const
    {
        return mSharedData.GetCollision();
    }

    void SimEntity::SetPosition( const Vector3f& pos )
    {
        mSharedData.SetPosition(pos);
    }

    bool SimEntity::CanCollide() const 
    {
        return mSharedData.GetCollision() != 0;
    }

	/// Get the set of objects colliding
    bool SimEntity::IsColliding( const SimEntitySet& others)
    {
        SimEntitySet::const_iterator iter; // iterate over the list
        for (iter = others.begin(); iter != others.end(); ++iter)
        {
            SimEntityPtr ent = *iter;
            if (ent.get() != this && // don't collide with self
                (mSharedData.GetCollision() & ent->GetType()) &&
                mSceneObject->isColliding(mSharedData.GetPosition(), ent->mSceneObject))
            {
                return true;
            }
        }
        return false;
    }
    
    /// Assume that a collision occurred and resolve it (bounce)
    /// For now we do this just by setting the position back to what it was 
    /// before (which is stored in mSceneObject).
    void SimEntity::ResolveCollision()
    {
        //LOG_F_DEBUG("collision", "ResolveCollision id: " << GetSimId() << " old: " << GetPosition() << " new: " << mSceneObject->getPosition());
		SetPosition(mSceneObject->getPosition());
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
    
    void SimEntity::SetCollision(uint32_t mask)
    {
        mSharedData.SetCollision(mask);
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
