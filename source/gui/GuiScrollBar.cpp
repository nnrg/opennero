//--------------------------------------------------------
// OpenNero : GuiScrollBar
//  a scroll bar abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiScrollBar.h"
#include "game/factories/IrrFactory.h"
#include "game/factories/SimFactory.h"
#include "scripting/scripting.h"

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

    PYTHON_BINDER( GuiScrollBar )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiScrollBar);

        class_<GuiScrollBar, noncopyable>( "GuiScrollBar", "A basic scroll bar", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiScrollBar)

            // export our scroll bar methods
            .def("setLargeStep",&GuiScrollBar::setLargeStep, "Sets the large step")
            .def("setMax",&GuiScrollBar::setMax, "Sets the max value of the slider")
            .def("setPos",&GuiScrollBar::setPos, "Sets the position of the slider")
            .def("getPos",&GuiScrollBar::getPos, "Gets the position of the slider")
            .def("setSmallStep",&GuiScrollBar::setSmallStep, "Sets the small step")
        ;
    }
}
