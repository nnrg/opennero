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

}
