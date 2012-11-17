// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SCENE_NODE_ANIMATOR_CAMERA_NERO_H_INCLUDED__
#define __C_SCENE_NODE_ANIMATOR_CAMERA_NERO_H_INCLUDED__

#include "ISceneNodeAnimatorCameraNero.h"
#include "ICameraSceneNode.h"
#include "SKeyMap.h"
#include "vector2d.h"

namespace irr
{

namespace gui
{
	class ICursorControl;
}

namespace scene
{

	//! Special scene node animator for Nero RTS-style camera
	/**
	*/
	class CSceneNodeAnimatorCameraNero : public ISceneNodeAnimatorCameraNero 
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

		//! Constructor

		//! Constructor
		CSceneNodeAnimatorCameraNero(gui::ICursorControl* cursor,
                                     bool edgeScroll = false, 
                                     f32 relEdgeSize = 0.0f, 
                                     f32 rotateSpeed = 100.0f,
                                     f32 moveSpeed = .5f,
                                     f32 zoomSpeed = 70.0f);

		//! Destructor
		virtual ~CSceneNodeAnimatorCameraNero();

		//! Animates the scene node, currently only works on cameras
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! Event receiver
		virtual bool OnEvent(const SEvent& event);

		//! Returns the speed of movement in units per second
		virtual f32 getMoveSpeed() const;

		//! Sets the speed of movement in units per second
		virtual void setMoveSpeed(f32 moveSpeed);

		//! Returns the rotation speed
		virtual f32 getRotateSpeed() const;

		//! Set the rotation speed
		virtual void setRotateSpeed(f32 rotateSpeed);

		virtual f32 getZoomSpeed() const;

		virtual void setZoomSpeed(f32 zoomSpeed);

		virtual bool isEdgeScroll() const;

		virtual void setEdgeScroll(bool value);

		virtual f32 getRelEdgeSize() const;

		virtual void setRelEdgeSize(f32 relEdgeSize);

		//! This animator will receive events when attached to the active camera
		virtual bool isEventReceiverEnabled() const
		{
			return true;
		}

		//! Returns the type of this animator
		virtual ESCENE_NODE_ANIMATOR_TYPE getType() const
		{
			return ESNAT_CAMERA_NERO;
		}

		//! Creates a clone of this animator.
		/** Please note that you will have to drop
		(IReferenceCounted::drop()) the returned pointer once you're
		done with it. */
		virtual ISceneNodeAnimator* createClone(ISceneNode* node, ISceneManager* newManager=0);

		struct SCamKeyMap
		{
			SCamKeyMap() {};
			SCamKeyMap(s32 a, EKEY_CODE k) : action(a), keycode(k) {}

			s32 action;
			EKEY_CODE keycode;
		};

		//! Sets the keyboard mapping for this animator
		/** Helper function for the clone method.
		\param keymap the new keymap array */
		void setKeyMap(const core::array<SCamKeyMap>& keymap);

	private:

		void allKeysUp();

		gui::ICursorControl *CursorControl;

		f32 MoveSpeed;
		f32 RotateSpeed;
		f32 ZoomSpeed;

		s32 LastAnimationTime;

		core::array<SCamKeyMap> KeyMap;
		core::position2d<f32> CenterCursor, CursorPos;

		bool CursorKeys[kActions_MAX];

		bool firstUpdate;

        bool EdgeScroll;
		f32 WheelMovement;
        f32 EdgeBoundSize;
	};

} // end namespace scene
} // end namespace irr

#endif

