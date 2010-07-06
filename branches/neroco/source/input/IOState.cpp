//--------------------------------------------------------
// OpenNero : IOState
//  Tracks the current status of input
//--------------------------------------------------------

#include "core/Common.h"
#include "input/IOState.h"
#include "core/IrrUtil.h"
#include "core/Error.h"

namespace OpenNero 
{
    using namespace irr;

    /// ctor - initializes the bit arrays to a size - they should _NEVER_ reallocate
    IOState::IOState()
        : mCurrentKeyState( KEY_KEY_CODES_COUNT, false )
        , mPrevKeyState( KEY_KEY_CODES_COUNT, false )        
        , mCurrentMouseState( MOUSE_BUTTONS_COUNT, false )
        , mPrevMouseState( MOUSE_BUTTONS_COUNT, false )

        , mPosX(0)
        , mPosY(0)
        , mDeltaX(0)
        , mDeltaY(0)
        , mScrollVal(0)
        , mScrollDelta(0)

        , mShiftPressed(false)
        , mCtrlPressed(false)

        , mTrackingSource( kTrackingSource_UserInput )
    {}

    /// Set the source of our tracking of input
    /// @param source the desired source
    void IOState::SetTrackingSource( TrackingSource source )
    {
        mTrackingSource = source;
    }

    /// Handler for Irrlicht events
    /// @param event an irrlicht created event
    void IOState::onIrrEvent( const irr::SEvent& event )
    {
        switch( event.EventType )
        {
            case EET_KEY_INPUT_EVENT:   HandleIrrKeyboardEvent(event); break;
            case EET_MOUSE_INPUT_EVENT: HandleIrrMouseEvent(event); break;
            default: break;
        }
    }

    /// Is the given key held down?
    bool IOState::IsKeyHeld( KEY key ) const
    {
        Assert( key < KEY_KEY_CODES_COUNT );
        Assert( mCurrentKeyState.size() == KEY_KEY_CODES_COUNT );
        return mCurrentKeyState[key];
    }

    /// Was the given key pressed between last frame and this frame (ie, not down last frame, down this frame)
    bool IOState::WasKeyPressedLastFrame( KEY key ) const
    {
        Assert( key < KEY_KEY_CODES_COUNT );
        Assert( mCurrentKeyState.size() == KEY_KEY_CODES_COUNT );
        Assert( mPrevKeyState.size() == KEY_KEY_CODES_COUNT );
        return mCurrentKeyState[key] && !mPrevKeyState[key];
    }

    /// Was the given key released between last frame and this frame
    bool IOState::WasKeyReleasedLastFrame( KEY key ) const
    {
        Assert( key < KEY_KEY_CODES_COUNT );
        Assert( mCurrentKeyState.size() == KEY_KEY_CODES_COUNT );
        Assert( mPrevKeyState.size() == KEY_KEY_CODES_COUNT );
        return !mCurrentKeyState[key] && mPrevKeyState[key];
    }

    /// Is the shift key being pressed?
    bool IOState::IsShiftDown() const
    {
        return mShiftPressed;
    }
    
    /// Is the ctrl key being pressed?
    bool IOState::IsCtrlDown() const
    {
        return mCtrlPressed;
    }

    /// Is the given mouse button held down?
    bool IOState::IsMouseButtonHeld( MOUSE_BUTTON button ) const
    {
        Assert( button < MOUSE_BUTTONS_COUNT );
        Assert( mCurrentMouseState.size() == MOUSE_BUTTONS_COUNT );
        Assert( mPrevMouseState.size() == MOUSE_BUTTONS_COUNT );
        return mCurrentMouseState[button];
    }

    /// Was the given mouse button pressed down between last frame and this frame?
    bool IOState::WasMouseButtonPressedLastFrame( MOUSE_BUTTON button ) const
    {
        Assert( button < MOUSE_BUTTONS_COUNT );
        Assert( mCurrentMouseState.size() == MOUSE_BUTTONS_COUNT );
        Assert( mPrevMouseState.size() == MOUSE_BUTTONS_COUNT );
        return mCurrentMouseState[button] && !mPrevMouseState[button];
    }
      
    /// Was the given mouse button pressed down between last frame and this frame?
    bool IOState::WasMouseButtonReleasedLastFrame( MOUSE_BUTTON button ) const
    {
        Assert( button < MOUSE_BUTTONS_COUNT );
        Assert( mCurrentMouseState.size() == MOUSE_BUTTONS_COUNT );
        Assert( mPrevMouseState.size() == MOUSE_BUTTONS_COUNT );
        return !mCurrentMouseState[button] && mPrevMouseState[button];
    }

    /// Get the current mouse position
    /// @param x,y the destination variables
    void IOState::GetMousePosition( int32_t& x, int32_t& y ) const
    {
        x = mPosX;
        y = mPosY;
    }

    /// Get the mouse movement delta of the mouse
    /// @param dx,dy the destination variables
    void IOState::GetMouseDelta( int32_t& dx, int32_t& dy ) const
    {
        dx = mDeltaX;
        dy = mDeltaY;
    }

    /// Determine if the key is one of our special modifier keys
    /// @param key the key to test
    /// @return true if it is a modifier key, false otherwise
    bool IOState::IsModifierKey( KEY key )
    {
        return key == KEY_SHIFT || key == KEY_CONTROL;
    }

    /// specialized keyboard handler, update the current key state
    /// @param event the irrlicht keyboard event
    void IOState::HandleIrrKeyboardEvent( const irr::SEvent& event )
    {   
        if( mTrackingSource == kTrackingSource_UserInput )
        {
            Assert( event.EventType == EET_KEY_INPUT_EVENT );
            Assert( event.KeyInput.Key < KEY_KEY_CODES_COUNT );
            Assert( mCurrentKeyState.size() == KEY_KEY_CODES_COUNT );
            Assert( mPrevKeyState.size() == KEY_KEY_CODES_COUNT );

            // save modifier state
            mShiftPressed = event.KeyInput.Shift;
            mCtrlPressed  = event.KeyInput.Control;

            // do not log if it is a modifier key
            if( IsModifierKey( (KEY)event.KeyInput.Key ) )
                return;
            
            // switch the key states            
            mCurrentKeyState[ event.KeyInput.Key ] = event.KeyInput.PressedDown;
        }
    }

    /// Specialzed handler for irrlicht mouse events - updates mouse state
    void IOState::HandleIrrMouseEvent( const irr::SEvent& event )
    {
        if( mTrackingSource == kTrackingSource_UserInput )
        {
            Assert( event.EventType == EET_MOUSE_INPUT_EVENT );

            // update the mouse position
            mDeltaX = event.MouseInput.X - mPosX;
            mDeltaY = event.MouseInput.Y - mPosY;
            mPosX   = event.MouseInput.X;
            mPosY   = event.MouseInput.Y;

            // update the button state
            switch( event.MouseInput.Event )
            {
                case EMIE_MOUSE_WHEEL:
                    // the Wheel parameter seems to be the delta instead of a 'position'
                    mScrollVal += event.MouseInput.Wheel;
                    mScrollDelta = event.MouseInput.Wheel;                    
                    break;
                
                case EMIE_LMOUSE_PRESSED_DOWN:
                    HandleMouseButton( MOUSE_LBUTTON, true );                    
                    break;

                case EMIE_RMOUSE_PRESSED_DOWN:
                    HandleMouseButton( MOUSE_RBUTTON, true );
                    break;

                case EMIE_MMOUSE_PRESSED_DOWN:
                    HandleMouseButton( MOUSE_MBUTTON, true );
                    break;

                case EMIE_LMOUSE_LEFT_UP:
                    HandleMouseButton( MOUSE_LBUTTON, false );
                    break;

                case EMIE_RMOUSE_LEFT_UP:
                    HandleMouseButton( MOUSE_RBUTTON, false );
                    break;

                case EMIE_MMOUSE_LEFT_UP:
                    HandleMouseButton( MOUSE_MBUTTON, false );
                    break;
                    
                default:
                    break;
            }
        }
    }

    /// Moves the current state of the mouse to the previous and stores the new state
    /// @param button the button state to update
    /// @param newState the new state to store as current
    void IOState::HandleMouseButton( MOUSE_BUTTON button, bool newState )
    {
        mPrevMouseState[button] = mCurrentMouseState[button];
        mCurrentMouseState[button] = newState;
    }

    /// Prepare our state for the next frame. This includes swapping the key
    /// state buffers such that the configuration for this frame becomes the
    /// configuration for next and all keys are assumed up until irr tells us
    void IOState::PrepareForNextSimFrame()
    {
        // ok, so irrlicht doesn't inform us when a key is released
        // but it will tell us if it is held down. So the idea is to
        // move the current buffer to the prev buffer and then clear the current
        std::swap( mPrevKeyState, mCurrentKeyState );
        std::fill( mCurrentKeyState.begin(), mCurrentKeyState.end(), false );

        // we also need to clear out the mouse delta because if we do not move
        // the mouse, irr won't send a new mouse event and thus the delta will
        // stay greater than 0 and mouse event will be fired...
        mDeltaX = mDeltaY = 0;
        mScrollDelta = 0.0f;

        // we also need to undo 1 frame only mouse states such as pressed and release.
        //          [ what it was ]                    -> [ what we change it to ]
        // press   = false last frame, true this frame -> true last frame, true this frame (Note: press turns into hold)
        // release = true last frame, false this frame -> false last frame, false this frame (Note: release turns into nothing)
        Assert( mCurrentMouseState.size() == mPrevMouseState.size() );
        Assert( (uint32_t)mCurrentMouseState.size() == MOUSE_BUTTONS_COUNT );

        for( uint32_t i = 0; i < mCurrentMouseState.size(); ++i )
        {
            // was it pressed?
            if( !mPrevMouseState[i] && mCurrentMouseState[i] )
                mPrevMouseState[i] = true;

            // was it released?
            if( mPrevMouseState[i] && !mCurrentMouseState[i] )
                mPrevMouseState[i] = false;
        }
    }

}; // end OpenNero
