//--------------------------------------------------------
// OpenNero : GuiComboBox
//  combo box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiComboBox.h"
#include "scripting/scripting.h"

namespace OpenNero
{    
     // An a text item to the box
    int32_t GuiComboBox::addItem( const std::string&  text )
    {
        return addItem( boost::lexical_cast<std::wstring>( text.c_str() ) );
    }

    int32_t GuiComboBox::addItem( const std::wstring& text )
    {
        Assert( getGuiElement() );

        // save the entry
        mEntries.push_back(text);

        // tell it to the irr gui
        IGuiComboBox_IPtr cbox = boost::static_pointer_cast<IGUIComboBox>( getGuiElement() );
        return cbox->addItem( mEntries.back().c_str() );
    }

    PYTHON_BINDER( GuiComboBox )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiComboBox);
        typedef int32_t (GuiComboBox::*AddItemPtr)( const std::string& );

        // export the combo box to python
        class_<GuiComboBox, noncopyable>( "GuiComboBox", "A basic gui combo box", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiComboBox)

            .def("addItem", (AddItemPtr)&GuiComboBox::addItem, "Add an item to the combo box", "addItem(myItemDescString)" )
        ;
    }
}
