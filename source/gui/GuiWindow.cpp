//--------------------------------------------------------
// OpenNero : GuiWindow
//  a window abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiWindow.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    void GuiWindow::addChild( GuiBasePtr child )
    {
        Assert( child );

        // let the parent work
        GuiBase::addChild(child);

        // offset to move it below the title bar
        Rect2i rect = child->getRelativeBounds();
        rect.LowerRightCorner += position2di( 0, kTitleBarHeight );
        rect.UpperLeftCorner  += position2di( 0, kTitleBarHeight );
       
        child->setRelativeBounds(rect);
    }


    PYTHON_BINDER( GuiWindow )
    {
        using namespace boost;
        using namespace boost::python;

        // ptrs to special overloaded member methods
        _GUI_BASE_PRE_HACK_(GuiWindow);

        class_<GuiWindow, noncopyable>( "GuiWindow", "A basic gui window", no_init )

            // Hack in our gui base methods
            _GUI_BASE_HACK_(GuiWindow)

            // Additional methods
            .def("setVisible",                          &GuiWindow::setVisible, "Set the visibility of this window" )
            .def("setVisibleCloseButton",               &GuiWindow::setVisibleCloseButton, "Set the visibility of the close button" )
            //.def("setDraggable",                        &GuiWindow::setDraggable, "Set whether the window can be dragged by the mouse" )
        ;
    }
}

