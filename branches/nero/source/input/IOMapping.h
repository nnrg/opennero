//--------------------------------------------------------
// OpenNero : IOMapping
//  a mapping of user input to game actions
//--------------------------------------------------------

#ifndef _INPUT_IOMAPPING_H_
#define _INPUT_IOMAPPING_H_

#include "core/ONTypes.h"
#include "core/BoostCommon.h"
#include "core/HashMap.h"
#include "input/IOState.h"
#include "scripting/scriptIncludes.h"

#include <map>
#include <vector>
#include <string>
#include <utility>

namespace OpenNero
{
	/// @cond
    BOOST_SHARED_DECL( IOMap );
    BOOST_SHARED_DECL( IOMapping );
    BOOST_SHARED_DECL( PythonIOMapping );
    /// @endcond

    /// a mix of modifiers and keys into a flag
    typedef uint16_t InputCode;

    /// An IOMapping is an individual mapping in the IOMap. The IOMapping
    /// waits for an appropriate state in the IOState and when this state
    /// occurs it will do some sort of action
    class IOMapping
    {
    public:
    	virtual ~IOMapping() {}
        /// Flag that specifies which input device a mapping is for
        enum InputSource
        {
            kInputSource_Keyboard,
            kInputSource_Mouse
        };

        /// The 'state' that the mapping should take action in
        enum InputType
        {
            // do not change the order of these
            //-------------------------------
            kInputType_Key_OnPress,                 ///< When a key is pressed
            kInputType_Key_OnRelease,               ///< When a key is released
            kInputType_Key_OnHold,                  ///< When a key is held down

            kInputType_Mouse_OnPress,               ///< When a button is pressed
            kInputType_Mouse_OnRelease,             ///< When a button is released
            kInputType_Mouse_OnHold,                ///< When a button is held down
            //-------------------------------

            kInputType_Mouse_Move_X,                ///< When the mouse is moved on the x axis
            kInputType_Mouse_Move_Y,                ///< When the mouse is moved on the y axis
            kInputType_Mouse_Scroll                 ///< When the scroll wheel position is changed
        };

        /// Modifiers that can contribute to an action
        enum InputModifiers
        {
            kInputMod_Ctrl  = (1<<15),
            kInputMod_Shift = (1<<14),

            kInputMod_Mod_Mask = (kInputMod_Ctrl | kInputMod_Shift),
            kInputMod_Clear_Mask = ~kInputMod_Mod_Mask
        };

    public:

        /// Check the state of all required input device codes and return true
        /// if they are in a proper state to take action
        virtual bool CheckIORequirements( const IOState& ioState ) const = 0;

        /// performed the action that is mapped to the key state
        virtual void DoMappedAction() = 0;

    protected:

        /// check to see if the state modifiers meet our modifiers
        bool CheckModifierRequirements( InputCode inputCode, const IOState& ioState ) const;
    };

    /// The PythonIOMapping is an implementation of the abstract IOMapping.
    /// This class takes in a requirement set for input states and will
    /// call a python method when that input state is met
    class PythonIOMapping : public IOMapping
    {
    public:
        typedef boost::python::object               Action; ///< a python executeable object to run
        typedef std::vector<InputCode>              InputCodeList; ///< a list of input codes to trigger on
        typedef std::pair<InputCodeList, InputType> InputRequirements; ///< input requirements (shift, ctrl)

    public:

    	/// @param inputSource source of the input (mouse, keyboard)
    	/// @param requirements input requirements (shift, ctrl)
    	/// @param pyAction python action to trigger
       PythonIOMapping( InputSource inputSource, const InputRequirements& requirements, Action pyAction );

       bool CheckIORequirements( const IOState& ioState ) const;
       void DoMappedAction();

    private:

        InputSource         mSource;            ///< The input source
        InputRequirements   mRequirements;      ///< The required codes and states to perform the action
        Action              mAction;            ///< The python method to call
    };

    /// The IOMap is a collection of individual IOMappings. The IOMap will manage
    /// the mappings and the current IOState in order to coordinate when these
    /// actions will fire in the game.
    class IOMap
    {
    public:

        typedef InputCode IOMapKey; ///< the key type for the IOMap

    public:

        /// collect any user input through an event
        void GetUserInput( const irr::SEvent& event );

        /// poll the iomappings to see if the iostate is right for acting
        void ActOnUserInput();

        /// Get the current position of the mouse on the screen
        Pos2i GetMousePosition() const;

        /// mapping management
        void AddIOMapping( IOMapKey key, IOMappingPtr mappingPtr );
        void ClearMappings();

    private:

        typedef hash_map<IOMapKey, IOMappingPtr > IOMappings;

    private:

        IOState             mIOState;       ///< the current io state
        IOMappings          mIOMappings;    ///< our io mappings
    };

    /// The PyIOMap accepts python io bindings
    class PyIOMap : public IOMap
    {
    public:

        // binding methods
        void BindKey( const std::string& key, const std::string& inputState, PythonIOMapping::Action pyAction );
        void BindMouseButton( const std::string& button, const std::string& inputState, PythonIOMapping::Action pyAction );
        void BindMouseAction( const std::string& action, PythonIOMapping::Action pyAction );

    private:

        void BindIO( IOMapping::InputSource source, IOMapping::InputType type, InputCode code, PythonIOMapping::Action pyAction );
    };

} //end OpenNero

#endif // _INPUT_IOMAPPING_H_
