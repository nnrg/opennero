//--------------------------------------------------------
// OpenNero : GuiContextWindow
//  a context window abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_CONTEXT_MENU_H_
#define _GAME_GUI_CONTEXT_MENU_H_

#include "gui/GuiBase.h"
#include <string>

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( GuiContextMenu );
	/// @endcond

    /// A context menu (sometimes called a title bar) that
    /// has menus and submenus which can present buttons to
    /// the user to direct navigation
	class GuiContextMenu : public PyGuiBase
    {
    public:

        /// Add a separator to the context menu
		void addSeparator();

        /// Add a subitem
        void AddSubItem( const std::string&  title, GuiBasePtr actionContainer );
        /// Add a subitem using a wide string
        void AddSubItem( const std::wstring& title, GuiBasePtr actionContainer );

        /// Add an item        
        void AddItem( const std::string&  title, GuiBasePtr actionContainer );
        void AddItem( const std::wstring& title, GuiBasePtr actionContainer );
    };
}

#endif // _GAME_GUI_CONTEXT_MENU_H_

