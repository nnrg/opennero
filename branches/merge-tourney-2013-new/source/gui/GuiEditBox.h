//--------------------------------------------------------
// OpenNero : GuiEditBox
//  an edit box abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_EDIT_BOX_H_
#define _GAME_GUI_EDIT_BOX_H_

#include "gui/GuiBase.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL( GuiEditBox );
    /// @endcond

    /// An edit box to enter text into
	class GuiEditBox : public PyGuiBase
    {
    public:
    	/// set the text of this edit box
        void setText( const std::string& text );

        /// This element_can_ gain focus
        bool CanGainFocus() { return true; }
    };
}

#endif // _GAME_GUI_EDIT_BOX_H_

