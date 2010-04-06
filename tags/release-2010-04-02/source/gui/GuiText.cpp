//--------------------------------------------------------
// OpenNero : GuiEditBox
//  an edit box abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiText.h"
#include "scripting/scripting.h"

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

    PYTHON_BINDER( GuiText )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiText);

        class_<GuiText, noncopyable>( "GuiText", "A basic gui text object.", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiText)

            // export our button methods
            .add_property( "color", &GuiText::GetColor, &GuiText::SetColor )
            .add_property("wordWrap", &GuiText::GetWordWrap, &GuiText::SetWordWrap )
        ;
    }
}
