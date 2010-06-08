#ifndef PHYSICSOBJECTIMPL_H_
#define PHYSICSOBJECTIMPL_H_

#include "core/Common.h"

#if NERO_BUILD_PHYSICS

#include <ode/ode.h>
#include <irrlicht.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "game/SimEntity.h"
#include "game/objects/TemplatedObject.h"
#include "game/objects/SimEntityComponent.h"
#include "PhysicsObject.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL(SceneObject);
    class SimEntityData;
    /// @endcond

    /// an ODE-based implementation of the IPhysicsObject interface
    class ODEPhysicsObject : public IPhysicsObject
    {
    public:
        /// create a new ODEPhysicsObject
        ODEPhysicsObject(SimEntityPtr ent);

        ~ODEPhysicsObject();

        void ProcessTick( float32_t dt );

        bool LoadFromTemplate(ObjectTemplatePtr objTemplate, const SimEntityData& data);

        void UpdateAfterPhysics();
        
        void UpdateBeforePhysics();
        
        std::ostream& stream(std::ostream& out) const;

        /// should we draw the aabb
        void DrawAABB( bool draw )
        {
            _drawAABB = draw;
        }

    private:

        // render the aabb to the screen
        void RenderAABB() const;

    private:
        dBodyID _id; ///< the id of the body
        dGeomID _geom; ///< the id of the associated geometry
        dReal _mass; ///< mass of the object
        dJointID _floor; ///< optional 2-D plain constraint

        bool _drawAABB; ///< flag to draw the aabb

        /// construct the geometry data based on the specified SimEntity's bounding box
        void setGeomDataFromBB(dSpaceID space_id, const SceneObjectPtr& obj);

		/// initialize the physics object from the mesh of the SceneObject
		void InitFromMesh(SceneObjectPtr obj, const SimEntityData& data);

		/// initialize the physics object from the bounding box of the SceneObject
		void InitFromBox(SceneObjectPtr obj, const SimEntityData& data);

        /// snap to Z=0 plane
        void SnapToPlane();
    };

    /// shared pointer to an ODEPhysicsObject
    typedef boost::shared_ptr<ODEPhysicsObject> ODEPhysicsObjectPtr;
    /// weak pointer to an ODEPhysicalObject
    typedef boost::weak_ptr<ODEPhysicsObject> ODEPhysicsObjectWPtr;
}

#endif // NERO_BUILD_PHYSICS

#endif /*PHYSICSOBJECTIMPL_H_*/
