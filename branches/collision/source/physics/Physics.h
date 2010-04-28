#ifndef _PHYSICS_PHYSICS_H_
#define _PHYSICS_PHYSICS_H_

#include "core/Common.h"

#if NERO_BUILD_PHYSICS

#include <set>
#include "core/ONTypes.h"
#include "physics/PhysicsObject.h"

namespace OpenNero
{
	/// @cond
    BOOST_SHARED_STRUCT(IPhysicsEngine);
    BOOST_SHARED_STRUCT(IPhysicsObject);
    BOOST_SHARED_DECL(SceneObject);
    BOOST_SHARED_DECL(ObjectTemplate);
    /// @endcond

    /// Physics engine interface
    struct IPhysicsEngine
    {
        public:
        	virtual ~IPhysicsEngine() {}

            /// get the physics engine instance
            static IPhysicsEngine& instance();

            /// initialize the physics engine
            virtual void init() = 0;

            /// step the physical simulation
            /// @param dt time since last step
            virtual void step(F32 dt) = 0;

            /// destroy the physical simulation
            virtual void destroy() = 0;

			/// Create an IPhysicsObject for this engine
			/// @param ent SimEntity which owns this object
			virtual IPhysicsObjectPtr createObject(SimEntityPtr ent) = 0;

            /// add a physical object to the simulation
            /// @param physicsObject an initialized physics object to add to the world
            virtual void addObject(IPhysicsObjectPtr physicsObject) = 0;

            /// remove a physical object from the simulation
            /// @param physicsObject a physics object to remove
            virtual void removeObject(IPhysicsObjectPtr physicsObject) = 0;

            /// Return whether or not the physics engine is enabled
            /// @return true iff the engine is enabled
            virtual bool IsEnabled() const = 0;

            /// Enable or disable the physics system
            virtual void SetEnabled(bool enabled) = 0;

            // ------------------------------------------
            // functions for sensors

            /// Find all the objects that fall within a sphere
            /// @param origin the origin of the sphere
            /// @param radius the radius of the sphere
            /// @return a set of physics objects that intersect the solid of interest
            virtual SimDataVector findInSphere(const Vector3f& origin, F32 radius) = 0;

    };

} // namespace OpenNero

#endif // NERO_BUILD_PHYSICS

#endif
