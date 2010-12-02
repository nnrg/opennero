//--------------------------------------------------------
// OpenNero : GuiEditBox
//  an edit box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiEditBox.h"
#include "scripting/scripting.h"

namespace OpenNero
{
    
    void GuiEditBox::setText( const std::string& text )
    {
        Assert( getGuiElement() );
        
        std::wstring wText = boost::lexical_cast<std::wstring>( text.c_str());
        
        IGuiEditBox_IPtr ebox = boost::static_pointer_cast<IGUIEditBox>( getGuiElement() );
        ebox->setText( wText.c_str() );
    
    }
    
    PYTHON_BINDER( GuiEditBox )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiEditBox);
        
        class_<GuiEditBox, noncopyable>( "GuiEditBox", "A basic gui edit box", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiEditBox)
            
            .def( "setText", &GuiEditBox::setText, "Set text of an edit box")
            ;
    }
}
