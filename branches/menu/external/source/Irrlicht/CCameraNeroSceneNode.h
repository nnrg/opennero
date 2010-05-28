// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_CAMERA_NERO_SCENE_NODE_H_INCLUDED__
#define __C_CAMERA_NERO_SCENE_NODE_H_INCLUDED__

#include "ICursorControl.h"
#include "CCameraSceneNode.h"
#include "vector2d.h"
#include "SKeyMap.h"
#include "irrArray.h"

namespace irr
{
namespace scene
{

	class CCameraNeroSceneNode : public CCameraSceneNode
	{
    public:

        enum Actions
        {
            kAction_MoveForward,
            kAction_MoveBackwards,
            kAction_StrafeLeft,
            kAction_StrafeRight,
            kAction_RotateLeft,
            kAction_RotateRight,
            kAction_ZoomOut,
            kAction_ZoomIn,
			kAction_TiltUp,
			kAction_TiltDown,
            kActions_MAX
        };

	public:

		//! constructor
        CCameraNeroSceneNode(ISceneNode* parent, ISceneManager* mgr,
                             gui::ICursorControl* cursorControl, bool edgeScroll, f32 relEdgeSize,
		                     s32 id, f32 rotateSpeed , f32 moveSpeed, f32 zoomSpeed );
		
		//! destructor
		virtual ~CCameraNeroSceneNode();

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
		virtual ESCENE_NODE_TYPE getType() { return ESNT_CAMERA_FPS; }

		//! Disables or enables the camera to get key or mouse inputs.
		//! If this is set to true, the camera will respond to key inputs
		//! otherwise not.
		virtual void setInputReceiverEnabled(bool enabled);

		//! Sets the speed that this camera rotates
		virtual void setRotateSpeed(const f32 speed);

		//! Sets the speed that this camera moves
		virtual void setMoveSpeed(const f32 speed);

		//! Gets the rotation speed
		virtual f32 getRotateSpeed();

		// Gets the movement speed
		virtual f32 getMoveSpeed();

	private:

		struct SCamKeyMap
		{
			SCamKeyMap() {};
			SCamKeyMap(s32 a, EKEY_CODE k) : action(a), keycode(k) {}

			s32 action;
			EKEY_CODE keycode;
		};

		void allKeysUp();
		void animate( u32 timeMs );

		bool CursorKeys[kActions_MAX];		

        gui::ICursorControl* CursorControl;

		f32 MoveSpeed;
		f32 RotateSpeed;
		f32 ZoomSpeed;

		bool firstUpdate;        
		s32 LastAnimationTime;

        bool mEdgeScroll;
        f32 mEdgeBoundSize;

        f32 mWheelMovement;

		core::vector3df TargetVector;
		core::array<SCamKeyMap> KeyMap;				
	};

} // end namespace
} // end namespace

#endif // end __C_CAMERA_NERO_SCENE_NODE_H_INCLUDED__
