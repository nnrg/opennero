#ifndef __C_CAMERA_NERO_FP_SCENE_NODE_H_INCLUDED__
#define __C_CAMERA_NERO_FP_SCENE_NODE_H_INCLUDED__

#include "CCameraSceneNode.h"
#include "vector2d.h"

namespace irr
{
    namespace scene
    {

        class CCameraNeroFPSceneNode : public CCameraSceneNode
        {
        public:

            //! constructor
            CCameraNeroFPSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id, 
               const core::vector3df& position = core::vector3df(0,0,0), 
               const core::vector3df& target = core::vector3df(100,0,0));

            //! destructor
            virtual ~CCameraNeroFPSceneNode();

            //! It is possible to send mouse and key events to the camera. Most cameras
            //! may ignore this input, but camera scene nodes which are created for 
            //! example with scene::ISceneManager::addMayaCameraSceneNode or
            //! scene::ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
            //! for changing their position, look at target or whatever. 
            virtual bool OnEvent(const SEvent& event);

            //! OnAnimate() is called just before rendering the whole scene.
            //! nodes may calculate or store animations here, and may do other useful things,
            //! dependent on what they are.
            virtual void OnAnimate(u32 timeMs);

            //! sets the look at target of the camera
            //! \param pos: Look at target of the camera.
            virtual void setTarget(const core::vector3df& pos);

            //! Returns type of the scene node
            virtual ESCENE_NODE_TYPE getType() const { return ESNT_CAMERA_FPS; }

        };

    } // end namespace
} // end namespace

#endif /* __C_CAMERA_NERO_FP_SCENE_NODE_H_INCLUDED__ */
