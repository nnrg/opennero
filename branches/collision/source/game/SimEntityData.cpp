//--------------------------------------------------------
// OpenNero : SimEntityData
//  data shared across components in a SimEntity
//--------------------------------------------------------

#include "core/Common.h"
#include "game/SimEntityData.h"
#include "game/Kernel.h"
#include "game/SimContext.h"
#include "core/IrrSerialize.h"

namespace OpenNero 
{
    SimEntityData::SimEntityInternals::SimEntityInternals()
        : mPosition()
        , mRotation()
        , mVelocity()
        , mScale(1,1,1)
        , mAcceleration()
        , mLabel()
        , mColor(0xFF, 0xFF, 0xFF, 0xFF)
        , mType()
        , mCollision()
    {
    }

    SimEntityData::SimEntityInternals::SimEntityInternals(const Vector3f& pos, 
                                 const Vector3f& rot, 
                                 const Vector3f& scale, 
                                 const std::string& label,
                                 uint32_t t,
                                 uint32_t collision)
        : mPosition(pos)
        , mRotation(rot)
        , mVelocity(0,0,0)
        , mScale(scale)
        , mAcceleration(0,0,0)
        , mLabel(label)
        , mColor(0xFF, 0xFF, 0xFF, 0xFF)
        , mType(t)
        , mCollision(collision)
    {
    }
    
    SimEntityData::SimEntityData()
        : mId()
        , mDirtyBits(uint32_t(-1))
        , mPrevious()
        , mCurrent()
    {
    }
    
    SimEntityData::SimEntityData(
        const Vector3f& pos, 
        const Vector3f& rot, 
        const Vector3f& scale, 
        const std::string& label,
        uint32_t t,
        uint32_t collision,
        SimId id)
        : mId(id)
        , mDirtyBits(uint32_t(-1))
        , mPrevious(pos, rot, scale, label, t, collision)
        , mCurrent(pos, rot, scale, label, t, collision)
    {
    }
    
    void SimEntityData::SetPosition( const Vector3f& pos )
    {
        if( pos != mCurrent.mPosition )
        {
            mCurrent.mPosition = pos;
            mDirtyBits |= kDB_Position;
        }
    }

    void SimEntityData::SetRotation( const Vector3f& rot )
    {
        if( rot != mCurrent.mRotation )
        {
            mCurrent.mRotation = rot;
            mDirtyBits |= kDB_Rotation;
        }
    }

    void SimEntityData::SetVelocity( const Vector3f& vel )
    {
        if( vel != mCurrent.mVelocity )
        {
            mCurrent.mVelocity = vel;
            mDirtyBits |= kDB_Velocity;
        }
    }

    void SimEntityData::SetAcceleration(const Vector3f& acc)
    {
        if (acc != mCurrent.mAcceleration )
        {
            mCurrent.mAcceleration = acc;
            mDirtyBits |= kDB_Acceleration;
        }
    }
    
    void SimEntityData::SetLabel(const std::string& label)
    {
        if (label != mCurrent.mLabel)
        {
            mCurrent.mLabel = label;
            mDirtyBits |= kDB_Label;
        }
    }

    void SimEntityData::SetColor(const SColor& color)
    {
        if (color != mCurrent.mColor)
        {
            mCurrent.mColor = color;
            mDirtyBits |= kDB_Color;
        }
    }

    void SimEntityData::SetScale( const Vector3f& scale )
    {
        if (scale != mCurrent.mScale )
        {
            mCurrent.mScale = scale;
            mDirtyBits |= kDB_Scale;
        }
    }
    
    void SimEntityData::SetType( uint32_t t )
    {
        if ( mCurrent.mType != t )
        {
            mCurrent.mType = t;
            mDirtyBits |= kDB_Type;
        }
    }

    void SimEntityData::SetCollision( uint32_t mask )
    {
        if ( mCurrent.mCollision != mask )
        {
            mCurrent.mCollision = mask;
            mDirtyBits |= kDB_Collision;
        }
    }

    void SimEntityData::SetAnimation( const std::string& animationType )
    {
        Kernel::GetSimContext()->SetObjectAnimation(mId, animationType);
    }
    
    void SimEntityData::SetAnimationSpeed( float32_t framesPerSecond )
    {
        Kernel::GetSimContext()->SetObjectAnimationSpeed(mId, framesPerSecond);
    }

    void SimEntityData::ClearDirtyBits()
    {
        mDirtyBits = 0;
    }
        
    const Vector3f& SimEntityData::GetPosition() const
    {
        return mCurrent.mPosition;
    }

    const Vector3f& SimEntityData::GetRotation() const
    {
        return mCurrent.mRotation;
    }

    const Vector3f& SimEntityData::GetVelocity() const
    {
        return mCurrent.mVelocity;
    }

    const Vector3f& SimEntityData::GetAcceleration() const
    {
        return mCurrent.mAcceleration;
    }

    const Vector3f& SimEntityData::GetScale() const
    {
        return mCurrent.mScale;
    }
    
    const std::string& SimEntityData::GetLabel() const
    {
        return mCurrent.mLabel;
    }

    const SColor& SimEntityData::GetColor() const
    {
        return mCurrent.mColor;
    }

    SimId SimEntityData::GetId() const
    {
        return mId;
    }
    
    uint32_t SimEntityData::GetType() const
    {
        return mCurrent.mType;
    }

    uint32_t SimEntityData::GetCollision() const
    {
        return mCurrent.mCollision;
    }

    std::string SimEntityData::GetAnimation() const
    {
        return Kernel::GetSimContext()->GetObjectAnimation(mId);
    }

    float32_t SimEntityData::GetAnimationSpeed() const
    {
        return Kernel::GetSimContext()->GetObjectAnimationSpeed(mId);
    }

    uint32_t SimEntityData::GetDirtyBits() const
    {
        return mDirtyBits;
    }

    void SimEntityData::SetAllDirtyBits()
    {
        mDirtyBits = uint32_t(-1); // all ones
        mPrevious = mCurrent;
    }

    void SimEntityData::SetDirtyBits(uint32_t bits)
    {
        mDirtyBits |= bits;
    }
    
    bool SimEntityData::IsDirty(SimEntityData::DataBits bits) const
    {
        return (mDirtyBits & bits) != 0;
    }
    
    void SimEntityData::ProcessTick(float32_t dt)
    {
        mPrevious = mCurrent;
    }
    
    /// Get the previous state of this object
    const SimEntityData::SimEntityInternals& SimEntityData::GetPrevious() const
    {
        return mPrevious;
    }
    
    bool SimEntityData::operator== (SimEntityData const& x)
    {
        return GetId() == x.GetId();
    }    

    bool SimEntityData::operator!= (SimEntityData const& x)
    {
        return GetId() != x.GetId();
    }    

	/// output sim entity data to stream
	std::ostream& operator<<(std::ostream& stream, const SimEntityData& data)
	{
		stream << "{id: " << data.GetId()
			<< ", position: " << data.GetPosition() 
			<< ", rotation: " << data.GetRotation()
			<< ", velocity: " << data.GetVelocity()
			<< ", scale: " << data.GetScale() << "}";
		return stream;
	}

} //end OpenNero
