//--------------------------------------------------------
// OpenNero : NeroDrawNode
//  A node for drawing nero systems
//--------------------------------------------------------

#ifndef _GAME_RENDER_NERO_DRAW_NODE_H_
#define _GAME_RENDER_NERO_DRAW_NODE_H_

#include "core/ONTypes.h"
#include "core/IrrUtil.h"

namespace OpenNero
{   
    using namespace irr;

    /// The NeroDrawNode is a special node in the irrlicht scene manager that draws
    /// any special subsystem in Nero that doesn't easily fit into the typical irrlicht
    /// system.
    class NeroDrawNode : public scene::ISceneNode
    {
    public:

        NeroDrawNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
        
        /// method overrides fro scene node
        void OnRegisterSceneNode();
        virtual void render();        
        /// returns the bounding box
        virtual const core::aabbox3df& getBoundingBox() const;

    private:

        // our bounding box
        core::aabbox3df mBBox;
    };

};//end OpenNero

#endif // _GAME_RENDER_NERO_DRAW_NODE_H_
