// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_SCENE_NODE_ANIMATOR_CAMERA_NERO_FP_H_INCLUDED__
#define __C_SCENE_NODE_ANIMATOR_CAMERA_NERO_FP_H_INCLUDED__

#include "ISceneNodeAnimatorCameraNeroFP.h"
#include "vector2d.h"
#include "SKeyMap.h"
#include "irrArray.h"

namespace irr
{
namespace gui
{
	class ICursorControl;
}

namespace scene
{

	//! Special scene node animator for NERO-style first person cameras
	class CSceneNodeAnimatorCameraNeroFP : public ISceneNodeAnimator
	{
	public:

		//! Constructor
		CSceneNodeAnimatorCameraNeroFP(gui::ICursorControl* cursorControl);
			
		//! Destructor
		virtual ~CSceneNodeAnimatorCameraNeroFP();

		//! Animates the scene node, currently only works on cameras
		virtual void animateNode(ISceneNode* node, u32 timeMs);

		//! Event receiver
		virtual bool OnEvent(const SEvent& event);

		//! This animator will receive events when attached to the active camera
		virtual bool isEventReceiverEnabled() const
		{
			return true;
		}

		//! Returns the type of this animator
		virtual ESCENE_NODE_ANIMATOR_TYPE getType() const
		{
			return ESNAT_CAMERA_NERO_FP;
		}

		//! Creates a clone of this animator.
		/** Please note that you will have to drop
		(IReferenceCounted::drop()) the returned pointer once you're
		done with it. */
		virtual ISceneNodeAnimator* createClone(ISceneNode* node, ISceneManager* newManager=0);

	private:
		gui::ICursorControl *CursorControl;
		s32 LastAnimationTime;
		bool firstUpdate;
	};

} // end namespace scene
} // end namespace irr

#endif // __C_SCENE_NODE_ANIMATOR_CAMERA_NERO_FP_H_INCLUDED__

