// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CCameraNeroSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "os.h"
#include "Keycodes.h"
#include "quaternion.h"

namespace irr
{
namespace scene
{

const f32 MAX_VERTICAL_ANGLE = 88.0f;

//! constructor
CCameraNeroSceneNode::CCameraNeroSceneNode(ISceneNode* parent, ISceneManager* mgr,
        gui::ICursorControl* cursorControl, bool edgeScroll, f32 relEdgeSize,
        s32 id, f32 rotateSpeed, f32 moveSpeed, f32 zoomSpeed )		
: CCameraSceneNode(parent, mgr, id),
	MoveSpeed(moveSpeed/1000.0f), 
    RotateSpeed(rotateSpeed),
	firstUpdate(true), 
    LastAnimationTime(0),
    CursorControl(cursorControl),
    ZoomSpeed(zoomSpeed),
    mEdgeScroll(edgeScroll),
    mEdgeBoundSize(relEdgeSize),
    mWheelMovement(0)
{
	#ifdef _DEBUG
	setDebugName("CCameraNeroSceneNode");
	#endif	

    if( CursorControl )
        CursorControl->grab();

	recalculateViewArea();

	allKeysUp();

    // create default key map
	KeyMap.push_back(SCamKeyMap(kAction_MoveForward,    irr::KEY_KEY_W));
	KeyMap.push_back(SCamKeyMap(kAction_MoveBackwards,  irr::KEY_KEY_S));
	KeyMap.push_back(SCamKeyMap(kAction_StrafeLeft,     irr::KEY_KEY_A));
	KeyMap.push_back(SCamKeyMap(kAction_StrafeRight,    irr::KEY_KEY_D));
    KeyMap.push_back(SCamKeyMap(kAction_RotateLeft,     irr::KEY_KEY_Q));
    KeyMap.push_back(SCamKeyMap(kAction_RotateRight,    irr::KEY_KEY_E));
    KeyMap.push_back(SCamKeyMap(kAction_ZoomOut,        irr::KEY_KEY_Z));
    KeyMap.push_back(SCamKeyMap(kAction_ZoomIn,         irr::KEY_KEY_C));
	KeyMap.push_back(SCamKeyMap(kAction_TiltUp,         irr::KEY_KEY_R));
	KeyMap.push_back(SCamKeyMap(kAction_TiltDown,       irr::KEY_KEY_F));
}


//! destructor
CCameraNeroSceneNode::~CCameraNeroSceneNode()
{
    if( CursorControl )
        CursorControl->drop();
}


//! It is possible to send mouse and key events to the camera. Most cameras
//! may ignore this input, but camera scene nodes which are created for 
//! example with scene::ISceneManager::addMayaCameraSceneNode or
//! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
//! for changing their position, look at target or whatever. 
bool CCameraNeroSceneNode::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT)
	{
		const u32 cnt = KeyMap.size();
		for (u32 i=0; i<cnt; ++i)
			if (KeyMap[i].keycode == event.KeyInput.Key)
			{
				CursorKeys[KeyMap[i].action] = event.KeyInput.PressedDown; 

				if ( InputReceiverEnabled )
					return true;
			}
	}

    if( event.EventType == EET_MOUSE_INPUT_EVENT )
    {
        if( event.MouseInput.Event == EMIE_MOUSE_WHEEL )
            mWheelMovement += event.MouseInput.Wheel;        
    }

	return false;
}



//! OnAnimate() is called just before rendering the whole scene.
//! nodes may calculate or store animations here, and may do other useful things,
//! dependent on what they are.
void CCameraNeroSceneNode::OnAnimate(u32 timeMs)
{
	animate( timeMs );

	core::list<ISceneNodeAnimator*>::Iterator ait = Animators.begin();
				for (; ait != Animators.end(); ++ait)
					(*ait)->animateNode(this, timeMs);

	updateAbsolutePosition();
	Target = getPosition() + TargetVector;

	core::list<ISceneNode*>::Iterator it = Children.begin();
				for (; it != Children.end(); ++it)
					(*it)->OnAnimate(timeMs);
}


void CCameraNeroSceneNode::animate( u32 timeMs )
{
	const u32 camIsMe = SceneManager->getActiveCamera() == this;

	if (firstUpdate)
	{
		LastAnimationTime = os::Timer::getTime();
		firstUpdate = false;
	}

	// get time. only operate on valid camera
	f32 timeDiff = 0.f;

	if ( camIsMe )
	{
		timeDiff = (f32) ( timeMs - LastAnimationTime );
		LastAnimationTime = timeMs;
	}

	f32 timeDiffScalar = timeDiff/1000.0f;
	if( timeDiffScalar < 0.001f )
	  timeDiffScalar = 0.001f;

    // state
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
	core::vector3df pos = getPosition();

	// Update rotation
	Target.set(0,0,1);

    if (InputReceiverEnabled && camIsMe)
	{   
        // edge scroll?

        if( CursorControl && mEdgeScroll )
        {
            core::position2df mousePos = CursorControl->getRelativePosition();
            
            if( mousePos.X < mEdgeBoundSize )
                strafeLeft = true;

            if( mousePos.X > 1.0f - mEdgeBoundSize )
                strafeRight = true;

            if( mousePos.Y <mEdgeBoundSize )
                moveForward = true;

            if( mousePos.Y > 1.0f - mEdgeBoundSize )
                moveBack = true;
        }        

        if( rotateLeft )
            RelativeRotation.Y -= RotateSpeed * timeDiffScalar;

        if( rotateRight )
            RelativeRotation.Y += RotateSpeed * timeDiffScalar;

        if( zoomOut )
            mWheelMovement += ZoomSpeed/50 * timeDiffScalar;

        if( zoomIn )
            mWheelMovement -= ZoomSpeed/50 * timeDiffScalar;

		if (tiltUp)
			RelativeRotation.X -= RotateSpeed * timeDiffScalar;

		if (tiltDown)
			RelativeRotation.X += RotateSpeed * timeDiffScalar;
    }

	// set target

	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df( RelativeRotation.X, RelativeRotation.Y, 0));
	mat.transformVect(Target);

	core::vector3df movedir = Target;
    core::vector3df zoomdir = Target;	
	movedir.Y = 0.f;

	movedir.normalize();

	if (InputReceiverEnabled && camIsMe)
	{
		if (moveForward)
			pos += movedir * timeDiff * MoveSpeed * timeDiffScalar;

		if (moveBack)
			pos -= movedir * timeDiff * MoveSpeed * timeDiffScalar;

		// strafing

		core::vector3df strafevect = Target;
		strafevect = strafevect.crossProduct(UpVector);		
		strafevect.Y = 0.0f;

		strafevect.normalize();

		if ( strafeLeft )
			pos += strafevect * timeDiff * MoveSpeed * timeDiffScalar;

		if ( strafeRight )
			pos -= strafevect * timeDiff * MoveSpeed * timeDiffScalar;
	
        if( mWheelMovement )
        {
            pos += ZoomSpeed*mWheelMovement * zoomdir * timeDiffScalar;
            mWheelMovement = 0;
        }
    }

	// write translation

	setPosition(pos);	

	// write right target

	TargetVector = Target;
	Target += pos;
}

void CCameraNeroSceneNode::allKeysUp()
{
	for (s32 i=0; i<kActions_MAX; ++i)
		CursorKeys[i] = false;
}


//! sets the look at target of the camera
//! \param pos: Look at target of the camera.
void CCameraNeroSceneNode::setTarget(const core::vector3df& tgt)
{
	updateAbsolutePosition();
	core::vector3df vect = tgt - getAbsolutePosition();
	vect = vect.getHorizontalAngle();
	RelativeRotation.X = vect.X;
	RelativeRotation.Y = vect.Y;

	if (RelativeRotation.X > MAX_VERTICAL_ANGLE)
		 RelativeRotation.X -= 360.0f;
}

// WE OVERRIDE THIS FOR OUR FUNCTIONALITY
// determine whether or not to edge scroll
void CCameraNeroSceneNode::setInputReceiverEnabled(bool enabled)
{
    mEdgeScroll = enabled;
    /*
   // So we don't skip when we return from a non-enabled mode and the
   // mouse cursor is now not in the middle of the screen
   if( !InputReceiverEnabled && enabled )
      firstUpdate = true;

   InputReceiverEnabled = enabled;
   */
} 

//! Sets the rotation speed
void CCameraNeroSceneNode::setRotateSpeed(const f32 speed)
{
	RotateSpeed = speed;	
}

//! Sets the movement speed
void CCameraNeroSceneNode::setMoveSpeed(const f32 speed)
{
	MoveSpeed = speed;
}

//! Gets the rotation speed
f32 CCameraNeroSceneNode::getRotateSpeed()
{
	return RotateSpeed;
}

// Gets the movement speed
f32 CCameraNeroSceneNode::getMoveSpeed()
{
	return MoveSpeed;
}

} // end namespace
} // end namespace
