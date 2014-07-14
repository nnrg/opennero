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
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/AIManager.h"

namespace OpenNero
{
	/// default constructor
	/// SimEntityData
	SimEntity::SimEntity(const SimEntityData& data, const std::string& templateName) :
		mAIObject(),
		mSceneObject(),
		mSharedData(data),
		mCreationTemplate(templateName),
        mRemoved(false)
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

    void SimEntity::BeforeTick(float32_t incAmt)
    {
        // before we get here, we tried to set sceneobject to mSharedData.current,
        // but it could be different now due to collisions

        bool collided = false;

        // we update mSharedData to sync up with the on-screen position
        if (mSceneObject->collisionOccurred()) {
            mSharedData.SetPosition(mSceneObject->getPosition());
            collided = true;
        }

        // we tick the shared data so that it can remember where it was
        mSharedData.ProcessTick(incAmt);
    }

    void SimEntity::TickScene(float32_t incAmt)
    {
        if (mSceneObject)
        {
            // This call will update the pose of the Irrlicht object to
            // correspond to our mSharedData
            mSceneObject->ProcessTick(incAmt);
        }
    }

    void SimEntity::TickAI(float32_t incAmt)
    {
        if (mAIObject)
        {
            // This call is the meat and potatoes of OpenNERO proper:
            // if this object has a decision to make will ask the
            // Environment about what it sees and tell it how it wants to
            // act.
            mAIObject->ProcessTick(incAmt);
        }
    }

    void SimEntity::ProcessAnimationTick(float32_t frac)
    {
        if( mSharedData.IsDirty(SimEntityData::kDB_Position) )
        {
            Vector3f pos(mSharedData.GetPosition() * frac + mSharedData.GetPrevious().mPosition * (1.0 - frac));
            // convert from open nero's coordinate system to irrlicht's
            mSceneObject->SetPosition( pos );
        }

        if( mSharedData.IsDirty(SimEntityData::kDB_Rotation) )
        {
        //    if (mCamera && mFPSCamera)
        //    {
        //        mFPSCamera->UpdateRotation(mSharedData, mCamera);
        //    }
        //
            Vector3f rotation = InterpolateNeroRotation(mSharedData.GetPrevious().mRotation, mSharedData.GetRotation(), frac);
            // Irrlicht expects a left handed basis with the x-z plane being horizontal and y being up
            // OpenNero uses a right handed basis with x-y plane being horizontal and z being up
            mSceneObject->SetRotation( rotation );
        }

        //if ( mSharedData->IsDirty(SimEntityData::kDB_Scale) )
        //{
        //    // set the node scale
        //    Vector3f scale = mSceneObjectTemplate->mScale;
        //    /// we can optionally multiply by a custom scale
        //    scale.X = scale.X * mSharedData->GetScale().X;
        //    scale.Y = scale.Y * mSharedData->GetScale().Y;
        //    scale.Z = scale.Z * mSharedData->GetScale().Z;
        //    // convert from open nero's coordinate system to irrlicht's
        //    mSceneNode->setScale( ConvertNeroToIrrlichtPosition(scale) );
        //}

    }

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

    void SimEntity::UpdateImmediately()
    {
        // set all the bits to indicate that the information was updated
        mSharedData.SetAllDirtyBits();
        mSceneObject->DisregardCollisions();
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
