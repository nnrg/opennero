//--------------------------------------------------------
// OpenNero : SimEntityData
//  data shared across components in a sim entity
//--------------------------------------------------------

#ifndef _GAME_SIMENTITY_DATA_H_
#define _GAME_SIMENTITY_DATA_H_

#include "core/IrrUtil.h"
#include "core/ONTypes.h"
#include "core/Preprocessor.h"
#include "core/Bitstream.h"

#include <iosfwd>

namespace OpenNero 
{
    /// @cond
    BOOST_PTR_DECL(SimEntityData);
    /// @endcond

    /// SimEntityData stores mutable data 
    class SimEntityData
    {
    public:
    
        struct SimEntityInternals {
            Vector3f mPosition;     ///< Position of the object
            Vector3f mRotation;     ///< Euler rotation angles of the object
            Vector3f mVelocity;     ///< Linear velocity of the object
            Vector3f mScale;        ///< Scale of the object (in x,y,z)
            Vector3f mAcceleration; ///< Linear acceleration of the object
            std::string mLabel;     ///< Text label for the object
            SColor mColor;          ///< Color of the object
            uint32_t mType;         ///< Type of this object (for sensors)
            uint32_t mCollision;    ///< The collision mask
            
            SimEntityInternals();
            SimEntityInternals(
                const Vector3f& pos, 
                const Vector3f& rot, 
                const Vector3f& scale, 
                const std::string& label,
                uint32_t t,
                uint32_t collision);
            
        };

        /// Declaration of how bits in a binary string correspond to data
        enum DataBits
        {
            kDB_Position        = (1<<0),
            kDB_Rotation        = (1<<1),
            kDB_Velocity        = (1<<2),
            kDB_Scale           = (1<<4),
            kDB_Acceleration    = (1<<5),
            kDB_Label           = (1<<6),
            kDB_Color           = (1<<7),
            kDB_Id              = (1<<8),
            kDB_Type            = (1<<9),
            kDB_Collision       = (1<<10),
        };
        
    public:
    
        /// Default constructor
        SimEntityData();

        /// Constructor for SimEntityData given initial values
        SimEntityData(const Vector3f& pos, 
                      const Vector3f& rot, 
                      const Vector3f& scale, 
                      const std::string& label,
                      uint32_t t,
                      uint32_t collision,
                      SimId id);

        void SetPosition( const Vector3f& pos );      ///< Set position of entity
        void SetRotation( const Vector3f& rot );      ///< Set rotation of entity
        void SetVelocity( const Vector3f& vel );      ///< Set velocity of entity
        void SetScale( const Vector3f& scale );       ///< Set scale of entity
        void SetAcceleration( const Vector3f& acc );  ///< Set acceleration for entity
        void SetLabel( const std::string& label );    ///< Set label for entity
        void SetColor( const SColor& color );         ///< Set the color of the entity
        void SetType( uint32_t t );                   ///< Type of this object (for sensors)
        void SetCollision( uint32_t mask );           ///< Set the collision mask for this object
        void SetAnimation( const std::string& animationType ); ///< Set the animation type of the object
        void SetAnimationSpeed( float32_t framesPerSectiond ); ///< Set the speed of the animation

        /// make all the bits dirty (forces full update)
        void SetAllDirtyBits();
        /// make certain bits dirty
        void SetDirtyBits(uint32_t bits);
        /// clear all the bits
        void ClearDirtyBits();
        
        const Vector3f& GetPosition() const;      ///< Returns position of entity
        const Vector3f& GetRotation() const;      ///< Returns rotation of entity
        const Vector3f& GetVelocity() const;      ///< Returns velocity of entity
        const Vector3f& GetScale() const;         ///< Returns scale of entity
        const Vector3f& GetAcceleration() const;  ///< Returns acceleration of entity
        const std::string& GetLabel() const;      ///< Returns label of entity
        const SColor& GetColor() const;           ///< Returns the color of the entity
        SimId GetId() const;                      ///< Returns the id of the entity
        uint32_t GetType() const;                 ///< Type of this object (for sensors)
        uint32_t GetCollision( ) const;           ///< Get the collision mask
        std::string GetAnimation() const;         ///< Returns the animation type
        float32_t GetAnimationSpeed() const;      ///< Returns the animation speed (frames per second)
        
        const SimEntityInternals& GetPrevious() const; ///< Get the previous state of this object
        void ProcessTick(float32_t dt); ///< Assign the current state to previous state

        uint32_t GetDirtyBits() const;            ///< Retrieve the dirty bits
        bool IsDirty(DataBits bit) const;         ///< Flag to say if SimEntity is dirty
        bool operator== ( SimEntityData const& x );
        bool operator!= ( SimEntityData const& x );

    private:
        /// The id of the object
        SimId mId;

        /// The bitmask of the object (which data has changed)
        uint32_t mDirtyBits;
    
        /// Previous state
        SimEntityInternals mPrevious;

        /// Current state
        SimEntityInternals mCurrent;
        
    };

    /// vector of SimEntityData objects
    typedef std::vector<SimEntityData> SimDataVector;

	/// output SimEntityData to stream
	std::ostream& operator<<(std::ostream& stream, const SimEntityData& data);

} //end OpenNero

#endif // _GAME_SIMENTITY_DATA_H_
