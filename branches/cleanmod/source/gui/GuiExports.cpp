#include "core/Common.h"
#include "core/BoostCommon.h"
#include "scripting/scriptIncludes.h"

#include "gui/GuiExports.h"

#include "gui/GuiButton.h"
#include "gui/GuiEditBox.h"
#include "gui/GuiText.h"
#include "gui/GuiManager.h"
#include "gui/GuiCheckBox.h"
#include "gui/GuiScrollBar.h"
#include "gui/GuiImage.h"
#include "gui/GuiComboBox.h"
#include "gui/GuiWindow.h"
#include "gui/GuiContextMenu.h"
#include "gui/GuiBase.h"

namespace b = boost;
namespace py = boost::python;

namespace OpenNero {
	namespace scripting {

        using namespace OpenNero;

        void ExportGuiButtonScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiButton);

            py::class_<GuiButton, noncopyable>( "GuiButton", "A basic gui button", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiButton)

                // export our button methods
                .def("setImages", &GuiButton::setImages,"Set the images to use for the button" )
                ;
        }

        void ExportGuiEditBoxScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiEditBox);

            py::class_<GuiEditBox, noncopyable>( "GuiEditBox", "A basic gui edit box", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiEditBox)

                .def( "setText", &GuiEditBox::setText, "Set text of an edit box")
                ;
        }

        void ExportGuiTextScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiText);

            class_<GuiText, noncopyable>( "GuiText", "A basic gui text object.", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiText)

                // export our button methods
                .add_property("color", &GuiText::GetColor, &GuiText::SetColor )
                .add_property("wordWrap", &GuiText::GetWordWrap, &GuiText::SetWordWrap )
                ;
        }

        void ExportGuiManagerScripts()
        {
            typedef GuiBasePtr (GuiManager::*GetElementPtr)( const std::string& );
            typedef void (GuiManager::*RemovePtr)( const std::string& );

            /// export the gui manager class
            class_<GuiManager,GuiManagerPtr>("GuiManager", "Manager of the gui elements", no_init)
                .def("removeAll",
                     &GuiManager::RemoveAll,
                     "Remove all gui elements from the manager", "removeAll()")
                .def("remove",
                     (RemovePtr)&GuiManager::Remove,
                     "Remove an individual element from the manager", "remove(guiName)")
                .def("getNumElements",
                     &GuiManager::getNumElements,
                     "Gets the number of elements managed", "getNumElements()")
                .def("setTransparency",
                     &GuiManager::setGuiTransparency,
                     "Sets the transparency of the gui elements", "setTransparency(floatVal_0_1)")
                .def("setFont",
                     &GuiManager::setFont,
                     "Sets the font file to use for text", "setFont(myFont.imageExt)")
                .def("getElement",
                     (GetElementPtr)&GuiManager::getElement,
                     "Get an element by its name.",
                     "getElement(elemNameStr)" )
                .def("createElement",
                     &GuiManager::createElement,
                     "Create a gui element", "createElement('type')" )
                .def("openFileChooserDialog",
                     &GuiManager::openFileChooserDialog,
                     "Open a dialog to choose a file.", "openFileChooserDialog('myDialog',modal?,python callback function)" )
                .def("isOpenFileChooserDialog",
                     &GuiManager::isOpenFileChooserDialog,
                     "Checks if a file chooser dialog is open.", "isOpenFileChooserDialog()" )
                ;
        }

        void ExportGuiCheckBoxScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiCheckBox);

            py::class_<GuiCheckBox, noncopyable>( "GuiCheckBox", "A basic gui Check box", no_init )
                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiCheckBox)
                .add_property( "checked",
                               &GuiCheckBox::isChecked, &GuiCheckBox::setChecked,
                               "Whether or not the checkbox is checked" )
                ;
        }


        void ExportGuiScrollBarScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiScrollBar);

            py::class_<GuiScrollBar, noncopyable>( "GuiScrollBar", "A basic scroll bar", no_init )

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

        void ExportGuiImageScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiImage);

            py::class_<GuiImage, noncopyable>( "GuiImage", "A basic gui image", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiImage)

                // export our button methods
                .def("setImage",                &GuiImage::setImage,"Set the image to use" )
                .def("setEnableAlphaChannel",   &GuiImage::setEnableAlphaChannel, "Set whether or not to use the alpha channel")
                ;
        }

        void ExportGuiComboBoxScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiComboBox);
            typedef int32_t (GuiComboBox::*AddItemPtr)( const std::string& );

            // export the combo box to python
            py::class_<GuiComboBox, noncopyable>( "GuiComboBox", "A basic gui combo box", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiComboBox)

                .def("addItem", (AddItemPtr)&GuiComboBox::addItem, "Add an item to the combo box", "addItem(myItemDescString)" )
                .def("getSelected", &GuiComboBox::getSelected, "Get the currently selected item index", "getSelected()")
                ;
        }

        void ExportGuiWindowScripts()
        {

            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiWindow);

            py::class_<GuiWindow, noncopyable>( "GuiWindow", "A basic gui window", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiWindow)

                // Additional methods
                .def("setVisible",
                     &GuiWindow::setVisible,
                     "Set the visibility of this window" )
                .def("setVisibleCloseButton",
                     &GuiWindow::setVisibleCloseButton,
                     "Set the visibility of the close button" )
                ;
        }

        void ExportGuiContextMenuScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(GuiContextMenu);

            typedef void (GuiContextMenu::*AddSubItemPtr)( const std::string&, GuiBasePtr );

            typedef void (GuiContextMenu::*AddItemPtr)( const std::string&, GuiBasePtr );

            typedef void (GuiContextMenu::*AddItemPtr)( const std::string&, GuiBasePtr );

            py::class_<GuiContextMenu, noncopyable>( "GuiContextMenu", "A basic gui context menu", no_init )

                // Hack in our gui base methods
                _GUI_BASE_HACK_(GuiContextMenu)

                // export our button methods
                .def("addSeparator", &GuiContextMenu::addSeparator, "Add a separator to the menu.")
                .def("addSubItem", (AddSubItemPtr)&GuiContextMenu::AddSubItem, "Add a sub item to the menu.")
                .def("addItem", (AddItemPtr)&GuiContextMenu::AddItem, "Add an item to the menu.")
                ;
        }


        void ExportGuiBaseScripts()
        {
            // ptrs to special overloaded member methods
            _GUI_BASE_PRE_HACK_(PyGuiBase);

            // exprt the PyGuiBase class
            py::class_<PyGuiBase, noncopyable, GuiBasePtr>( "PyGuiBase", "A basic python gui element", no_init )
                _GUI_BASE_HACK_(PyGuiBase)
                ;
        }

        void ExportGuiScripts()
        {
            ExportGuiButtonScripts();
            ExportGuiEditBoxScripts();
            ExportGuiTextScripts();
            ExportGuiManagerScripts();
            ExportGuiCheckBoxScripts();
            ExportGuiScrollBarScripts();
            ExportGuiImageScripts();
            ExportGuiComboBoxScripts();
            ExportGuiWindowScripts();
            ExportGuiContextMenuScripts();
            ExportGuiBaseScripts();
        }
    }
}
