//--------------------------------------------------------
// OpenNero : SceneObject
//  Responsible for maintaining object presence on-screen
//--------------------------------------------------------

#ifndef _GAME_RENDER_SCENEOBJECT_H_
#define _GAME_RENDER_SCENEOBJECT_H_

#include "game/objects/SimEntityComponent.h"
#include "game/objects/TemplatedObject.h"
#include "core/IrrUtil.h"

#include <list>

namespace OpenNero
{
    using namespace irr;
    using namespace irr::video;
    using namespace irr::scene;

    /// A unique identifier for a scene object
    typedef uint32_t SceneObjectId;

    /// @cond
    BOOST_SHARED_DECL( SceneObjectIdManager );
    BOOST_SHARED_DECL( SceneObjectTemplate );
    BOOST_SHARED_DECL( ObjectTemplate );
    BOOST_SHARED_DECL( SceneObject );
    BOOST_SHARED_DECL( Camera );
    /// @endcond

    // Irrlicht data types

    // the type of material to use on an object
    typedef E_MATERIAL_TYPE IrrMaterialType;

    /// a wrapper for the irrlicht material flags
    struct IrrMaterialFlag
    {
        E_MATERIAL_FLAG     mFlag;  ///< irrlicht flag
        bool                mValue; ///< enabled or disabled

        IrrMaterialFlag() : mFlag( (E_MATERIAL_FLAG)-1 ), mValue(false) {}
        /// @param flag the Irrlicht material flag constant
        /// @param val value
        IrrMaterialFlag( E_MATERIAL_FLAG flag, bool val ) : mFlag(flag), mValue(val) {}
    };

    /// @cond
	BOOST_SHARED_STRUCT(FPSCameraTemplate);
    /// @endcond

    /**
     * The scene object template stores information about how to
     * render a scene node to the screen
    */
    class SceneObjectTemplate : public ObjectTemplate
    {

    public:

        SceneObjectTemplate( const SceneObjectTemplate& objTempl );
        SceneObjectTemplate( SimFactoryPtr factory, const PropertyMap& propMap );
        virtual ~SceneObjectTemplate();

        // create the concrete type of template we need
        static boost::shared_ptr<SceneObjectTemplate> createTemplate(SimFactoryPtr factory, const PropertyMap& propMap);

        /// returns the template type of a scene object
        static const std::string TemplateType() { return "SceneObject"; }

    public:

        Vector3f                        mScale;             ///< Scale of node
        Vector2f                        mScaleTexture;      ///< Scale of texture
        std::vector<ITexture_IPtr>      mTextures;          ///< The textures in use
        std::vector<IrrMaterialFlag>    mMaterialFlags;     ///< the material flags
        IrrMaterialType                 mMaterialType;      ///< the type of the material
        std::string                     mHeightmap;         ///< heightmap to use (if terrain)
        std::string                     mParticleSystem;    ///< Particle System File
        IAnimatedMesh_IPtr              mAniMesh;           ///< animated mesh to use (if valid)
        bool                            mCastsShadow;       ///< whether or not the object casts a shadow
        bool                            mDrawBoundingBox;   ///< whether or not to draw the object's bounding box
        bool                            mDrawLabel;         ///< whether or not to draw the object's label
        FPSCameraTemplatePtr            mFPSCamera;         ///< information about camera attachment
        float32_t                       mAnimationSpeed;    ///< animation speed
		uint32_t                        mCollisionMask;     ///< mask of objects this object collides with
    };

    SimId ConvertSceneIdToSimId(uint32_t simId);

    uint32_t ConvertSimIdToSceneId(SimId id, uint32_t type);

    /**
     * A scene object is responsible for being able to provide the proper
     * information to irrlicht such that a node can be rendered
    */
    class SceneObject : public SimEntityComponent, public TemplatedObject, public BOOST_SHARED_THIS(SceneObject)
    {
    public:

        // Allow the Camera to attach to the SceneObject
        friend class Camera;

        // Allow SimEntity to manage scene object ids.
        friend class SimEntity;

        /// number of bits in the TYPE_ bitmask of a SceneObjectId
        static const SceneObjectId BITMASK_SIZE = 4;

        SceneObject(SimEntityPtr parent = SimEntityPtr());
        SceneObject( const SceneObject& so );

    public:

        virtual ~SceneObject();

        /// load the scene object from a template
        bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data );

        /// update the scene object by a time delta
        void ProcessTick( float32_t dt );

        /// get the template
        ObjectTemplatePtr GetObjectTemplate();

        /// the shared sim entity data
        void SetSharedState( SimEntityData* sharedData );

        /// set the debugging text for this scene node
        void SetText(const std::string& str);

		/// set the animation of this node if we know how to do it
		bool SetAnimation(const std::string& animation_type);

        /// set the animation speed fo this node
        void SetAnimationSpeed(float32_t framesPerSecond);

        /// set the position of this scene object
        void SetPosition(const Vector3f& pos);

        /// set the rotation of this scene object
        void SetRotation(const Vector3f& rotation);

        /// bounding box data
        BBoxf getBoundingBox() const;

        /// get transformed bounding box
        BBoxf getTransformedBoundingBox() const;

        /// transform the given vector by applying the object's matrix
        Vector3f transformVector(const Vector3f& vect) const;

        /// get the scene object id
        SceneObjectId GetId();

        /// does this scene object have a mesh?
        bool hasMesh() const { return mTerrSceneNode != NULL; }

        /// get mesh internals
        bool getMeshBuffer(MeshBuffer &mb, S32 lod) const;

        /// get object scale
        Vector3f getScale() const;

        /// get object position
        Vector3f getPosition() const;

        /// get object rotation
        Vector3f getRotation() const;

        /// Get the animation name
        std::string GetAnimation() const;

        /// Get the animation speed
        float32_t GetAnimationSpeed() const;

        /// can we possibly collide with any other object?
        bool canCollide() const;

        /// attach an FPS camera to this scene object
        void attachCamera(CameraPtr cam);

        /// get the triangle selector for this scene node, creating it if needed
        ITriangleSelector_IPtr GetTriangleSelector();

        /// return true if a collision was detected
        bool collisionOccurred();

    private:

        // not sure we want to expose these.
        // TODO : Should there be a copy constructor?
        SceneObject& operator=( const SceneObject& obj );

        /// make sure that the new position assigned to this object
        /// actually gets assigned (disregard collisions for one step)
        void DisregardCollisions();

        // this points to the actual node in use
        ISceneNode_IPtr                     mSceneNode;             ///< Irr Scene node

        /// only one of these will be used for any object
        ///@{
        IAnimatedMeshSceneNode_IPtr         mAniSceneNode;          ///< Irr animated mesh node
        ITerrainSceneNode_IPtr              mTerrSceneNode;         ///< Irr Terrain node
        IParticleSystemSceneNode*           mParticleSystemNode;    ///< Irr Particle System node
        ///@}
        ITextSceneNode*                     mTextNode;              ///< optional text attached

        SceneObjectTemplatePtr              mSceneObjectTemplate;   ///< The template to use

        S32                                 mStartFrame;            ///< starting animation frame
        S32                                 mEndFrame;              ///< ending animation frame
        CameraPtr                           mCamera;                ///< camera that is attached to us (if any)
		FPSCameraTemplatePtr				mFPSCamera;			    ///< information about whether to attach a camera to this object
        std::string                         mAnimation;             ///< current animation

        /// collision response animator
        ISceneNodeAnimatorCollisionResponse_IPtr    mCollider;
    };

};//end OpenNero

#endif // _GAME_RENDER_SCENEOBJECT_H_

