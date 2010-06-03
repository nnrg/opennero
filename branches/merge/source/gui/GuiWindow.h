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

        /// Set the visibility of this window
        /// @param visible visibility of the window
        void setVisible(bool visible) { static_cast<IGUIWindow*>(getIrrGuiElement())->setVisible(visible); }

        /// Set the visibility of the close button of the window
        /// @param visible visibility of the close button
        void setVisibleCloseButton(bool visible) { static_cast<IGUIWindow*>(getIrrGuiElement())->getCloseButton()->setVisible(visible); }

        /// Set whether the window can be dragged by the mouse
        /// @param draggable whether the window can be dragged
        //void setDraggable(bool draggable) { static_cast<IGUIWindow*>(getIrrGuiElement())->setDraggable(draggable); }

    protected:
        /// Can this element gain focus?
        bool Default_CanGainFocus() { return true; }
    };
}

#endif // _GAME_GUI_WINDOW_H_

