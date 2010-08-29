// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSceneNodeAnimatorCameraNero.h"
#include "ICursorControl.h"
#include "ICameraSceneNode.h"
#include "SViewFrustum.h"
#include "ISceneManager.h"

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
        f32 zoomSpeed,
        f32 translationSpeed)
    : CursorControl(cursor), Zooming(false), Rotating(false), Moving(false),
    Translating(false), ZoomSpeed(zoomSpeed), RotateSpeed(rotateSpeed), TranslateSpeed(translationSpeed),
    CurrentZoom(70.0f), RotX(0.0f), RotY(0.0f), OldCamera(0), MousePos(0.5f, 0.5f)
{
    #ifdef _DEBUG
    setDebugName("CSceneNodeAnimatorCameraNero");
    #endif

    if (CursorControl)
    {
        CursorControl->grab();
        MousePos = CursorControl->getRelativePosition();
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
    if (event.EventType != EET_MOUSE_INPUT_EVENT &&
        event.EventType != EET_KEY_INPUT_EVENT)
        return false;

    if (event.EventType == EET_KEY_INPUT_EVENT) {
        switch (event.KeyInput.Key) {

        }
        for (u32 i=0; i<KeyMap.size(); ++i) {
        	if (KeyMap[i].keycode == event.KeyInput.Key) {
        		CursorKeys[KeyMap[i].action] = event.KeyInput.PressedDown;
        	}
        }
    }

    if (event.EventType == EET_MOUSE_INPUT_EVENT) {
        switch(event.MouseInput.Event)
            {
            case EMIE_LMOUSE_PRESSED_DOWN:
                MouseKeys[0] = true;
                break;
            case EMIE_RMOUSE_PRESSED_DOWN:
                MouseKeys[2] = true;
                break;
            case EMIE_MMOUSE_PRESSED_DOWN:
                MouseKeys[1] = true;
                break;
            case EMIE_LMOUSE_LEFT_UP:
                MouseKeys[0] = false;
                break;
            case EMIE_RMOUSE_LEFT_UP:
                MouseKeys[2] = false;
                break;
            case EMIE_MMOUSE_LEFT_UP:
                MouseKeys[1] = false;
                break;
            case EMIE_MOUSE_MOVED:
                MousePos = CursorControl->getRelativePosition();
                break;
            case EMIE_MOUSE_WHEEL:
            case EMIE_LMOUSE_DOUBLE_CLICK:
            case EMIE_RMOUSE_DOUBLE_CLICK:
            case EMIE_MMOUSE_DOUBLE_CLICK:
            case EMIE_LMOUSE_TRIPLE_CLICK:
            case EMIE_RMOUSE_TRIPLE_CLICK:
            case EMIE_MMOUSE_TRIPLE_CLICK:
            case EMIE_COUNT:
                return false;
            }
        return true;
    }
}


//! OnAnimate() is called just before rendering the whole scene.
//! nodes may calculate or store animations here, and may do other useful things,
//! dependent on what they are.
void CSceneNodeAnimatorCameraNero::animateNode(ISceneNode *node, u32 timeMs)
{
    //Alt + LM = Rotate around camera pivot
    //Alt + LM + MM = Dolly forth/back in view direction (speed % distance camera pivot - max distance to pivot)
    //Alt + MM = Move on camera plane (Screen center is about the mouse pointer, depending on move speed)

    if (!node || node->getType() != ESNT_CAMERA)
        return;

    ICameraSceneNode* camera = static_cast<ICameraSceneNode*>(node);

    // If the camera isn't the active camera, and receiving input, then don't process it.
    if(!camera->isInputReceiverEnabled())
        return;

    scene::ISceneManager * smgr = camera->getSceneManager();
    if(smgr && smgr->getActiveCamera() != camera)
        return;

    if (OldCamera != camera)
    {
        OldTarget = camera->getTarget();
        OldCamera = camera;
        LastCameraTarget = OldTarget;
    }
    else
    {
        OldTarget += camera->getTarget() - LastCameraTarget;
    }

    core::vector3df target = camera->getTarget();

    f32 nRotX = RotX;
    f32 nRotY = RotY;
    f32 nZoom = CurrentZoom;

    if ( (isMouseKeyDown(0) && isMouseKeyDown(2)) || isMouseKeyDown(1) )
    {
        if (!Zooming)
        {
            ZoomStart = MousePos;
            Zooming = true;
            nZoom = CurrentZoom;
        }
        else
        {
            const f32 targetMinDistance = 0.1f;
            nZoom += (ZoomStart.X - MousePos.X) * ZoomSpeed;

            if (nZoom < targetMinDistance) // jox: fixed bug: bounce back when zooming to close
                nZoom = targetMinDistance;
        }
    }
    else if (Zooming)
    {
        const f32 old = CurrentZoom;
        CurrentZoom = CurrentZoom + (ZoomStart.X - MousePos.X ) * ZoomSpeed;
        nZoom = CurrentZoom;

        if (nZoom < 0)
            nZoom = CurrentZoom = old;
        Zooming = false;
    }

    // Translation ---------------------------------

    core::vector3df translate(OldTarget), upVector(camera->getUpVector());

    core::vector3df tvectX = Pos - target;
    tvectX = tvectX.crossProduct(upVector);
    tvectX.normalize();

    const SViewFrustum* const va = camera->getViewFrustum();
    core::vector3df tvectY = (va->getFarLeftDown() - va->getFarRightDown());
    tvectY = tvectY.crossProduct(upVector.Y > 0 ? Pos - target : target - Pos);
    tvectY.normalize();

    if (isMouseKeyDown(2) && !Zooming)
    {
        if (!Translating)
        {
            TranslateStart = MousePos;
            Translating = true;
        }
        else
        {
            translate +=  tvectX * (TranslateStart.X - MousePos.X)*TranslateSpeed +
                          tvectY * (TranslateStart.Y - MousePos.Y)*TranslateSpeed;
        }
    }
    else if (Translating)
    {
        translate += tvectX * (TranslateStart.X - MousePos.X)*TranslateSpeed +
                     tvectY * (TranslateStart.Y - MousePos.Y)*TranslateSpeed;
        OldTarget = translate;
        Translating = false;
    }

    // Rotation ------------------------------------

    if (isMouseKeyDown(0) && !Zooming)
    {
        if (!Rotating)
        {
            RotateStart = MousePos;
            Rotating = true;
            nRotX = RotX;
            nRotY = RotY;
        }
        else
        {
            nRotX += (RotateStart.X - MousePos.X) * RotateSpeed;
            nRotY += (RotateStart.Y - MousePos.Y) * RotateSpeed;
        }
    }
    else if (Rotating)
    {
        RotX += (RotateStart.X - MousePos.X) * RotateSpeed;
        RotY += (RotateStart.Y - MousePos.Y) * RotateSpeed;
        nRotX = RotX;
        nRotY = RotY;
        Rotating = false;
    }

    // Set Pos ------------------------------------

    target = translate;

    Pos.X = nZoom + target.X;
    Pos.Y = target.Y;
    Pos.Z = target.Z;

    Pos.rotateXYBy(nRotY, target);
    Pos.rotateXZBy(-nRotX, target);

    // Rotation Error ----------------------------

    // jox: fixed bug: jitter when rotating to the top and bottom of y
    upVector.set(0,1,0);
    upVector.rotateXYBy(-nRotY);
    upVector.rotateXZBy(-nRotX+180.f);

    camera->setPosition(Pos);
    camera->setTarget(target);
    camera->setUpVector(upVector);
    LastCameraTarget = camera->getTarget();
}


bool CSceneNodeAnimatorCameraNero::isMouseKeyDown(s32 key)
{
    return MouseKeys[key];
}


void CSceneNodeAnimatorCameraNero::allKeysUp()
{
    for (s32 i=0; i<3; ++i)
        MouseKeys[i] = false;
}


//! Sets the rotation speed
void CSceneNodeAnimatorCameraNero::setRotateSpeed(f32 speed)
{
    RotateSpeed = speed;
}


//! Sets the movement speed
void CSceneNodeAnimatorCameraNero::setMoveSpeed(f32 speed)
{
    TranslateSpeed = speed;
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
    return TranslateSpeed;
}


//! Gets the zoom speed
f32 CSceneNodeAnimatorCameraNero::getZoomSpeed() const
{
    return ZoomSpeed;
}

ISceneNodeAnimator* CSceneNodeAnimatorCameraNero::createClone(ISceneNode* node, ISceneManager* newManager)
{
    CSceneNodeAnimatorCameraNero * newAnimator =
        new CSceneNodeAnimatorCameraNero(CursorControl, RotateSpeed, ZoomSpeed, TranslateSpeed);
    return newAnimator;
}

} // end namespace
} // end namespace

