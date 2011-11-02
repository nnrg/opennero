//--------------------------------------------------------
// OpenNero : GuiScrollBar
//  a scroll bar abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_SCROLL_BAR_H_
#define _GAME_GUI_SCROLL_BAR_H_

#include "gui/GuiBase.h"
#include "core/BoostCommon.h"

namespace OpenNero
{   
    BOOST_PTR_DECL( GuiScrollBar );

    /**
	 * A container for a gui button element. Buttons contain three states: normal, hover,
	 * and depressed. Normal is when no action is occuring, hover is when the mouse moves
	 * over, and depressed is when the user is pressing down on it. The image state of the
	 * button is represented by three button images with a base path. For example, if I specify
	 * the base button base of "mybutton" the system will search for .png images with the following
	 * names:
	 *			mybutton_n.png
	 *			mybutton_h.png
	 *			mybutton_d.png
	 * and will save the ones it can successfully load.
	*/
	class GuiScrollBar : public PyGuiBase
    { 
    public:
        void setLargeStep(s32 step);
        void setMax(s32 max);
        void setPos(s32 pos);
        s32 getPos(void);
        void setSmallStep(s32 step);

    };
}

#endif // _GAME_GUI_SCROLL_BAR_H_

