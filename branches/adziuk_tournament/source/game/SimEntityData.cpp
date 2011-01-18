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
    SimEntityData::SimEntityData()
        : mPosition()
        , mRotation()
        , mVelocity()
        , mScale(1,1,1)
        , mAcceleration()
        , mLabel()
        , mColor(0xFF, 0xFF, 0xFF, 0xFF)
        , mId()
        , mType()
        , mCollision()
        , mDirtyBits(uint32_t(-1))
    {
    }

    SimEntityData::SimEntityData(const Vector3f& pos, 
                                 const Vector3f& rot, 
                                 const Vector3f& scale, 
                                 const std::string& label,
                                 uint32_t t,
                                 uint32_t collision,
                                 SimId id)
        : mPosition(pos)
        , mRotation(rot)
        , mVelocity(0,0,0)
        , mScale(scale)
        , mAcceleration(0,0,0)
        , mLabel(label)
        , mColor(0xFF, 0xFF, 0xFF, 0xFF)
        , mId(id)
        , mType(t)
        , mCollision(collision)
        , mDirtyBits(uint32_t(-1))
    {
    }

    void SimEntityData::SetPosition( const Vector3f& pos )
    {
        if( pos != mPosition )
        {
            mPosition = pos;
            mDirtyBits |= kDB_Position;
        }
    }

    void SimEntityData::SetRotation( const Vector3f& rot )
    {
        if( rot != mRotation )
        {
            mRotation = rot;
            mDirtyBits |= kDB_Rotation;
        }
    }

    void SimEntityData::SetVelocity( const Vector3f& vel )
    {
        if( vel != mVelocity )
        {
            mVelocity = vel;
            mDirtyBits |= kDB_Velocity;
        }
    }

    void SimEntityData::SetAcceleration(const Vector3f& acc)
    {
        if (acc != mAcceleration )
        {
            mAcceleration = acc;
            mDirtyBits |= kDB_Acceleration;
        }
    }
    
    void SimEntityData::SetLabel(const std::string& label)
    {
        if (label != mLabel)
        {
            mLabel = label;
            mDirtyBits |= kDB_Label;
        }
    }

    void SimEntityData::SetColor(const SColor& color)
    {
        if (color != mColor)
        {
            mColor = color;
            mDirtyBits |= kDB_Color;
        }
    }

    void SimEntityData::SetScale( const Vector3f& scale )
    {
        if (scale != mScale )
        {
            mScale = scale;
            mDirtyBits |= kDB_Scale;
        }
    }
    
    void SimEntityData::SetType( uint32_t t )
    {
        if ( mType != t )
        {
            mType = t;
            mDirtyBits |= kDB_Type;
        }
    }

    void SimEntityData::SetCollision( uint32_t mask )
    {
        if ( mCollision != mask )
        {
            mCollision = mask;
            mDirtyBits |= kDB_Collision;
        }
    }

    void SimEntityData::ClearDirtyBits()
    {
        mDirtyBits = 0;
    }
        
    const Vector3f& SimEntityData::GetPosition() const
    {
        return mPosition;
    }

    const Vector3f& SimEntityData::GetRotation() const
    {
        return mRotation;
    }

    const Vector3f& SimEntityData::GetVelocity() const
    {
        return mVelocity;
    }

    const Vector3f& SimEntityData::GetAcceleration() const
    {
        return mAcceleration;
    }

    const Vector3f& SimEntityData::GetScale() const
    {
        return mScale;
    }
    
    const std::string& SimEntityData::GetLabel() const
    {
        return mLabel;
    }

    const SColor& SimEntityData::GetColor() const
    {
        return mColor;
    }

    SimId SimEntityData::GetId() const
    {
        return mId;
    }
    
    uint32_t SimEntityData::GetType() const
    {
        return mType;
    }

    uint32_t SimEntityData::GetCollision() const
    {
        return mCollision;
    }

    uint32_t SimEntityData::GetDirtyBits() const
    {
        return mDirtyBits;
    }

    void SimEntityData::SetDirtyBits()
    {
        mDirtyBits = uint32_t(-1); // all ones
    }

    void SimEntityData::SetDirtyBits(uint32_t bits)
    {
        mDirtyBits |= bits;
    }
    
    bool SimEntityData::IsDirty(SimEntityData::DataBits bits) const
    {
        return (mDirtyBits & bits) != 0;
    }

    /// output a SimEntityData object to the Bitstream for packet transmission
    Bitstream& operator<<( Bitstream& stream, const SimEntityData& data)
    {
        // Note: The dirty bits can technically be 0 if the initialized
        // values are the default. This is such an edge case that it is probably
        // not worth putting in special logic for to prevent this unecessary network transmit

        // the order needs to be in synch with operator>>
        stream << data.GetDirtyBits();
        if (data.mDirtyBits & SimEntityData::kDB_Position)
        {
            stream << data.mPosition;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Rotation)
        {
            stream << data.mRotation;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Velocity)
        {
            stream << data.mVelocity;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Scale)
        {
            stream << data.mScale;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Acceleration)
        {
            stream << data.mAcceleration;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Label)
        {
            stream << data.mLabel;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Color)
        {
            stream << data.mColor;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Type)
        {
            stream << data.mType;
        }
        if (data.mDirtyBits & SimEntityData::kDB_Collision)
        {
            stream << data.mCollision;
        }
        return stream;
    }

    /// Update a SimEntityData object from tthe Bitstream.
    /// First, the dirty bits are read from the stream. The rest of the data is
    /// read only if the appropriate dirty bits are set.
    Bitstream& operator>>( Bitstream& stream, SimEntityData& data)
    {
        // Note: The dirty bits can technically be 0 if the initialized
        // values are the default. This is such an edge case that it is probably
        // not worth putting in special logic for to prevent this unecessary network transmit

        // the order needs to be in sync with operator<<
        uint32_t dirty_bits;
        stream >> dirty_bits;
        data.mDirtyBits = data.mDirtyBits | dirty_bits; // whatever was dirty before should remain dirty
        if (dirty_bits & SimEntityData::kDB_Position)
        {
            stream >> data.mPosition;
        }
        if (dirty_bits & SimEntityData::kDB_Rotation)
        {
            stream >> data.mRotation;
        }
        if (dirty_bits & SimEntityData::kDB_Velocity)
        {
            stream >> data.mVelocity;
        }
        if (dirty_bits & SimEntityData::kDB_Scale)
        {
            stream >> data.mScale;
        }
        if (dirty_bits & SimEntityData::kDB_Acceleration)
        {
            stream >> data.mAcceleration;
        }
        if (dirty_bits & SimEntityData::kDB_Label)
        {
            stream >> data.mLabel;
        }
        if (dirty_bits & SimEntityData::kDB_Color)
        {
            stream >> data.mColor;
        }
        if (dirty_bits & SimEntityData::kDB_Type)
        {
            stream >> data.mType;
        }
        if (dirty_bits & SimEntityData::kDB_Collision)
        {
            stream >> data.mCollision;
        }
        return stream;
    }
    
    bool SimEntityData::operator== (SimEntityData const& x)
    {
        return GetId() == x.GetId();
    }    

    bool SimEntityData::operator!= (SimEntityData const& x)
    {
        return GetId() != x.GetId();
    }    

	bool SimEntityData::SetAnimation( const std::string& animation )
	{
		return Kernel::instance().GetSimContext()->SetObjectAnimation(mId, animation);
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
