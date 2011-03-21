//--------------------------------------------------------
// OpenNero : GuiCheckBox
// check box abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_CHECK_BOX_H_
#define _GAME_GUI_CHECK_BOX_H_

#include "gui/GuiBase.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL( GuiCheckBox );
    /// @endcond

    /// An edit box to enter text into
	class GuiCheckBox : public PyGuiBase
    {
    public:
        /// get the checked state of the checkbox
        bool isChecked();
        /// set the checked state of the checkbox
        void setChecked(bool checked);
        /// This element_can_ gain focus
        bool CanGainFocus() { return true; }
    };
}

#endif // _GAME_GUI_CHECK_BOX_H_

