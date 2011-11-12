from OpenNero import *

"""

This module provides a series of helpful utility
methods for the creation of gui elements

"""

def gui_init_base( element, name, position, extent ):
    """
    This is the basic utility init method.
    @param element the element to initialize
    @param name the name of the element
    @param position the position to place the element (Pos2i)
    @param extent the extent of the element (Pos2i)
    @return the initialized element
    """
    element.name = name
    element.position = position
    element.extent = extent
    return element

def create_image( guiMan, name, position, extent, imagePath ):
    """
    @param imagePath the path to the image file to load
    """
    e = gui_init_base( guiMan.createElement('image'), name, position, extent )
    e.setImage(imagePath)
    return e

def create_button( guiMan, name, position, extent, imageBase ):
    """
    @param imageBase the base of the set of three images to load for the button (normal, depressed, hover)
    """
    e = gui_init_base( guiMan.createElement('button'), name, position, extent )
    if imageBase != '':
        e.setImages(imageBase)
    return e

def create_combo_box( guiMan, name, position, extent ):
    return gui_init_base( guiMan.createElement('combo box'), name, position, extent )

def create_context_menu( guiMan, name, position ):
    e = guiMan.createElement('context menu')
    e.name = name
    e.position = position
    return e

def create_text( guiMan, name, position, extent, text ):
    """
    @param text the text for the element to display
    """
    e = gui_init_base( guiMan.createElement('text'), name, position, extent )
    e.text = text
    return e

def create_window( guiMan, name, position, extent, title ):
    """
    @param title the title to display on the window
    """
    e = gui_init_base( guiMan.createElement('window'), name, position, extent )
    e.text = title
    return e

def create_edit_box( guiMan, name, position, extent, text ):
    """
    @param text the text to initially display inside the edit box
    """
    e = gui_init_base( guiMan.createElement('edit box'), name, position, extent )
    e.text = text
    return e
    
def create_check_box( guiMan, name, position, extent, checked = False ):
    """
    @param checked the initial state of the checkbox
    """
    e = gui_init_base( guiMan.createElement('check box'), name, position, extent )
    e.checked = checked
    return e

def create_scroll_bar( guiMan, name, position, extent, horizontal = False ):
    """
    @param horizontal - whether or not the bar is horizontal
    """
    if horizontal:
        e = gui_init_base( guiMan.createElement('horizontal scroll bar'), name, position, extent )
    else:
        e = gui_init_base( guiMan.createElement('scroll bar'), name, position, extent )
    return e
