//--------------------------------------------------------
// OpenNero : GuiEditBox
//  an edit box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiEditBox.h"

namespace OpenNero
{
    
    void GuiEditBox::setText( const std::string& text )
    {
        Assert( getGuiElement() );
        
        std::wstring wText = boost::lexical_cast<std::wstring>( text.c_str());
        
        IGuiEditBox_IPtr ebox = boost::static_pointer_cast<IGUIEditBox>( getGuiElement() );
        ebox->setText( wText.c_str() );
    
    }    
}
