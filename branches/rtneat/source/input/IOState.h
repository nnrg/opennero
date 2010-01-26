//--------------------------------------------------------
// OpenNero : IOState
//  Tracks the current status of input
//--------------------------------------------------------

#ifndef _INPUT_IOSTATE_H_
#define _INPUT_IOSTATE_H_

#include "core/ONTypes.h"
#include "input/IOEnums.h"
#include <vector>

namespace irr
{
    struct SEvent;
};

namespace OpenNero
{
    /// The IOState class is responsible for tracking all input coming from any source in the game.
    /// The class should be a complete abstract for the entire input state that can be polled at
    /// any time with various input related queries. The class follows a methodology where there
    /// is a configuration for the current state and a configuration for the previous state and
    /// it uses these two configurations to tell if status has changed for a given input device.
    class IOState
    {
    public:

        /// where is this input coming from?
        enum TrackingSource
        {
            kTrackingSource_UserInput,          ///< input coming directly from the user

            // TODO : Add recording support
            kTrackingSource_RecordedInput       ///< input coming from some previous recording
        };

    public:

        IOState();

        // set where we are getting our input from
        void SetTrackingSource( TrackingSource source );

        // event handler for irrlicht
        void onIrrEvent( const irr::SEvent& event );

        // keyboard status checks
        bool IsKeyHeld( KEY key ) const;
        bool WasKeyPressedLastFrame( KEY key ) const;
        bool WasKeyReleasedLastFrame( KEY key ) const;

        bool IsShiftDown() const;
        bool IsCtrlDown() const;

        // mouse status checks
        bool IsMouseButtonHeld( MOUSE_BUTTON button ) const;
        bool WasMouseButtonPressedLastFrame( MOUSE_BUTTON button ) const;
        bool WasMouseButtonReleasedLastFrame( MOUSE_BUTTON button ) const;

        void GetMousePosition( int32_t& x, int32_t& y ) const;
        void GetMouseDelta( int32_t& dx, int32_t& dy ) const;

        /// get the value of the mouse scroll
        inline float32_t GetMouseScrollValue() const { return mScrollVal; }

        /// get the delta of the mouse scroll
        inline float32_t GetMouseScrollDelta() const { return mScrollDelta; }

        // does any preparation needed for the next simulation frame
        void PrepareForNextSimFrame();

    public:

        // utility methods
        static bool IsModifierKey( KEY key );

    protected:

        // specialized handlers for events
        void HandleIrrKeyboardEvent( const irr::SEvent& event );
        void HandleIrrMouseEvent( const irr::SEvent& event );
        void HandleMouseButton( MOUSE_BUTTON button, bool newState );

    private:

        // TODO : Maybe this should use our BitVector class so we can quickly set all bits to zero
        typedef std::vector<bool> KeyState;
        typedef std::vector<bool> MouseButtonState;
        typedef int32_t           MousePos;
        typedef int32_t           MouseDelta;
        typedef float32_t         MouseScrollVal;
        typedef float32_t         MouseScrollDelta;

    private:

        KeyState            mCurrentKeyState;       ///< our current key configuration this sim frame
        KeyState            mPrevKeyState;          ///< the key configuration from last sim frame
        MouseButtonState    mCurrentMouseState;     ///< our current mouse state this sim frame
        MouseButtonState    mPrevMouseState;        ///< the mouse state from last sim frame
        MousePos            mPosX, mPosY;           ///< the mouse position
        MouseDelta          mDeltaX, mDeltaY;       ///< the mouse delta
        MouseScrollVal      mScrollVal;             ///< the mouse scroll value
        MouseScrollDelta    mScrollDelta;           ///< the mouse scroll delta

        bool                mShiftPressed;          ///< is a shift key pressed?
        bool                mCtrlPressed;           ///< is a ctrl key pressed?

        TrackingSource      mTrackingSource;        ///< the source of the input
    };

}; // end OpenNero


#endif // _INPUT_IOSTATE_H_
