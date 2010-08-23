//--------------------------------------------------------
// OpenNero : GuiContextWindow
//  a context window abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiContextMenu.h"
#include "scripting/scripting.h"

namespace OpenNero
{
    void GuiContextMenu::addSeparator()
	{
        Assert( getGuiElement() );
        IGuiContextMenu_IPtr context_menu = boost::static_pointer_cast<IGUIContextMenu>( getGuiElement() );
        context_menu->addSeparator();		
	}

    void GuiContextMenu::AddSubItem( const std::string&  title, GuiBasePtr actionContainer )
    {
        AddSubItem( boost::lexical_cast<std::wstring>(title.c_str()), actionContainer );
    }

    void GuiContextMenu::AddSubItem( const std::wstring& title, GuiBasePtr actionContainer )
    {
        // Explanation of how this works:
        //
        //  The actionContainer is assumed to have previously been registered with the system and
        //  initialized. This means that it has a valid id and has any response callback already set.
        //  The associate the id from the action container with the new menu element we created. So,
        //  when the user clicks on the submenu element, the gui manager find the id of the actionContainer
        //  and executes its overrides.

        Assert( getGuiElement() );
        IGuiContextMenu_IPtr context_menu = boost::static_pointer_cast<IGUIContextMenu>( getGuiElement() );

        // create the sub item
        // TODO : Do we need to store the title ?
        uint32_t c = context_menu->addItem( title.c_str(), -1, true, false );

        // get the submenu
        irr::gui::IGUIContextMenu* guiSubMenu = context_menu->getSubMenu(c);
        Assert( guiSubMenu );

        // add the item to the submenu.
        guiSubMenu->addItem( title.c_str(), actionContainer->getId() );
    }

    void GuiContextMenu::AddItem( const std::string&  title, GuiBasePtr actionContainer )
    {
        AddItem( boost::lexical_cast<std::wstring>(title.c_str()), actionContainer );
    }

    void GuiContextMenu::AddItem( const std::wstring& title, GuiBasePtr actionContainer )
    {
        // Explanation of how this works:
        //
        //  The actionContainer is assumed to have previously been registered with the system and
        //  initialized. This means that it has a valid id and has any response callback already set.
        //  The associate the id from the action container with the new menu element we created. So,
        //  when the user clicks on the menu element, the gui manager find the id of the actionContainer
        //  and executes its overrides.

        Assert( getGuiElement() );
        IGuiContextMenu_IPtr context_menu = boost::static_pointer_cast<IGUIContextMenu>( getGuiElement() );
        context_menu->addItem( title.c_str(), actionContainer->getId() );
    }

    PYTHON_BINDER( GuiContextMenu )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiContextMenu);

        typedef void (GuiContextMenu::*AddSubItemPtr)( const std::string&, GuiBasePtr );

        typedef void (GuiContextMenu::*AddItemPtr)( const std::string&, GuiBasePtr );

        typedef void (GuiContextMenu::*AddItemPtr)( const std::string&, GuiBasePtr );

        class_<GuiContextMenu, noncopyable>( "GuiContextMenu", "A basic gui context menu", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiContextMenu)

            // export our button methods
            .def("addSeparator", &GuiContextMenu::addSeparator, "Add a separator to the menu.")
            .def("addSubItem", (AddSubItemPtr)&GuiContextMenu::AddSubItem, "Add a sub item to the menu.")            
            .def("addItem", (AddItemPtr)&GuiContextMenu::AddItem, "Add an item to the menu.")            
        ;
    }
}
