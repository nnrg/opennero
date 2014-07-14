//--------------------------------------------------------
// OpenNero : Camera
//  A camera wrapper
//--------------------------------------------------------

#ifndef _RENDER_CAMERA_H_
#define _RENDER_CAMERA_H_

#include "core/IrrUtil.h"
#include "core/Common.h"
#include "core/BoostCommon.h"
#include "game/SimEntity.h"
#include "game/objects/TemplatedObject.h"

namespace OpenNero
{
	/// shared pointer to a camera
    BOOST_SHARED_DECL(Camera);
    
    /// shared pointer to a first person camera template
    BOOST_SHARED_STRUCT(FPSCameraTemplate);

    /**
     * The camera class is an abstraction for the underlying rendering engine 
     * camera. It allows us to remain api independent while still harnessing 
     * the power of the underlying api.
    */
    class Camera : public BOOST_SHARED_THIS(Camera)
    {
    public:

        /// the kind of functionality the camera should express
        enum Functionality
        {
            kFunc_Maya, ///< The Maya camera feel
            kFunc_Nero, ///< The NERO camera feel
            kFunc_FPS,  ///< First Person camera
            kFunc_MAX
        };

    public:

        /// constructor
        Camera( IrrHandles& handles, float32_t rotateSpeed, float32_t moveSpeed, float32_t zoomSpeed );

        /// get the position of the camera
        Vector3f getPosition() const;

        /// get the look at location of the camera
        Vector3f getTarget() const;

        /// get the rotation of the camera
        Vector3f getRotation() const;

        /// get the up-vector of the camera
        Vector3f getUp() const;

        /// get the far plane of the camera
        F32 getFarPlane() const;

        /// get the near plane of the camera
        F32 getNearPlane() const;

        /// enable or disable edge scrolling
        void setEdgeScroll( bool enable );

        /// change the position of the camera
        void setPosition( const Vector3f& pos );

        /// change what the camera is looking at
        void setTarget( const Vector3f& target );

        /// change the rotation of the camera
        void setRotation( const Vector3f& rotation );

        /// set the far clipping plane distance from the eye
        /// @param farPlane distance to far plane
        void setFarPlane( F32 farPlane );

		/// set the near clipping plane distance from the eye
        /// @param nearPlane distance to near plane
		void setNearPlane( F32 nearPlane );

        /// get the current active camera
        ICameraSceneNode_IPtr getCamera() { return mCamera; }

        /// get the functionality of the camera
        Functionality getFunctionality() const;

        /// set the functionality of the camera
        void setFunctionality( Functionality f );

        /// attach the camera to the specified sim entity for the lifetime of the object
        bool attach( SimEntityPtr entity, const FPSCameraTemplatePtr );
        
        /// detach the camera and switch to the specified functionality
        void detach( Functionality f = kFunc_Nero);
		
		/// take a snapshot of whatever the camera is currently seeing
		void snapshot(const std::string& filename);

        /// output information about the camera to human-readable stream
        friend std::ostream& operator<<(std::ostream& output, const CameraPtr camera);

    private:

        IrrHandles_Weak         mIrr;

        float32_t               mRotateSpeed;
        float32_t               mMoveSpeed;
        float32_t               mZoomSpeed;

        /// The irrlicht camera ptr
        ICameraSceneNode_IPtr mCamera;

        /// Our functionality based cameras
        ICameraSceneNode_IPtr mFuncCameras[ kFunc_MAX ];
    };
    
	/// a template for a First Person camera
	struct FPSCameraTemplate
	{
		Vector3f attach_point; ///< where to attach the camera (relative to the body center of mass)
		Vector3f target;       ///< where the camera is looking
		F32 near_plane;        ///< near plane of the camera
		F32 far_plane;         ///< far plane of the camera
        
		/// construct the template from a property map
		FPSCameraTemplate(const std::string& prefix, const PropertyMap& propMap);
        
        /// Update rotation of camera
        void UpdateRotation(const SceneObject* scene_object, CameraPtr cam, Vector3f new_rotation);
        
        /// Update the position of the camera
        void UpdatePosition(const SceneObject* scene_object, CameraPtr cam, Vector3f new_position);
	};
    

} //end OpenNero

#endif // _RENDER_CAMERA_H_

