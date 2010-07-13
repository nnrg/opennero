//--------------------------------------------------------
// OpenNero : GuiCheckBox
//  an Check box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiCheckBox.h"
#include "scripting/scripting.h"

namespace OpenNero
{
    
    void GuiCheckBox::setChecked( bool checked )
    {
        Assert( getGuiElement() );
        
        IGuiCheckBox_IPtr cbox = boost::static_pointer_cast<IGUICheckBox>( getGuiElement() );
        cbox->setChecked( checked );
    }
    
    bool GuiCheckBox::isChecked()
    {
        Assert( getGuiElement() );
        
        IGuiCheckBox_IPtr cbox = boost::static_pointer_cast<IGUICheckBox>( getGuiElement() );
        return cbox->isChecked();
    }

    PYTHON_BINDER( GuiCheckBox )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiCheckBox);
        
        class_<GuiCheckBox, noncopyable>( "GuiCheckBox", "A basic gui Check box", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiCheckBox)
            
            .add_property( "checked", &GuiCheckBox::isChecked, &GuiCheckBox::setChecked, "Whether or not the checkbox is checked" )
            ;
    }
}
