#include "core/Common.h"

#if NERO_BUILD_PHYSICS

#include <string>
#include "physics/PhysicsObject.h"
#include "physics/PhysicsObjectImpl.h"
#include "physics/Physics.h"
#include "physics/PhysicsImpl.h"
#include "core/IrrUtil.h"
#include "game/Kernel.h"
#include "game/SimEntity.h"
#include "utils/Performance.h"
#include "utils/Config.h"
#include "render/SceneObject.h"
#include "render/LineSet.h"

namespace OpenNero
{
    using namespace Physics;
    using namespace irr;
    using namespace irr::core;
    
    ODEPhysicsObject::ODEPhysicsObject(SimEntityPtr ent) :
        IPhysicsObject(ent), _id(0), _geom(0), 
        _mass(GetPhysicsConfig().default_mass), 
        _floor(0),
        _drawAABB(NERO_DEBUG)
    {
    }

    bool ODEPhysicsObject::LoadFromTemplate(ObjectTemplatePtr objTemplate, const SimEntityData& data)
    {
        // we only hold on to this scene object while we are initializing
        // TODO: make ODEPhysicsObject loading more efficient
        SceneObjectPtr shape(new SceneObject());
        if (shape->LoadFromTemplate(objTemplate, data))
        {
			if (shape->hasMesh())
				InitFromMesh(shape, data);
			else
				InitFromBox(shape, data);
            ODEPhysicsEngine::instance().addBody(data.GetId(), _id);
            ODEPhysicsEngine::instance().addGeom(data.GetId(), _geom);
            return true;
        }
        else
        {
            return false;
        }
    }

    ODEPhysicsObject::~ODEPhysicsObject()
    {
        if (_id)
        {
            dBodyDestroy(_id);
        }
        if (_geom)
        {
            dGeomDestroy(_geom);
        }
        if (_floor)
        {
            dJointDestroy(_floor);
        }
    }

    void ODEPhysicsObject::ProcessTick(float32_t dt)
    {
        // draw the bounding box of the geometry if needed
        if( _geom && _drawAABB )
            RenderAABB();
    }

    /// Synchronize object properties from Simulation to Physics.
    /// This is where changes to position or velocity requested by AI 
    /// get reconciled with the physical world.
    void ODEPhysicsObject::UpdateBeforePhysics()
    {
        const Vector3f pos(GetSharedData()->GetPosition()); // r.h. position
        const Vector3f vel(GetSharedData()->GetVelocity()); // r.h. velocity
        const Vector3f rot(GetSharedData()->GetRotation()); // r.h. Euler angle in degrees
        const Vector3f acc(GetSharedData()->GetAcceleration()); // r.h. linear acceleration
        // set position of the body and the geometry
        dBodySetPosition(_id, pos.X, pos.Y, pos.Z);
        dGeomSetPosition(_geom, pos.X, pos.Y, pos.Z);
        // add a force from the acceleration
        dBodyAddForce(_id, acc.X, acc.Y, acc.Z);

        // translate the rotation to ODE system
        dQuaternion q;
        EulerToODEQuaternion(rot, q);
        // set rotation for body and geometry
        dBodySetQuaternion(_id, q);
        dGeomSetQuaternion(_geom, q);

        if (_floor)
        {
            dJointSetPlane2DAngleParam(_floor,dParamVel,1);
            dJointSetPlane2DXParam(_floor,dParamVel,1);
            dJointSetPlane2DYParam(_floor,dParamVel,1);
        }

        // add damping force
        Vector3f dv( - vel * GetPhysicsConfig().damping );
        dBodyAddForce(_id, dv.X, dv.Y, dv.Z);
        // add a force from the acceleration
        dBodyAddForce(_id, acc.X, acc.Y, acc.Z);
    }

    /// Synchronize object properties from Physics to Simulation.
    /// This is where the physical simulation 
    void ODEPhysicsObject::UpdateAfterPhysics()
    {
        Assert(_geom != 0);
        if (_floor) SnapToPlane();
        // export the new position, rotation and velocity to other components
        const dReal *ode_pos = dBodyGetPosition(_id);
        const dReal *ode_quat = dBodyGetQuaternion(_id);
        const dReal *ode_vel = (dReal*)dBodyGetLinearVel(_id);
        GetSharedData()->SetPosition(Vector3f((F32)ode_pos[0], (F32)ode_pos[1], (F32)ode_pos[2]));
        Vector3f euler;
        ODEQuaternionToEuler(ode_quat, euler);
        GetSharedData()->SetRotation(euler);
        GetSharedData()->SetVelocity(Vector3f((F32)ode_vel[0], (F32)ode_vel[1], (F32)ode_vel[2]));
    }

    void ODEPhysicsObject::SnapToPlane()
    {
        const dReal *rot = dBodyGetAngularVel(_id);
        const dReal *quat_ptr;
        dReal quat[4], quat_len;
        quat_ptr = dBodyGetQuaternion(_id);
        quat[0] = quat_ptr[0];
        quat[1] = 0;
        quat[2] = 0; 
        quat[3] = quat_ptr[3]; 
        quat_len = sqrt(quat[0] * quat[0] + quat[3] * quat[3]);
        quat[0] /= quat_len;
        quat[3] /= quat_len;
        dBodySetQuaternion (_id, quat);
        dBodySetAngularVel (_id, 0, 0, rot[2]);
    }

	/// Initialize this physics object from the 3D mesh in the SceneObject
	/// See http://opende.sourceforge.net/wiki/index.php/Manual_(Collision_Detection)#Triangle_Mesh_Class
	/// for documentation about the TriMesh objects in ODE.
	/// @param obj scene object to create the mesh from
	void ODEPhysicsObject::InitFromMesh( SceneObjectPtr obj, const SimEntityData& simData )
	{
		// get the handles of the world
		dSpaceID space_id = ODEPhysicsEngine::const_instance().getSpaceID();

		// get the properties of the object
		aabbox3df box = obj->getBoundingBox();
		Vector3f extent = ConvertIrrlichtToNeroPosition(box.getExtent());
		Vector3f scale = simData.GetScale();
		Vector3f position = simData.GetPosition();
		Vector3f rotation = simData.GetRotation();
		Vector3f dimensions(extent.X * scale.X, extent.Y * scale.Y, extent.Z * scale.Z);

		// count vertices and indices
		CDynamicMeshBuffer mb(irr::video::EVT_STANDARD, irr::video::EIT_16BIT);
		Assert(obj->getMeshBuffer(mb, 2));
		int indexcount = mb.getIndexCount();
		int vertexcount = mb.getVertexCount();
		// build structure for ode trimesh _geom
		int VertexStride = 4;
		dReal* vertices = new dReal[vertexcount * VertexStride];
		memset(vertices, 0, sizeof(dReal) * vertexcount * VertexStride);
		dTriIndex* indices = new dTriIndex[indexcount];
		// fill trimesh _geom
		int ci=0; // current index in the indices array
		int cif=0; // offset of the irrlicht-vertex-index in the vetices array
		int cv=0; // current index in the vertices array
		// fill indices
		U16* mb_indices = mb.getIndices();
		for (U32 j = 0; j < mb.getIndexCount(); ++j)
		{
			// scale the indices from multiple meshbuffers to single index array
			indices[ci] = cif+mb_indices[j];
			ci++;
		}
		// update the offset for the next meshbuffer
		cif = cif + mb.getVertexCount();
		// fill vertices
		if (mb.getVertexType()==irr::video::EVT_STANDARD)
		{
			irr::video::S3DVertex
				* mb_vertices =(irr::video::S3DVertex*)mb.getVertices();
			for (U32 j = 0; j < mb.getVertexCount(); ++j)
			{
				// switch y and z to transform from a left-handed Irrlicht system
				vertices[cv * VertexStride + 0] = mb_vertices[j].Pos.X * scale.X;
				vertices[cv * VertexStride + 2] = mb_vertices[j].Pos.Y * scale.Y;
				vertices[cv * VertexStride + 1] = mb_vertices[j].Pos.Z * scale.Z;
				cv++;
			}
		}
		else if (mb.getVertexType()==irr::video::EVT_2TCOORDS)
		{
			irr::video::S3DVertex2TCoords
				* mb_vertices=(irr::video::S3DVertex2TCoords*)mb.getVertices();
			for (U32 j = 0; j < mb.getVertexCount(); ++j)
			{
				// switch y and z to transform from a left-handed Irrlicht system
				vertices[cv * VertexStride + 0] = mb_vertices[j].Pos.X * scale.X;
				vertices[cv * VertexStride + 2] = mb_vertices[j].Pos.Y * scale.Y;
				vertices[cv * VertexStride + 1] = mb_vertices[j].Pos.Z * scale.Z;
				cv++;
			}
		}

		// build the trimesh data
		dTriMeshDataID data = dGeomTriMeshDataCreate();
		// no data is copied here, so the pointers passed into the function must remain valid (do not delete them)
		dGeomTriMeshDataBuildSimple(
			data, /* data id */
			(dReal*)vertices, vertexcount,
			indices, indexcount);

		// build the trimesh geom
		_geom = dCreateTriMesh(space_id, data, 0, 0, 0);        

		// bind the physics representation with this PhysicsObject
		dGeomSetData(_geom, (void*)this);

		// set the position and orientation of the geometry and the body
		dGeomSetPosition(_geom, position.X, position.Y, position.Z);
		dQuaternion q;
		EulerToODEQuaternion(rotation, q);
		dGeomSetQuaternion(_geom, q);
	}

	void ODEPhysicsObject::InitFromBox( SceneObjectPtr obj, const SimEntityData& data )
	{
		dWorldID world_id = ODEPhysicsEngine::const_instance().getWorldID();
		dSpaceID space_id = ODEPhysicsEngine::const_instance().getSpaceID();

		// get the properties of the object
		aabbox3df box = obj->getBoundingBox();
		Vector3f extent = ConvertIrrlichtToNeroPosition(box.getExtent());
		Vector3f scale = data.GetScale();
		Vector3f position = data.GetPosition();
		Vector3f rotation = data.GetRotation();
		Vector3f dimensions(extent.X * scale.X, extent.Y * scale.Y, extent.Z * scale.Z);

		// build a box shaped geometry for ODE
		_geom = dCreateBox(space_id, dimensions.X, dimensions.Y, dimensions.Z);

		// create body
		_id = dBodyCreate(world_id);

		// setup the mass
		dMass mass;
		dMassSetBoxTotal(&mass, _mass, dimensions.X, dimensions.Y, dimensions.Z);
		dBodySetMass(_id, &mass);
		dGeomSetBody(_geom, _id);

		// bind the physics representation with this PhysicsObject
		dBodySetData(_id, (void*)this);
		dGeomSetData(_geom, (void*)this);

		// set the position and orientation of the geometry and the body
		dGeomSetPosition(_geom, position.X, position.Y, position.Z);
		dQuaternion q;
		EulerToODEQuaternion(rotation, q);
		dGeomSetQuaternion(_geom, q);
	}

    std::ostream& ODEPhysicsObject::stream(std::ostream& out) const
    {
        out << "<body id=\""<< _id<< "\" geom=\""<< _geom<< "\" />";
        return out;
    }

    /// Draw the axis aligned bounding box for the physics object
    void ODEPhysicsObject::RenderAABB() const
    {
        dReal d_aabb[6];
        float32_t aabb[6];

        enum
        {
            kMinX,
            kMaxX,
            kMinY,
            kMaxY,
            kMinZ,
            kMaxZ
        };

        // (minx, maxx, miny, maxy, minz, maxz)
        dGeomGetAABB( _geom, d_aabb );

        aabb[0] = (float32_t)d_aabb[0];
        aabb[1] = (float32_t)d_aabb[1];
        aabb[2] = (float32_t)d_aabb[2];
        aabb[3] = (float32_t)d_aabb[3];
        aabb[4] = (float32_t)d_aabb[4];
        aabb[5] = (float32_t)d_aabb[5];

        const LineSet::LineColor kBlue( 255, 0, 0, 255 );

        LineSet& ls = LineSet::instance();
        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMinZ] ), Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMinZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMinZ] ), Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMinZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMinZ] ), Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMinZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMinZ] ), Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMinZ] ), kBlue );

        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMaxZ] ), Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMaxZ] ), Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMaxZ] ), Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMaxZ] ), Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMaxZ] ), kBlue );

        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMinZ] ), Vector3f( aabb[kMinX], aabb[kMinY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMinZ] ), Vector3f( aabb[kMinX], aabb[kMaxY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMinZ] ), Vector3f( aabb[kMaxX], aabb[kMinY], aabb[kMaxZ] ), kBlue );
        ls.AddSegment( Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMinZ] ), Vector3f( aabb[kMaxX], aabb[kMaxY], aabb[kMaxZ] ), kBlue );
    }

    std::ostream& operator<<( std::ostream& os, IPhysicsObjectPtr body )
    {
        os << "PhysicsObject for " << body->GetEntity();
        return os;
    }
}

#endif // NERO_BUILD_PHYSICS
