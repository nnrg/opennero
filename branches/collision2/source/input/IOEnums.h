//--------------------------------------------------------
// OpenNero : IOEnums
//  All the enumerations for input
//--------------------------------------------------------

#ifndef _INPUT_IOENUMS_H_
#define _INPUT_IOENUMS_H_

namespace OpenNero 
{
    /// Currently we only accept input via Irrlicht
    typedef irr::EKEY_CODE KEY;

    /// All mouse buttons
    enum MOUSE_BUTTON
    {
        MOUSE_LBUTTON = 0x0001,
        MOUSE_MBUTTON,
        MOUSE_RBUTTON,

        // pseudo buttons
        MOUSE_MOVE_X,
        MOUSE_MOVE_Y,
        MOUSE_SCROLL,

        MOUSE_BUTTONS_COUNT
    };

}; // end OpenNero


#endif // _INPUT_IOENUMS_H_
