// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CCameraNeroFPSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "os.h"
#include "Keycodes.h"

namespace irr
{
    namespace scene
    {

        const f32 MAX_VERTICAL_ANGLE = 88.0f;

        //! constructor
        CCameraNeroFPSceneNode::CCameraNeroFPSceneNode(
            ISceneNode* parent, 
            ISceneManager* mgr, 
            s32 id,
            const core::vector3df& position,
            const core::vector3df& lookat)
            : CCameraSceneNode(parent, mgr, id, position, lookat)
        {
#ifdef _DEBUG
            setDebugName("CCameraNeroFPSceneNode");
#endif
            recalculateViewArea();
        }


        //! destructor
        CCameraNeroFPSceneNode::~CCameraNeroFPSceneNode()
        {
        }


        //! It is possible to send mouse and key events to the camera. Most cameras
        //! may ignore this input, but camera scene nodes which are created for 
        //! example with scene::ISceneManager::addMayaCameraSceneNode or
        //! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
        //! for changing their position, look at target or whatever. 
        bool CCameraNeroFPSceneNode::OnEvent(const SEvent& event)
        {
            return false;
        }



        //! OnAnimate() is called just before rendering the whole scene.
        //! nodes may calculate or store animations here, and may do other useful things,
        //! dependent on what they are.
        void CCameraNeroFPSceneNode::OnAnimate(u32 timeMs)
        {
            core::list<ISceneNodeAnimator*>::Iterator ait = Animators.begin();
            for (; ait != Animators.end(); ++ait)
                (*ait)->animateNode(this, timeMs);

            updateAbsolutePosition();

            core::list<ISceneNode*>::Iterator it = Children.begin();
            for (; it != Children.end(); ++it)
                (*it)->OnAnimate(timeMs);
        }

        //! sets the look at target of the camera
        //! \param tgt Look at target of the camera.
        void CCameraNeroFPSceneNode::setTarget(const core::vector3df& tgt)
        {
            updateAbsolutePosition();
            core::vector3df vect = tgt - RelativeTranslation;
            vect = vect.getHorizontalAngle();
            RelativeRotation.X = vect.X;
            RelativeRotation.Y = vect.Y;

            if (RelativeRotation.X > MAX_VERTICAL_ANGLE)
                RelativeRotation.X -= 360.0f;

            Target.set(tgt);
        }

    } // end namespace
} // end namespace

