// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SCENE_NODE_ANIMATOR_CAMERA_NERO_H_INCLUDED__
#define __I_SCENE_NODE_ANIMATOR_CAMERA_NERO_H_INCLUDED__

#include "ISceneNodeAnimator.h"

namespace irr
{
struct SKeyMap;

namespace scene
{

	//! Special scene node animator for Nero-style cameras
	/**
	*/
	class ISceneNodeAnimatorCameraNero : public ISceneNodeAnimator
	{
	public:

		//! Returns the speed of movement in units per millisecond
		virtual f32 getMoveSpeed() const = 0;

		//! Sets the speed of movement in units per millisecond
		virtual void setMoveSpeed(f32 moveSpeed) = 0;

		//! Returns the rotation speed in degrees
		/** The degrees are equivalent to a half screen movement of the mouse,
		i.e. if the mouse cursor had been moved to the border of the screen since
		the last animation. */
		virtual f32 getRotateSpeed() const = 0;

		//! Set the rotation speed in degrees
		virtual void setRotateSpeed(f32 rotateSpeed) = 0;

		virtual bool isEdgeScroll() const = 0;

		virtual void setEdgeScroll(bool value) = 0;

		virtual f32 getRelEdgeSize() const = 0;

		virtual void setRelEdgeSize(f32 relEdgeSize) = 0;
	};

} // end namespace scene
} // end namespace irr

#endif

