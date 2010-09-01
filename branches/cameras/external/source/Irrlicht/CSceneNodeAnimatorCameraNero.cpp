// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorCameraNero.h"
#include "ICursorControl.h"
#include "ICameraSceneNode.h"
#include "SViewFrustum.h"
#include "ISceneManager.h"
#include "ISceneNodeAnimatorCollisionResponse.h"

namespace irr
{
namespace scene
{

//! constructor
CSceneNodeAnimatorCameraNero::CSceneNodeAnimatorCameraNero(
        gui::ICursorControl* cursor,
        bool edgeScroll,
        f32 relEdgeSize,
        f32 rotateSpeed,
        f32 moveSpeed,
        f32 zoomSpeed)
    : CursorControl(cursor),
      MoveSpeed(moveSpeed/100.0f),
      RotateSpeed(rotateSpeed),
      ZoomSpeed(zoomSpeed),
      LastAnimationTime(0),
      firstUpdate(true),
      EdgeScroll(edgeScroll),
      WheelMovement(0),
	  EdgeBoundSize(relEdgeSize)
{
    #ifdef _DEBUG
    setDebugName("CSceneNodeAnimatorCameraNero");
    #endif

    if (CursorControl)
    {
        CursorControl->grab();
    }

    allKeysUp();

	// create default key map
	KeyMap.push_back(SCamKeyMap(kAction_MoveForward, irr::KEY_KEY_W));
	KeyMap.push_back(SCamKeyMap(kAction_MoveBackwards, irr::KEY_KEY_S));
	KeyMap.push_back(SCamKeyMap(kAction_StrafeLeft, irr::KEY_KEY_A));
	KeyMap.push_back(SCamKeyMap(kAction_StrafeRight, irr::KEY_KEY_D));
	KeyMap.push_back(SCamKeyMap(kAction_RotateLeft, irr::KEY_KEY_Q));
	KeyMap.push_back(SCamKeyMap(kAction_RotateRight, irr::KEY_KEY_E));
	KeyMap.push_back(SCamKeyMap(kAction_ZoomOut, irr::KEY_KEY_Z));
	KeyMap.push_back(SCamKeyMap(kAction_ZoomIn, irr::KEY_KEY_C));
	KeyMap.push_back(SCamKeyMap(kAction_TiltUp, irr::KEY_KEY_R));
	KeyMap.push_back(SCamKeyMap(kAction_TiltDown, irr::KEY_KEY_F));
}

//! destructor
CSceneNodeAnimatorCameraNero::~CSceneNodeAnimatorCameraNero()
{
    if (CursorControl)
        CursorControl->drop();
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for
//! example with scene::ISceneManager::addNeroCameraSceneNode or
//! scene::ISceneManager::addMeshViewerCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever.
bool CSceneNodeAnimatorCameraNero::OnEvent(const SEvent& event)
{
	switch(event.EventType)
	{
	case EET_KEY_INPUT_EVENT:
		for (u32 i=0; i<KeyMap.size(); ++i)
		{
			if (KeyMap[i].keycode == event.KeyInput.Key)
			{
				CursorKeys[KeyMap[i].action] = event.KeyInput.PressedDown;
				return true;
			}
		}
		break;

	case EET_MOUSE_INPUT_EVENT:
        if( event.MouseInput.Event == EMIE_MOUSE_WHEEL )
        {
            WheelMovement += event.MouseInput.Wheel;
            return true;
        }
		break;

	default:
		break;
	}

	return false;
}


void CSceneNodeAnimatorCameraNero::animateNode(ISceneNode* node, u32 timeMs)
{
	if (!node || node->getType() != ESNT_CAMERA)
		return;
        
	ICameraSceneNode* camera = static_cast<ICameraSceneNode*>(node);

	if (firstUpdate)
	{
		camera->updateAbsolutePosition();
		if (CursorControl && camera)
		{
			CursorControl->setPosition(0.5f, 0.5f);
			CursorPos = CenterCursor = CursorControl->getRelativePosition();
		}

		LastAnimationTime = timeMs;

		firstUpdate = false;
	}

	// If the camera isn't the active camera, and receiving input, then don't process it.
	if(!camera->isInputReceiverEnabled())
		return;

	scene::ISceneManager * smgr = camera->getSceneManager();
	if(smgr && smgr->getActiveCamera() != camera)
		return;

	// get time
	f32 timeDiff = (f32) ( timeMs - LastAnimationTime )/1000.0f;
	LastAnimationTime = timeMs;
    if (timeDiff < 0.001f)
        timeDiff = 0.001f;

    // input state
    bool moveForward = CursorKeys[kAction_MoveForward];
    bool moveBack    = CursorKeys[kAction_MoveBackwards];	
    bool strafeLeft  = CursorKeys[kAction_StrafeLeft];
    bool strafeRight = CursorKeys[kAction_StrafeRight];
    bool rotateLeft  = CursorKeys[kAction_RotateLeft];
    bool rotateRight = CursorKeys[kAction_RotateRight];
    bool zoomIn      = CursorKeys[kAction_ZoomIn];
    bool zoomOut     = CursorKeys[kAction_ZoomOut];
	bool tiltUp      = CursorKeys[kAction_TiltUp];
	bool tiltDown    = CursorKeys[kAction_TiltDown];
    
	// update position
	core::vector3df pos = camera->getPosition();

	// Update rotation
    core::vector3df Target = camera->getTarget() - camera->getAbsolutePosition();
    core::vector3df RelativeRotation = Target.getHorizontalAngle();
    Target.set(0,0,1);


	if (CursorControl)
	{
        // edge scroll?
        if (CursorControl && EdgeScroll)
        {
            core::position2df mousePos = CursorControl->getRelativePosition();
            
            if (mousePos.X < EdgeBoundSize)
            {
                strafeLeft = true;
            }
            if (mousePos.X > 1.0f - EdgeBoundSize)
            {
                strafeRight = true;
            }
            if (mousePos.Y < EdgeBoundSize)
            {
                moveForward = true;
            }
            if (mousePos.Y > 1.0f - EdgeBoundSize)
            {
                moveBack = true;
            }
        }
        
        if( rotateLeft )
            RelativeRotation.Y -= RotateSpeed * timeDiff;

        if( rotateRight )
            RelativeRotation.Y += RotateSpeed * timeDiff;

        if( zoomOut )
            WheelMovement += ZoomSpeed/50 * timeDiff;

        if( zoomIn )
            WheelMovement -= ZoomSpeed/50 * timeDiff;

		if (tiltUp)
			RelativeRotation.X -= RotateSpeed * timeDiff;

		if (tiltDown)
			RelativeRotation.X += RotateSpeed * timeDiff;
    }            

	// set target
    
	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df(RelativeRotation.X, RelativeRotation.Y, 0));
	mat.transformVect(Target);

    core::vector3df movedir = Target;
    core::vector3df zoomdir = Target;
    movedir.Y = 0.0f;
	movedir.normalize();

	if (moveForward)
		pos += movedir * timeDiff * MoveSpeed;

	if (moveBack)
		pos -= movedir * timeDiff * MoveSpeed;

	// strafing

	core::vector3df strafevect = Target;
	strafevect = strafevect.crossProduct(camera->getUpVector());
    strafevect.Y = 0.0f;

	strafevect.normalize();

	if (strafeLeft)
		pos += strafevect * timeDiff * MoveSpeed;

	if (strafeRight)
		pos -= strafevect * timeDiff * MoveSpeed;
        
    if (WheelMovement)
    {
        pos += ZoomSpeed * WheelMovement * zoomdir * timeDiff;
        WheelMovement = 0;
    }

	// write translation
	camera->setPosition(pos);

	// write right target
	Target += pos;
	camera->setTarget(Target);
}

void CSceneNodeAnimatorCameraNero::allKeysUp()
{
	for (u32 i=0; i<kActions_MAX; ++i)
		CursorKeys[i] = false;
}

//! Sets the rotation speed
void CSceneNodeAnimatorCameraNero::setRotateSpeed(f32 speed)
{
    RotateSpeed = speed;
}


//! Sets the movement speed
void CSceneNodeAnimatorCameraNero::setMoveSpeed(f32 speed)
{
    MoveSpeed = speed;
}


//! Sets the zoom speed
void CSceneNodeAnimatorCameraNero::setZoomSpeed(f32 speed)
{
    ZoomSpeed = speed;
}


//! Gets the rotation speed
f32 CSceneNodeAnimatorCameraNero::getRotateSpeed() const
{
    return RotateSpeed;
}


// Gets the movement speed
f32 CSceneNodeAnimatorCameraNero::getMoveSpeed() const
{
    return MoveSpeed;
}


//! Gets the zoom speed
f32 CSceneNodeAnimatorCameraNero::getZoomSpeed() const
{
    return ZoomSpeed;
}

bool CSceneNodeAnimatorCameraNero::isEdgeScroll() const
{
	return EdgeScroll;
}

void CSceneNodeAnimatorCameraNero::setEdgeScroll(bool value)
{
	EdgeScroll = value;
}

f32 CSceneNodeAnimatorCameraNero::getRelEdgeSize() const
{
	return EdgeBoundSize;
}

void CSceneNodeAnimatorCameraNero::setRelEdgeSize(f32 relEdgeSize)
{
	EdgeBoundSize = relEdgeSize;
}


ISceneNodeAnimator* CSceneNodeAnimatorCameraNero::createClone(ISceneNode* node, ISceneManager* newManager)
{
    CSceneNodeAnimatorCameraNero * newAnimator =
        new CSceneNodeAnimatorCameraNero(CursorControl,
                                        EdgeScroll,
                                        EdgeBoundSize,
                                        RotateSpeed,
                                        MoveSpeed,
                                        ZoomSpeed);
    return newAnimator;
}

} // end namespace
} // end namespace

