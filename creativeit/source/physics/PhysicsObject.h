#ifndef _PHYSICSOBJECT_H_
#define _PHYSICSOBJECT_H_

#include "core/Common.h"

#if NERO_BUILD_PHYSICS

#include "game/objects/SimEntityComponent.h"
#include "game/objects/TemplatedObject.h"

namespace OpenNero
{
	/// @cond
    BOOST_SHARED_STRUCT(IPhysicsObject);
    BOOST_SHARED_DECL(SimEntityData);
    BOOST_SHARED_DECL(ObjectTemplate);
    /// @endcond

    /// An object representing all the physical aspects of a simulation entity
    struct IPhysicsObject : public enable_shared_from_this<IPhysicsObject>, public SimEntityComponent, public TemplatedObject
    {
        public:
            /// constructor
            IPhysicsObject(SimEntityPtr ent) : SimEntityComponent(ent) {}

            /// virtual destructor
            virtual ~IPhysicsObject() {}

            /// process the frame (frame time in seconds)
            virtual void ProcessTick(float32_t dt) = 0;

            /// update all objects before the physics simulation step
            virtual void UpdateBeforePhysics() = 0;

            /// update all objects after the physics simulation step
            virtual void UpdateAfterPhysics() = 0;

            /// print object summary to stream
            virtual std::ostream& stream(std::ostream& out) const = 0;

            /// check whether the bounding box should be drawn for this object
            virtual void DrawAABB( bool draw ) = 0;

            friend std::ostream& operator<<(std::ostream& os, IPhysicsObjectPtr body);
    };

    /// print a physics object to stream
    std::ostream& operator<<(std::ostream& os, IPhysicsObjectPtr body);

    typedef std::set<IPhysicsObjectPtr> PhysicsObjectSet; ///< set of physics objects

}

#endif // NERO_BUILD_PHYSICS

#endif /*_PHYSICSOBJECT_H*/
