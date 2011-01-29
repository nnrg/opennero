//--------------------------------------------------------
// OpenNero : GuiButton
//  a button abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_BUTTON_H_
#define _GAME_GUI_BUTTON_H_

#include "gui/GuiBase.h"
#include "core/BoostCommon.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( GuiButton );
	/// @endcond

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
	class GuiButton : public PyGuiBase
    {
    public:

        /// Set the image path of this button and try to load all related images.
        /// @param imgBase the base image path to use
        void setImages( const std::string& imgBase );
    };
}

#endif // _GAME_GUI_BUTTON_H_

