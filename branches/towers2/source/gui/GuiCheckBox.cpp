//--------------------------------------------------------
// OpenNero : GuiCheckBox
//  an Check box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiCheckBox.h"

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

}
