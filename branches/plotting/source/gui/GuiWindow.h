//--------------------------------------------------------
// OpenNero : GuiWindow
//  a window abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_WINDOW_H_
#define _GAME_GUI_WINDOW_H_

#include "gui/GuiBase.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL( GuiWindow );
    /// @endcond
    
    /// A Window Container
	class GuiWindow : public PyGuiBase
    { 
    private:
        static const uint32_t kTitleBarHeight = 20;

    public:

        /// Add a child and place it relative to ourself
        /// @param child the child element to add to our hierarchy
        void addChild( GuiBasePtr child );

    protected:
        /// Can this element gain focus?
        bool Default_CanGainFocus() { return true; }
    };
}

#endif // _GAME_GUI_WINDOW_H_

