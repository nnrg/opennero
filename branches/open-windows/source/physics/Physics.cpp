#include "core/Common.h"

#if NERO_BUILD_PHYSICS

#include <set>
#include <cassert>
#include <ostream>
#include <boost/pool/detail/singleton.hpp>
#include "scripting/scriptIncludes.h"
#include "core/Log.h"
#include "core/Error.h"
#include "game/SimEntity.h"
#include "game/SimContext.h"
#include "game/factories/SimFactory.h"
#include "render/SceneObject.h"
#include "PhysicsImpl.h"
#include "PhysicsObjectImpl.h"
#include "utils/Performance.h"
#include "utils/Config.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

namespace OpenNero
{
    namespace Physics
    {
        const U32 kMaxNrOfSteps = 100;
        const U32 kNrOfContacts = 8;
        const U32 kLevelOfDetail = 2;
        const dReal kTrackingRadius = 1000;
        const dReal kGravity = 9.81;
        const dReal kWorldCFM = 1e-5;
        const dReal kWorldERP = 0.8;
        const dReal kDamping = 0.01;
        const dReal kDampingTorque = 0.01;
        const dReal kStepSize = 0.005;
        // TODO: this should come from the ODE headers, duh
        const char* const kEngine = "ODE 0.10"; ///< version of the physics engine we are using
        const dReal kRobotMass = 10.0;
        std::ostream& operator<<(std::ostream& os, const dQuaternion& qt)
        {
            return os << "( "<< qt[0]<< ", "<< qt[1]<< ", "<< qt[2]<< ", " << qt[3]<< " )";
        }
        std::ostream& operator<<(std::ostream& os, const dSurfaceParameters& surface)
        {
            os << "<surface "
               << "mu='" << surface.mu << "' "
               << "mu2='" << surface.mu2 << "' "
               << "bounce='" << surface.bounce << "' "
               << "bounce_vel='" << surface.bounce_vel << "' "
               << "soft_erp='" << surface.soft_erp << "' "
               << "soft_cfm='" << surface.soft_cfm << "' "
               << "motion1='" << surface.motion1 << "' "
               << "motion2='" << surface.motion2 << "' "
               << "slip1='" << surface.slip1 << "' "
               << "slip2='" << surface.slip2 << "' "
               << "/>";
            return os;
        }
        std::ostream& operator<<(std::ostream& os, const dGeomID& geomID)
        {
            if (geomID == NULL)
            {
                os << "null-geom";
            }
            switch (dGeomGetClass(geomID))
            {
                case dSphereClass:
                    os << "sphere-geom";
                    break;
                case dBoxClass:
                    os << "box-geom";
                    break;
                case dCapsuleClass:
                    os << "capsule-geom";
                    break;
                case dCylinderClass:
                    os << "cylinder-geom";
                    break;
                case dPlaneClass:
                    os << "plane-geom";
                    break;
                case dRayClass:
                    os << "ray-geom";
                    break;
                case dConvexClass:
                    os << "convex-geom";
                    break;
                case dGeomTransformClass:
                    os << "transform-geom";
                    break;
                case dTriMeshClass:
                    os << "mesh-geom";
                    break;
                case dHeightfieldClass:
                    os << "hfield-geom";
                    break;
                case dHashSpaceClass:
                    os << "hashspace-geom";
                    break;
                case dQuadTreeSpaceClass:
                    os << "quadspace-geom";
                    break;
                default:
                    os << "?-geom";
                    break;
            }

            const dReal* pos = dGeomGetPosition(geomID);
            if (pos)
            {
                os << " @ " << Vector3f(pos[0],pos[1],pos[2]);
            }
            dReal aabb[6];
            dGeomGetAABB(geomID, aabb);
            os << " " << Vector3f(abs(aabb[0] - aabb[1]), abs(aabb[2] - aabb[3]),abs(aabb[4] - aabb[5]));
            
            os << ": ";
            void* data = dGeomGetData(geomID);
            if (data != NULL)
            {
                os << static_cast<IPhysicsObject*>(data)->shared_from_this();
            }
            return os;
        }
    }

    using namespace Physics;

    /// @brief Constructor for PhysicsConfig.
    /// It is defined here because that's where the constants and the default values are used
    PhysicsConfig::PhysicsConfig() :
        tracking_radius(1000),
        gravity(kGravity),
        CFM(kWorldCFM),
        ERP(kWorldERP),
        damping(kDamping),
        damping_torque(kDampingTorque),
        default_mass(kRobotMass),
        step_size(kStepSize),
        slip1(0),
        slip2(0),
        soft_erp(0.8),
        soft_cfm(0.01),
        mu(0.5)
    {

    }

    IPhysicsEngine& IPhysicsEngine::instance()
    {
        return ODEPhysicsEngine::instance();
    }

    std::ostream& operator<<(std::ostream& os, const IPhysicsObject& body)
    {
        return body.stream(os);
    }

    ODEPhysicsEngine::ODEPhysicsEngine()
        : _world(0)                 // Physics simulation world (ODE id)
        , _space(0)                 // The collision space to use
        , _contactgroup(0)          // a group of ephemeral joints responsible for making things bounce, slide, etc.
        , _bodies()                 // bodies associated with simulation nodes
        , _enabled(false)           // whether the engine is enabled
        , _ground(0)                // the id of the "floor" plane
        , _geoms_by_id()            // a map from simulation entities to their physics geometries
        , _ids_by_geom()
        , _bodies_by_id()           // a map from simulation entities to their physics bodies
        , _ids_by_body()
    {

    }

    /// we use Boost singleton implementation
    ODEPhysicsEngine& ODEPhysicsEngine::instance()
    {
        return boost::details::pool::singleton_default<ODEPhysicsEngine>::instance();
    }

    /// we use Boost singleton implementation
    const ODEPhysicsEngine& ODEPhysicsEngine::const_instance()
    {
        return boost::details::pool::singleton_default<ODEPhysicsEngine>::instance();
    }

    /// This function creates an ODE world, a collision joint group and performs other setup functions
    void ODEPhysicsEngine::init()
    {
        dInitODE(); // initialize the ODE engine
        _world = dWorldCreate();
        dWorldSetGravity(_world, 0, 0, -GetPhysicsConfig().gravity);
        dWorldSetCFM(_world,GetPhysicsConfig().CFM);
        dWorldSetERP(_world,GetPhysicsConfig().ERP);
        dWorldSetAutoDisableFlag(_world, true );
        dWorldSetAutoDisableAverageSamplesCount( _world, 10 );
        dWorldSetContactMaxCorrectingVel(_world, 0.1);
        dWorldSetContactSurfaceLayer(_world, 0.001);
        _space = dHashSpaceCreate(0);
        _contactgroup = dJointGroupCreate(0);
        _ground = dCreatePlane(_space,0,0,1,-GetPhysicsConfig().ground_offset); // floor plane
        dGeomSetData(_ground, NULL); // no data - static object
        _enabled = true;
        LOG_F_MSG( "physics", "ODEPhysicsEngine using " << kEngine);

    }

    /// This needs to be called on simulation shutdown in order to free resources used by the physics engine
    void ODEPhysicsEngine::destroy()
    {
        LOG_F_MSG( "physics", "Shutting down ODEPhysicsEngine");
        _bodies.clear(); // this should cause all the physical bodies to be destroyed
        if (_space)
        {
            dSpaceDestroy(_space);
            _space = 0;
        }
        if (_contactgroup)
        {
            dJointGroupDestroy(_contactgroup);
            _contactgroup = 0;
        }
        if (_world)
        {
            dWorldDestroy(_world);
            _world = 0;
        }
        dCloseODE();
    }

	/// Create an IPhysicsObject for this engine
	/// @param ent SimEntity which owns this object
	IPhysicsObjectPtr ODEPhysicsEngine::createObject(SimEntityPtr ent)
	{
		IPhysicsObjectPtr obj(new ODEPhysicsObject(ent));
		return obj;
	}

    void ODEPhysicsEngine::addObject(IPhysicsObjectPtr obj)
    {
		_bodies.insert(obj);
		//
    }

    void ODEPhysicsEngine::addGeom(SimId id, dGeomID geom) {
        _geoms_by_id[id] = geom;
        _ids_by_geom[geom] = id;
    }

    void ODEPhysicsEngine::addBody(SimId id, dBodyID body) {
        _bodies_by_id[id] = body;
        _ids_by_body[body] = id;
    }

    /// we use dWorldQuickStep which is less precise but much faster
    void ODEPhysicsEngine::step(F32 dt)
    {
        if (!_enabled) return;
        NERO_PERF_EVENT_SCOPED(ODEPhysicsEngine__step);
        // step the world
        U32 nrofsteps = (U32) ceilf(dt/GetPhysicsConfig().step_size);
        if (nrofsteps > kMaxNrOfSteps)
        {
            LOG_F_DEBUG( "physics", "Physics clipping");
            nrofsteps = kMaxNrOfSteps;
        }
        if (nrofsteps > 0)
        {
            updateBeforePhysics();
            for (U32 i=0; i<nrofsteps; ++i)
            {
                dSpaceCollide(_space, this, &ODEPhysicsEngine::collisionCallback);
                dWorldQuickStep(_world, GetPhysicsConfig().step_size);
                dJointGroupEmpty(_contactgroup);
            }
            updateAfterPhysics();
        }
    }

    void ODEPhysicsEngine::updateAfterPhysics()
    {
        PhysicsObjectSet::iterator iter;
        PhysicsObjectSet remove; // remove objects that are too far away
        for (iter = _bodies.begin(); iter != _bodies.end(); ++iter)
        {
            (*iter)->UpdateAfterPhysics();
        }
        for (iter = remove.begin(); iter != remove.end(); ++iter)
        {
            _bodies.erase(*iter);
        }
    }

    void ODEPhysicsEngine::updateBeforePhysics()
    {
        PhysicsObjectSet::iterator iter;
        for (iter = _bodies.begin(); iter != _bodies.end(); ++iter)
        {
            (*iter)->UpdateBeforePhysics();
        }
    }

    void ODEPhysicsEngine::removeObject(IPhysicsObjectPtr obj)
    {
        _bodies.erase(obj);
    }

    /**
     * Find all the objects that fall within a sphere
     * @param origin the origin of the sphere
     * @param radius the radius of the sphere
     * @return the list of all the objects that fall within the specified sphere
     */
    SimDataVector ODEPhysicsEngine::findInSphere(const Vector3f& origin, F32 radius) {
        if (radius <= 0) {
            return SimDataVector();
        }
        // make the sphere:
        dGeomID sphere = dCreateSphere(_space, radius);
        dGeomSetPosition(sphere, origin.X, origin.Y, origin.Z);
        // make the result set
        dSpaceCollide2(sphere, (dGeomID) _space, (void*)this, &findInSphereCallback);
        // remove temporary sphere
        dSpaceRemove(_space, sphere);
        SimDataVector result(_found_objects.begin(), _found_objects.end());
        _found_objects.clear();
        return result;
    }

    using namespace Physics;

    /**
     * This function is called by ODE when two geometries are found to collide. This
     * version is used to accumulate a vector of SimEntityData objects in a region of
     * interest. The method has to be static so the data payload stores a pointer to
     * the ODEPhysicsEngine that made that call.
     * @param data a pointer to an ODEPhysicsEngine that made the query
     * @param o1 first object involved in the collision
     * @param o2 second object involved in the collision
     */
    void ODEPhysicsEngine::findInSphereCallback(void* data, dGeomID o1, dGeomID o2)
    {
        Assert(data);
        Assert(o1);
        Assert(o2);
        ODEPhysicsEngine* engine = static_cast<ODEPhysicsEngine*>(data);
        if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
        {
            dSpaceCollide2(o1,o2,data,&findInSphereCallback);
            return;
        }
        void* data1 = dGeomGetData(o1);
        void* data2 = dGeomGetData(o2);
        Assert(data1 == NULL || data2 == NULL);
        if (data1 != NULL) {
            engine->found(* static_cast<IPhysicsObject*>(data1)->GetSharedData());
        }
        if (data2 != NULL) {
            engine->found(* static_cast<IPhysicsObject*>(data2)->GetSharedData());
        }
    }

    /// Add a SimEntityData from a query to the current query queue of the engine
    /// @param data a data object that should be included in the query result
    void ODEPhysicsEngine::found(const SimEntityData& data)
    {
        _found_objects.push_back(data);
    }
    
    /// Add a geometry from a query to the current query queue of the engine
    /// @param geom a geom object that should be included in the query result
    void ODEPhysicsEngine::found(dGeomID geom)
    {
        GeomToSimMap::const_iterator needle = _ids_by_geom.find(geom);
        if (needle != _ids_by_geom.end()) {
            SimEntityPtr ent = Kernel::GetSimContext()->getSimulation()->Find(needle->second);
            if (ent) {
                found(ent->GetData());
            } else {
                LOG_F_WARNING("physics", "sensor hit an unknown object: " << geom);
            }
        }
    }

    /**
     * This is a callback that gets called by ODE when two geometries
     * are potentially colliding. We want to check for intersections between
     * spaces.
     * @param data pointer to the engine
     * @param o1 first geometry id
     * @param o2 second geometry id
     */
    void ODEPhysicsEngine::collisionCallback(void* data, dGeomID o1, dGeomID o2)
    {
        Assert(data);
        Assert(o1);
        Assert(o2);
        ODEPhysicsEngine* engine = static_cast<ODEPhysicsEngine*>(data);
        Assert(engine);
        if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
        {
            // this is actually a space collision, pass it on
            dSpaceCollide2(o1, o2, data, &collisionCallback);
            return;
        }
        // only collide objects with data payload (otherwise they are static)
        if (dGeomGetData(o1) || dGeomGetData(o2))
        {
            engine->collide(o1,o2);
        }

    }

    /**
     * collide two geometries by creating temporary contact joints for them
     * @param o1 first geometry id
     * @param o2 second geometry id
     */
    void ODEPhysicsEngine::collide(dGeomID o1, dGeomID o2)
    {
        // the max number of contact points has to fit in the lower 16 bits
        // of this number
        dContact contacts[kNrOfContacts]; // array to hold at least N contact points
        // dCollide views the contacts array as an array of dContactGeom with a skip parameter
        int n = dCollide(o1, o2, kNrOfContacts, &(contacts[0].geom), sizeof(dContact));
        if (n > 0)
        {
            for (int i = 0; i < n; ++i)
            {
                // for each contact point, set surface properties
                // TODO(ikarpov): take the magic out of these constants
                contacts[i].surface.mode = dContactBounce | dContactSoftCFM | dContactApprox1;
                contacts[i].surface.mu = GetPhysicsConfig().mu;
                contacts[i].surface.mu2 = 0;
                contacts[i].surface.bounce = 0.1;
                contacts[i].surface.bounce_vel = 0.1;
                contacts[i].surface.soft_erp = GetPhysicsConfig().soft_erp;
                contacts[i].surface.soft_cfm = GetPhysicsConfig().soft_cfm;
                // create a special ephemeral contact joint
                dJointID c = dJointCreateContact (_world, _contactgroup, &contacts[i]);
                // and attach the two bodies with it
                dJointAttach (c, dGeomGetBody(contacts[i].geom.g1), dGeomGetBody(contacts[i].geom.g2));
            }
        }
    }

    namespace Physics
    {
        /**
         * Convert ODE rotation quaternion to right-handed Euler angle in degrees
         * @param q ODE quaternion
         * @param euler Euler rotation vector to store the result in
         */
        void ODEQuaternionToEuler(const dQuaternion q, Vector3f& euler)
        {
            // ODE uses w,x,y,z in radians while Irrlicht uses x,z,y,w in degrees
            quaternion irr_q(q[1], q[3], q[2], q[0]); // make an Irrlicht quaternion
            irr_q.toEuler(euler); // convert it to Euler angles
            euler *= RAD_2_DEG;
        }

        /**
         * Convert right-handed Euler angles in degrees to ODE rotation quaternions
         * @param euler Euler angle vector to convert
         * @param q target ODE quaternion
         */
        void EulerToODEQuaternion(const Vector3f& euler, dQuaternion q)
        {
            Assert(q);
            quaternion iq(euler.X * DEG_2_RAD, euler.Y * DEG_2_RAD, euler.Z * DEG_2_RAD);
            q[0] = iq.W;
            q[1] = iq.X;
            q[2] = iq.Y;
            q[3] = iq.Z;
        }
    }

    /// enable or disable physics
    void ODEPhysicsEngine::SetEnabled(bool enabled)
    {
        LOG_F_DEBUG( "physics", "Physics Engine " << (enabled ? "enabled" : "disabled") );
        _enabled = enabled;
    }

    void toggle_physics()
    {
        IPhysicsEngine::instance().SetEnabled(!IPhysicsEngine::instance().IsEnabled());
    }

    /// enable physics
    void enable_physics()
    {
        IPhysicsEngine::instance().SetEnabled(true);
    }

    /// disable physics
    void disable_physics()
    {
        IPhysicsEngine::instance().SetEnabled(false);
    }

    PYTHON_BINDER(Physics)
    {
        boost::python::def("toggle_physics", &toggle_physics, "enable or disable physics");
        boost::python::def("enable_physics", &enable_physics, "enable physics");
        boost::python::def("disable_physics", &disable_physics, "disable physics");
    }

} // namespace OpenNero

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // NERO_BUILD_PHYSICS
