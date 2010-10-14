//--------------------------------------------------------
// OpenNero : IOMapping
//  a mapping of user input to game actions
//--------------------------------------------------------

#include "core/Common.h"
#include "core/IrrUtil.h"
#include "input/IOMapping.h"
#include "scripting/scripting.h"
#include "boost/pool/detail/singleton.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "core/HashMap.h"
#include "game/Kernel.h"
#include "game/SimContext.h"
#include <Keycodes.h>

namespace OpenNero 
{
    /// Check to see if our modifier requirements meet that of the io state
    /// @param inputCode the inputtype and modifiers flag
    /// @param ioState the current io state
    /// @return true if they match
    bool IOMapping::CheckModifierRequirements( InputCode inputCode, const IOState& ioState ) const
    {
        // get the modifier flags
        bool bNeedsShift = (inputCode & kInputMod_Shift) != 0;
        bool bNeedsCtrl  = (inputCode & kInputMod_Ctrl)  != 0;

        return (bNeedsShift == ioState.IsShiftDown() ) && ( bNeedsCtrl == ioState.IsCtrlDown() );
    }

    // convert a code to a key
    static KEY AsKey( InputCode code )
    {
        return static_cast<KEY>(code & IOMapping::kInputMod_Clear_Mask);
    }

    static MOUSE_BUTTON AsButton( InputCode code )
    {
        return static_cast<MOUSE_BUTTON>(AsKey(code));
    }

    PythonIOMapping::PythonIOMapping( InputSource inputSource, const InputRequirements& requirements, PythonIOMapping::Action pyAction )
        : mSource(inputSource)
        , mRequirements(requirements)        
        , mAction(pyAction)
    {}

    /// Check the io state to see if we can do our python action
    /// @param ioState the current io state
    /// @return true if the state is proper for action
    bool PythonIOMapping::CheckIORequirements( const IOState& ioState ) const
    {
        // get the input type and modifiers
        Assert( mRequirements.first.size() > 0 );        
        InputType type = mRequirements.second;

        InputCodeList::const_iterator citr = mRequirements.first.begin();
        InputCodeList::const_iterator cend = mRequirements.first.end();

        // all these loops are separated for performance, not clarity...
        switch( mSource )
        {
        case kInputSource_Keyboard:
            {   
                if( type == kInputType_Key_OnPress )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.WasKeyPressedLastFrame( AsKey(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )
                            return false;
                }
                else if( type == kInputType_Key_OnRelease )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.WasKeyReleasedLastFrame( AsKey(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )
                            return false;
                }
                else if( type == kInputType_Key_OnHold )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.IsKeyHeld( AsKey(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )
                            return false;
                }
            }
            return true;

        case kInputSource_Mouse:
            {
                if( type == kInputType_Mouse_OnPress )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.WasMouseButtonPressedLastFrame( AsButton(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )
                            return false;
                }
                else if( type == kInputType_Mouse_OnRelease )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.WasMouseButtonReleasedLastFrame( AsButton(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )
                            return false;
                }
                else if( type == kInputType_Mouse_OnHold )
                {
                    for( ; citr != cend; ++citr )
                        if( !ioState.IsMouseButtonHeld( AsButton(*citr) ) || !CheckModifierRequirements( *citr, ioState ) )                        
                            return false;
                }
                else if( type == kInputType_Mouse_Move_X )
                {
                    int32_t dx,dy;
                    ioState.GetMouseDelta(dx,dy);
                    return dx != 0;
                }

                else if( type == kInputType_Mouse_Move_Y )
                {
                    int32_t dx,dy;
                    ioState.GetMouseDelta(dx,dy);
                    return dy != 0;
                }

                else if( type == kInputType_Mouse_Scroll )
                {
                    return ioState.GetMouseScrollDelta() != 0.0f;
                }
            }
            return true;

        default:
            AssertMsg( false, "Unrecognized input source passed to python io mapping" );
            return false;
        }
    }
    
    /// Performed the mapped python action
    void PythonIOMapping::DoMappedAction()
    {
        TryCall(mAction);
    }
    
    /// Feed an event to the current io state to munch on
    /// @param event an input event from irrlicht
    void IOMap::GetUserInput( const irr::SEvent& event )
    {
        mIOState.onIrrEvent(event);
    }

    /// Poll the current state to see which mappings can take action
    void IOMap::ActOnUserInput()
    {
        // perform any actions needed with the current io state
        IOMappings::iterator itr = mIOMappings.begin();
        IOMappings::iterator end = mIOMappings.end();
        for( ; itr != end; ++itr )
        {
            Assert( itr->second );
            if( itr->second->CheckIORequirements(mIOState) )
                itr->second->DoMappedAction();
        }
        
        // clean up the io state for the next frame
        mIOState.PrepareForNextSimFrame();
    }

    /// @return the current position of the mouse
    Pos2i IOMap::GetMousePosition() const
    {
        int32_t x(0), y(0);
        mIOState.GetMousePosition(x,y);
        return Pos2i(x, y);
    }

    /// Add a mapping to the map
    /// @param key the key to insert at
    /// @param mappingPtr the mapping to map
    void IOMap::AddIOMapping( IOMapKey key, IOMappingPtr mappingPtr )
    {
        mIOMappings[key] = mappingPtr;
    }

    /// Clear all mappings from the map
    void IOMap::ClearMappings()
    {
        mIOMappings.clear();
    }

    // utility parsing procedures
    namespace
    {
        /// Convert a string to all lowercase
        std::string ToUpper( const std::string& str )
        {
            std::string strUp = str;
            std::transform( strUp.begin(), strUp.end(), strUp.begin(), ::toupper );
            return strUp;
        }

        /// Convert a string to all uppercase
        std::string ToLower( const std::string& str )
        {
            std::string strLow = str;
            std::transform( strLow.begin(), strLow.end(), strLow.begin(), ::tolower );
            return strLow;
        }

        typedef hash_map<std::string,KEY> StringToKeyMap;

        // some helpful macros
        #define IO_KEY_TO_STRING(key) #key
        #define IO_INSERT_KEY_TO_MAP(key) sStringToKeyMap[ IO_KEY_TO_STRING(key) ] = irr::key

        /// Get a key value from an input string        
        KEY GetKeyFromString( const std::string& str )
        {   
            static StringToKeyMap sStringToKeyMap;

            // only initialize the string map once
            if( sStringToKeyMap.size() == 0 )
            {
                IO_INSERT_KEY_TO_MAP( KEY_LBUTTON );
		        IO_INSERT_KEY_TO_MAP( KEY_RBUTTON );
		        IO_INSERT_KEY_TO_MAP( KEY_CANCEL  );
		        IO_INSERT_KEY_TO_MAP( KEY_MBUTTON );
		        IO_INSERT_KEY_TO_MAP( KEY_XBUTTON1 );
		        IO_INSERT_KEY_TO_MAP( KEY_XBUTTON2 );
		        IO_INSERT_KEY_TO_MAP( KEY_BACK     );
		        IO_INSERT_KEY_TO_MAP( KEY_TAB      );
		        IO_INSERT_KEY_TO_MAP( KEY_CLEAR    );
		        IO_INSERT_KEY_TO_MAP( KEY_RETURN   );
		        IO_INSERT_KEY_TO_MAP( KEY_SHIFT    );
		        IO_INSERT_KEY_TO_MAP( KEY_CONTROL  );
		        IO_INSERT_KEY_TO_MAP( KEY_MENU     );
		        IO_INSERT_KEY_TO_MAP( KEY_PAUSE    );
		        IO_INSERT_KEY_TO_MAP( KEY_CAPITAL  );
		        IO_INSERT_KEY_TO_MAP( KEY_KANA     );
		        IO_INSERT_KEY_TO_MAP( KEY_HANGUEL  );
		        IO_INSERT_KEY_TO_MAP( KEY_HANGUL   );
		        IO_INSERT_KEY_TO_MAP( KEY_JUNJA    );
		        IO_INSERT_KEY_TO_MAP( KEY_FINAL    );
		        IO_INSERT_KEY_TO_MAP( KEY_HANJA    );
		        IO_INSERT_KEY_TO_MAP( KEY_KANJI    );
		        IO_INSERT_KEY_TO_MAP( KEY_ESCAPE   );
		        IO_INSERT_KEY_TO_MAP( KEY_CONVERT  );
		        IO_INSERT_KEY_TO_MAP( KEY_NONCONVERT );
		        IO_INSERT_KEY_TO_MAP( KEY_ACCEPT     );
		        IO_INSERT_KEY_TO_MAP( KEY_MODECHANGE );
		        IO_INSERT_KEY_TO_MAP( KEY_SPACE      );
		        IO_INSERT_KEY_TO_MAP( KEY_PRIOR      );
		        IO_INSERT_KEY_TO_MAP( KEY_NEXT       );
		        IO_INSERT_KEY_TO_MAP( KEY_END        );
		        IO_INSERT_KEY_TO_MAP( KEY_HOME       );
		        IO_INSERT_KEY_TO_MAP( KEY_LEFT       );
		        IO_INSERT_KEY_TO_MAP( KEY_UP         );
		        IO_INSERT_KEY_TO_MAP( KEY_RIGHT      );
		        IO_INSERT_KEY_TO_MAP( KEY_DOWN       );
		        IO_INSERT_KEY_TO_MAP( KEY_SELECT     );
		        IO_INSERT_KEY_TO_MAP( KEY_PRINT      );
		        IO_INSERT_KEY_TO_MAP( KEY_EXECUT     );
		        IO_INSERT_KEY_TO_MAP( KEY_SNAPSHOT   );
		        IO_INSERT_KEY_TO_MAP( KEY_INSERT     );
		        IO_INSERT_KEY_TO_MAP( KEY_DELETE     );
		        IO_INSERT_KEY_TO_MAP( KEY_HELP       );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_0      );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_1      );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_2     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_3     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_4     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_5     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_6     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_7     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_8     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_9     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_A     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_B     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_C     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_D     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_E     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_F     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_G     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_H     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_I     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_J     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_K     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_L     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_M     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_N     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_O     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_P     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_Q     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_R     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_S     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_T     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_U     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_V     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_W     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_X     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_Y     );
		        IO_INSERT_KEY_TO_MAP( KEY_KEY_Z     );
		        IO_INSERT_KEY_TO_MAP( KEY_LWIN      );
		        IO_INSERT_KEY_TO_MAP( KEY_RWIN      );
		        IO_INSERT_KEY_TO_MAP( KEY_APPS      );
		        IO_INSERT_KEY_TO_MAP( KEY_SLEEP     );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD0   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD1   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD2   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD3   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD4   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD5   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD6   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD7   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD8   );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMPAD9   );
		        IO_INSERT_KEY_TO_MAP( KEY_MULTIPLY  );
		        IO_INSERT_KEY_TO_MAP( KEY_ADD       );
		        IO_INSERT_KEY_TO_MAP( KEY_SEPARATOR );
		        IO_INSERT_KEY_TO_MAP( KEY_SUBTRACT  );
		        IO_INSERT_KEY_TO_MAP( KEY_DECIMAL   );
		        IO_INSERT_KEY_TO_MAP( KEY_DIVIDE    );
		        IO_INSERT_KEY_TO_MAP( KEY_F1        );
		        IO_INSERT_KEY_TO_MAP( KEY_F2        );
		        IO_INSERT_KEY_TO_MAP( KEY_F3        );
		        IO_INSERT_KEY_TO_MAP( KEY_F4        );
		        IO_INSERT_KEY_TO_MAP( KEY_F5        );
		        IO_INSERT_KEY_TO_MAP( KEY_F6        );
		        IO_INSERT_KEY_TO_MAP( KEY_F7        );
		        IO_INSERT_KEY_TO_MAP( KEY_F8        );
		        IO_INSERT_KEY_TO_MAP( KEY_F9        );
		        IO_INSERT_KEY_TO_MAP( KEY_F10       );
		        IO_INSERT_KEY_TO_MAP( KEY_F11       );
		        IO_INSERT_KEY_TO_MAP( KEY_F12       );
		        IO_INSERT_KEY_TO_MAP( KEY_F13       );
		        IO_INSERT_KEY_TO_MAP( KEY_F14       );
		        IO_INSERT_KEY_TO_MAP( KEY_F15       );
		        IO_INSERT_KEY_TO_MAP( KEY_F16       );
		        IO_INSERT_KEY_TO_MAP( KEY_F17       );
		        IO_INSERT_KEY_TO_MAP( KEY_F18       );
		        IO_INSERT_KEY_TO_MAP( KEY_F19       );
		        IO_INSERT_KEY_TO_MAP( KEY_F20       );
		        IO_INSERT_KEY_TO_MAP( KEY_F21       );
		        IO_INSERT_KEY_TO_MAP( KEY_F22       );
		        IO_INSERT_KEY_TO_MAP( KEY_F23       );
		        IO_INSERT_KEY_TO_MAP( KEY_F24       );
		        IO_INSERT_KEY_TO_MAP( KEY_NUMLOCK   );
		        IO_INSERT_KEY_TO_MAP( KEY_SCROLL    );
		        IO_INSERT_KEY_TO_MAP( KEY_LSHIFT    );
		        IO_INSERT_KEY_TO_MAP( KEY_RSHIFT    );
		        IO_INSERT_KEY_TO_MAP( KEY_LCONTROL  );
		        IO_INSERT_KEY_TO_MAP( KEY_RCONTROL  );
		        IO_INSERT_KEY_TO_MAP( KEY_LMENU     );
		        IO_INSERT_KEY_TO_MAP( KEY_RMENU     );
		        IO_INSERT_KEY_TO_MAP( KEY_PLUS      );
		        IO_INSERT_KEY_TO_MAP( KEY_COMMA     );
		        IO_INSERT_KEY_TO_MAP( KEY_MINUS     );
		        IO_INSERT_KEY_TO_MAP( KEY_PERIOD    );
		        IO_INSERT_KEY_TO_MAP( KEY_ATTN      );
		        IO_INSERT_KEY_TO_MAP( KEY_CRSEL     );
		        IO_INSERT_KEY_TO_MAP( KEY_EXSEL     );
		        IO_INSERT_KEY_TO_MAP( KEY_EREOF     );
		        IO_INSERT_KEY_TO_MAP( KEY_PLAY      );
		        IO_INSERT_KEY_TO_MAP( KEY_ZOOM      );
		        IO_INSERT_KEY_TO_MAP( KEY_PA1       );
		        IO_INSERT_KEY_TO_MAP( KEY_OEM_CLEAR );                
            }            

            std::string uKey = ToUpper(str);
            if( sStringToKeyMap.find(uKey) == sStringToKeyMap.end() )
            {
                // arbitrary wrong key...
                return (KEY)-1;
            }

            return sStringToKeyMap[uKey];
        }

        // don't need the mappings anymore
        #undef IO_KEY_TO_STRING
        #undef IO_INSERT_KEY_TO_MAP


        /// Parse an input type
        /// @param inputSource the source that the input type is for
        /// @param inputType the inputType string to be parsed
        /// @return the inputType value
        IOMapping::InputType GetInputType( IOMapping::InputSource inputSource, const std::string& inputType )
        {
            std::string lType = ToUpper(inputType);

            // enum is arranged such that mouse_onpress is 3 after key_onpress so add three if this is a mouse
            const uint32_t kTypeOffset = inputSource == IOMapping::kInputSource_Keyboard? 0 : 3;

            // use addition trick on these three types
            if( lType == "ONPRESS" )            
                return (IOMapping::InputType)(IOMapping::kInputType_Key_OnPress + kTypeOffset);

            else if( lType == "ONRELEASE" )
                return (IOMapping::InputType)(IOMapping::kInputType_Key_OnRelease + kTypeOffset);

            else if( lType == "ONHOLD" )
                return (IOMapping::InputType)(IOMapping::kInputType_Key_OnHold + kTypeOffset);

            //-----
            else if( lType == "MOVEX" )
                return IOMapping::kInputType_Mouse_Move_X;

            else if( lType == "MOVEY" )
                return IOMapping::kInputType_Mouse_Move_Y;

            else if( lType == "SCROLL" )
                return IOMapping::kInputType_Mouse_Scroll;

            AssertWarnMsg( false, "Unrecognized input type " << inputType );
            return (IOMapping::InputType)-1;
        }

        /// Parse a mouse button from a string
        /// @param button the mouse button string
        /// @return the mouse button value
        MOUSE_BUTTON GetMouseButtonFromString( const std::string& button )
        {
            std::string lButton = ToUpper(button);
            if( lButton == "RIGHT" )
                return MOUSE_RBUTTON;

            else if( lButton == "LEFT" )
                return MOUSE_LBUTTON;

            else if( lButton == "MIDDLE" )
                return MOUSE_MBUTTON;

            else if( lButton == "MOVEX" )
                return MOUSE_MOVE_X;

            else if( lButton == "MOVEY" )
                return MOUSE_MOVE_Y;

            else if( lButton == "SCROLL" )
                return MOUSE_SCROLL;

            AssertWarnMsg( false, "Invalid mouse button trying to be bound: " << button );
            return (MOUSE_BUTTON)-1;
        }

        typedef std::vector<string> TokenList;

        /// Parse a string into a set of lowercase tokens
        /// @param inputString the string to be parsed
        /// @param delim the delimitor that separates the tokens
        /// @return a list of upper case tokens
        TokenList ParseTokens( const std::string& inputString, const std::string& delim )
        {
            using namespace boost;
            using namespace algorithm;

            TokenList tl;
            split( tl, inputString, is_any_of(delim) );

            for( uint32_t i = 0; i < (uint32_t)tl.size(); ++i )
            {
                tl[i] = ToUpper(tl[i]);
            }

            return tl;
        }

        /// Get the input code for a set of input tokens
        /// @param tlist input token list
        /// @return the input code
        InputCode GetInputCode( const TokenList& tlist )
        {
            InputCode code = 0;

            TokenList::const_iterator citr = tlist.begin();
            TokenList::const_iterator cend = tlist.end();

            for( ; citr != cend; ++citr )
            {
                // is this a key?
                KEY key = GetKeyFromString(*citr);
                if( key != (KEY)-1 )
                {
                    // if it is control, add to mod list
                    if( key == irr::KEY_CONTROL )
                        code |= IOMapping::kInputMod_Ctrl;

                    // if it is shift, add to mod list
                    else if( key == irr::KEY_SHIFT )
                        code |= IOMapping::kInputMod_Shift;

                    // otherwise, if we have not set a key, add the key into the code
                    else if( (code & IOMapping::kInputMod_Clear_Mask) == 0 )
                        code |= key;
                }

                // maybe it is a button
                else
                {
                    MOUSE_BUTTON button = GetMouseButtonFromString(*citr);
                    if( button != (MOUSE_BUTTON)-1 )
                    {
                        if( (code & IOMapping::kInputMod_Clear_Mask) == 0 )
                            code |= button;
                    }
                }
            }

            // did someone try to pass in _only_ modifiers?
            if( (code & IOMapping::kInputMod_Clear_Mask) == 0 )
                code = 0;

            return code;
        }
    };

    /// Bind a key to the python io map
    /// @param key a string version of the key to bind to the map
    /// @param inputType a string version of the input type to bind
    /// @param pyAction the python action to take when the state is correct
    void PyIOMap::BindKey( const std::string& key, const std::string& inputType, PythonIOMapping::Action pyAction )
    {
        // convert the parameters into values
        InputCode            inputCode   = GetInputCode( ParseTokens( key, "+" ) );        
        IOMapping::InputType eInputType  = GetInputType( IOMapping::kInputSource_Keyboard, inputType); 

        // bind the key
        BindIO( IOMapping::kInputSource_Keyboard, eInputType, inputCode, pyAction );
    }

    /// Bind a button to the python io map
    /// @param button a string version of the button to bind to the map
    /// @param inputState a string version of the input type to bind
    /// @param pyAction the python action to take when the state is correct
    void PyIOMap::BindMouseButton( const std::string& button, const std::string& inputState, PythonIOMapping::Action pyAction )
    {
        // convert the strings into values
        InputCode               inputCode   = GetInputCode( ParseTokens( button, "+" ) );
        IOMapping::InputType    eInputType  = GetInputType( IOMapping::kInputSource_Mouse, inputState );

        // bind the button
        BindIO( IOMapping::kInputSource_Mouse, eInputType, inputCode, pyAction );
    }

    /// Bind a mouse action to the python io map
    /// @param action a string version of the action to bind to the map
    /// @param pyAction the python action to take when the state is correct
    void PyIOMap::BindMouseAction( const std::string& action, PythonIOMapping::Action pyAction )
    {
        // convert the strings into values
        InputCode               inputCode   = GetInputCode( ParseTokens(action, "+" ) );
        IOMapping::InputType    eInputType  = GetInputType( IOMapping::kInputSource_Mouse, action );

        // bind the button
        BindIO( IOMapping::kInputSource_Mouse, eInputType, inputCode, pyAction );
    }

    /// Bind an io mapping
    /// @param source the input source to bind as
    /// @param type the input type to bind as
    /// @param code the input code to bind
    /// @param pyAction the action to take
    void PyIOMap::BindIO( IOMapping::InputSource source, IOMapping::InputType type, InputCode code, PythonIOMapping::Action pyAction )
    {
        // if a non-blank code
        if( code > 0 )
        {
            // create the mapping requirements
            PythonIOMapping::InputRequirements req;
            req.first.push_back(code);
            req.second = type;

            // create the key
            // the key consists of [mod flags...inputType...keys]
            IOMap::IOMapKey key = code + ((uint16_t)type<<8);

            // bind the action
            AddIOMapping( key, IOMappingPtr( new PythonIOMapping( source, req, pyAction ) ) );
        }
    }
} //end OpenNero
