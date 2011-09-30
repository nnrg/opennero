// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorCameraNeroFP.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "Keycodes.h"
#include "ICursorControl.h"
#include "ICameraSceneNode.h"
#include "ISceneNodeAnimatorCollisionResponse.h"

namespace irr
{
namespace scene
{

//! constructor
CSceneNodeAnimatorCameraNeroFP::CSceneNodeAnimatorCameraNeroFP(gui::ICursorControl* cursorControl)
: CursorControl(cursorControl)
{
	#ifdef _DEBUG
	setDebugName("CCameraSceneNodeAnimatorNeroFP");
	#endif

	if (CursorControl)
		CursorControl->grab();
}


//! destructor
CSceneNodeAnimatorCameraNeroFP::~CSceneNodeAnimatorCameraNeroFP()
{
	if (CursorControl)
		CursorControl->drop();
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for
//! example with scene::ISceneManager::addMayaCameraSceneNode or
//! scene::ISceneManager::addNeroFPCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever.
bool CSceneNodeAnimatorCameraNeroFP::OnEvent(const SEvent& evt)
{
	return false;
}


void CSceneNodeAnimatorCameraNeroFP::animateNode(ISceneNode* node, u32 timeMs)
{
    return;
}


ISceneNodeAnimator* CSceneNodeAnimatorCameraNeroFP::createClone(ISceneNode* node, ISceneManager* newManager)
{
	CSceneNodeAnimatorCameraNeroFP * newAnimator =
		new CSceneNodeAnimatorCameraNeroFP(CursorControl);
	return newAnimator;
}

} // namespace scene
} // namespace irr

