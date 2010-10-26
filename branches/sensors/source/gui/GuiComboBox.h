//--------------------------------------------------------
// OpenNero : GuiComboBox
//  combo box abstraction
//--------------------------------------------------------

#ifndef _GAME_GUI_COMBO_BOX_H_
#define _GAME_GUI_COMBO_BOX_H_

#include "gui/GuiBase.h"
#include <vector>
#include <string>

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( GuiComboBox );
    /// @endcond

    /**
     * A combo box is a dropdown mention that presents a series of options to the user. The user can then
     * click on the appropriate option and this will be known as the "selected" element.
    */
	class GuiComboBox : public PyGuiBase
    {
    public:

        /// Add a text item to the box
        /// @param text the string to add to the box
        /// @return the index of the newly added string
        int32_t addItem( const std::string&  text );

        /// Add a text item to the box
        /// @param text the wide string to add to the box
        /// @return the index of the newly added string
        int32_t addItem( const std::wstring& text );
        
        int32_t getSelected(); 
        
    private:

        typedef std::vector<std::wstring> EntryVector;

	private:

        /// a collection of all the entries that we are forced to store.
        EntryVector     mEntries;
    };
}

#endif // _GAME_GUI_COMBO_BOX_H_

