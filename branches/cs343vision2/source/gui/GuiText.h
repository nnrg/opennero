//--------------------------------------------------------
// OpenNero : GuiText
//  text abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_TEXT_H_
#define _GAME_GUI_TEXT_H_

#include "gui/GuiBase.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL( GuiText );
    /// @endcond

    /// A piece of static text
	class GuiText : public PyGuiBase
    { 
    public:

        /// set the color of the text displayed
        /// @param color the color of the text
        void SetColor( const SColor& color );

        /// retrieve the text color
        /// @return a structure containing the current text color
        SColor GetColor();

        /// Setter for whether or not to word wrap text
        void SetWordWrap( bool wrap );
        /// getter for whether or not to word wrap text
        bool GetWordWrap();
    };
}

#endif // _GAME_GUI_TEXT_H_

