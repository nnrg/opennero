//--------------------------------------------------------
// OpenNero : NeroDrawNode
//  A node for drawing nero systems
//--------------------------------------------------------

#include "core/Common.h"
#include "render/NeroDrawNode.h"
#include "render/LineSet.h"

namespace OpenNero
{   
    /// Ctor - initialize the base call and our bbox
    NeroDrawNode::NeroDrawNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
        : irr::scene::ISceneNode(parent,mgr,id)        
    {
        // we don't want this node to ever be clipped so make a rediculously large bbox
        const float32_t kBoxSize = 10000;
        mBBox = core::aabbox3df(-kBoxSize, -kBoxSize, -kBoxSize, kBoxSize, kBoxSize, kBoxSize );
    }

    void NeroDrawNode::OnRegisterSceneNode()
    {   
        SceneManager->registerNodeForRendering(this);
        ISceneNode::OnRegisterSceneNode();
    }
        
    /// render everything that nero needs with this node
    void NeroDrawNode::render()
    {
        // draw our lineset
        LineSet::instance().Render( SceneManager->getVideoDriver() );
    }
    
    const core::aabbox3df& NeroDrawNode::getBoundingBox() const
    {
        return mBBox;
    }    

};//end OpenNero
