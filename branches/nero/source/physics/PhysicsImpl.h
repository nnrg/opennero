#ifndef PHYSICSIMPL_H_
#define PHYSICSIMPL_H_

#inlclude "core/Common.h"

#if NERO_BUILD_PHYSICS

#include <set>
#include <ode/ode.h>
#include <ostream>
#include <vector>
#include "core/IrrUtil.h"
#include "core/HashMap.h"
#include "game/Kernel.h"

#include "Physics.h"

namespace OpenNero
{
    namespace Physics
    {
        /// returns the config for the current physics engine
        inline const PhysicsConfig& GetPhysicsConfig() { return GetAppConfig().physics_config; }
        
        /// convert a rotation quaternion to an Euler rotation vector
        extern void ODEQuaternionToEuler(const dQuaternion quaternion, Vector3f &euler);

        /// convert an Euler rotation vector to a quaternion
        extern void EulerToODEQuaternion(const Vector3f& euler, dQuaternion dq);

    } // namespace Physics

    struct PhysicsConfig;

    /// An ODE-based physics engine that simulates the physics (dynamics and collisions) in our world
    class ODEPhysicsEngine : public IPhysicsEngine
    {
    		friend class ODEPhysicsObject;
        public:

        	// -------------- singleton implementation ------------------------------
        	
        	/// @return singleton engine reference
            static ODEPhysicsEngine& instance();

            /// @return constant singleton reference
            static const ODEPhysicsEngine& const_instance();
            
            /// @return true iff the engine is enabled
            bool IsEnabled() const { return _enabled; }

            /// Enable or disable the physics system
            void SetEnabled(bool enabled);

            ODEPhysicsEngine();
            
            // -------------- physics stepping functions -----------------------------
            
            /// initialize the physics engine
            void init();

            /// advance the physics time by dt seconds
            void step(F32 dt);

            /// destroy the physics engine and all of its objects
            void destroy();

			/// Create an IPhysicsObject for this engine
			/// @param ent SimEntity which owns this object
			IPhysicsObjectPtr createObject(SimEntityPtr ent);

			/// add a physical object to the simulation
			/// @param physicsObject an initialized physics object to add to the world
            void addObject(IPhysicsObjectPtr physObj);

            /// remove a physics object from the simulation
            void removeObject(IPhysicsObjectPtr physicsObj);
            
            /// Find all the objects that fall within a sphere
            SimDataVector findInSphere(const Vector3f& origin, F32 radius);

        private:

            dWorldID _world; ///< Physics simulation world (ODE id)
            dSpaceID _space; ///< The collision space to use
            dJointGroupID _contactgroup; ///< a group of ephemeral joints responsible for making things bounce, slide, etc.
            PhysicsObjectSet _bodies; ///< bodies associated with simulation nodes
            bool _enabled; ///< whether the engine is enabled
            dGeomID _ground; ///< the id of the "floor" plane

            typedef hash_map<SimId, dGeomID> SimToGeomMap; ///< a map from simulation entities to their physics geometries
            typedef hash_map<dGeomID, SimId> GeomToSimMap; ///< a map from physics geometries to their SimId
            typedef hash_map<SimId, dBodyID> SimToBodyMap; ///< a map from simulation entities to their physics bodies
            typedef hash_map<dBodyID, SimId> BodyToSimMap; ///< a map from physics bodies to their SimId

            SimToGeomMap _geoms_by_id; ///< a map from simulation entities to their physics geometries
            GeomToSimMap _ids_by_geom; ///< a map from physics geometries to their SimId
            SimToBodyMap _bodies_by_id; ///< a map from simulation entities to their physics bodies
            BodyToSimMap _ids_by_body; ///< a map from physics bodies to their SimId
            
            SimDataVector _found_objects; ///< the current set of objects for findInXXX methods

            /// @brief this method gets called when two geometries collide
            static void collisionCallback(void* data, dGeomID o1, dGeomID o2);

            /// this function is called when searching for sphere collisions
            static void findInSphereCallback(void* data, dGeomID o1, dGeomID o2);
            
            /// add the SimEntityData object to the current found set
            void found(const SimEntityData& data);

            /// lookup and add the SimEntityData object to the current found set
            void found(dGeomID geom);

            /// register a geometry with the engine
            void addGeom(SimId id, dGeomID geom);

            /// register a body with the engine
            void addBody(SimId id, dBodyID body);
            
            /// collide two geometries by creating temporary contact joints between them
            void collide(dGeomID o1, dGeomID o2);

            /// update the objects before the physics simulation step
            void updateBeforePhysics();

            /// update the objects after the physics simulation step
            void updateAfterPhysics();

            /// get the root physics world ID
            dWorldID getWorldID() const { return _world; }

            /// get the root physics space ID
            dSpaceID getSpaceID() const { return _space; }

    };

} // namespace OpenNero

#endif // NERO_BUILD_PHYSICS

#endif /*PHYSICSIMPL_H_*/
