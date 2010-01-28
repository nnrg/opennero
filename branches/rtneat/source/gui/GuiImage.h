//--------------------------------------------------------
// OpenNero : GuiImage
//  an image abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_IMAGE_H_
#define _GAME_GUI_IMAGE_H_

#include "gui/GuiBase.h"

namespace OpenNero
{
    /// @cond
    BOOST_PTR_DECL( GuiImage );
    /// @endcond

    /// A Gui element for displaying a static image  
	class GuiImage : public PyGuiBase
    { 
    public:		

		// set a image
		void setImage( const std::string imagePath );

		/// set if we should use the alpha channel
		void setEnableAlphaChannel( bool enable );
    };
}

#endif // _GAME_GUI_IMAGE_H_

