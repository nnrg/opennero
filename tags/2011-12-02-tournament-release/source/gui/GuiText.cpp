//--------------------------------------------------------
// OpenNero : GuiEditBox
//  an edit box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiText.h"

namespace OpenNero
{	
    void GuiText::SetColor( const SColor& color )
    {
        Assert( getGuiElement() );
        IGuiText_IPtr text = boost::static_pointer_cast<IGUIStaticText>( getGuiElement() );
        text->setOverrideColor(color);
    }
     
    SColor GuiText::GetColor()
    {
        Assert( getGuiElement() );
        IGuiText_IPtr text = boost::static_pointer_cast<IGUIStaticText>( getGuiElement() );
        return text->getOverrideColor();
    }

    void GuiText::SetWordWrap( bool wrap )
    {
        Assert( getGuiElement() );
        IGuiText_IPtr text = boost::static_pointer_cast<IGUIStaticText>( getGuiElement() );
        text->setWordWrap(wrap);
    }

    bool GuiText::GetWordWrap()
    {
        Assert( getGuiElement() );
        IGuiText_IPtr text = boost::static_pointer_cast<IGUIStaticText>( getGuiElement() );
        return text->isWordWrapEnabled();
    }

}
