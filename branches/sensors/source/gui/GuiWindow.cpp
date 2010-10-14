//--------------------------------------------------------
// OpenNero : GuiWindow
//  a window abstraction
//--------------------------------------------------------

#include "core/Common.h"
#include "gui/GuiWindow.h"

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


}

