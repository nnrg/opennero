//--------------------------------------------------------
// OpenNero : GuiScrollBar
//  a scroll bar abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiScrollBar.h"
#include "game/factories/IrrFactory.h"
#include "game/factories/SimFactory.h"

namespace OpenNero
{	
    void GuiScrollBar::setPos(s32 pos)
    {
        IGuiScrollBar_IPtr scrollbar = boost::static_pointer_cast<IGUIScrollBar>( getGuiElement() );
        
        assert(scrollbar);
        
        scrollbar->setPos(pos);
    }

    s32 GuiScrollBar::getPos(void)
    {
        IGuiScrollBar_IPtr scrollbar = boost::static_pointer_cast<IGUIScrollBar>( getGuiElement() );

        assert(scrollbar);

        return scrollbar->getPos();
    }

    void GuiScrollBar::setLargeStep(s32 step)
    {
        IGuiScrollBar_IPtr scrollbar = boost::static_pointer_cast<IGUIScrollBar>( getGuiElement() );

        assert(scrollbar);

        scrollbar->setLargeStep(step);
    }
    void GuiScrollBar::setSmallStep(s32 step)
    {
        IGuiScrollBar_IPtr scrollbar = boost::static_pointer_cast<IGUIScrollBar>( getGuiElement() );

        assert(scrollbar);

        scrollbar->setSmallStep(step);
    }

    void GuiScrollBar::setMax(s32 max)
    {
        IGuiScrollBar_IPtr scrollbar = boost::static_pointer_cast<IGUIScrollBar>( getGuiElement() );

        assert(scrollbar);

        scrollbar->setMax(max);
    }

}
