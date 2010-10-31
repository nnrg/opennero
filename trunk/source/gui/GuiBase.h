//--------------------------------------------------------
// OpenNero : GuiBase
//  a basic container for a gui piece
//--------------------------------------------------------

#ifndef _GAME_GUI_CONTAINER_H_
#define _GAME_GUI_CONTAINER_H_

#include "core/Common.h"
#include "core/IrrUtil.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( GuiBase );
    BOOST_PTR_DECL( GuiManagerProxy );
    BOOST_PTR_DECL( PyGuiBase );
    BOOST_PTR_DECL( SimFactory );
    /// @endcond
    class GuiManagerProxy;

    using namespace irr;
    using namespace irr::core;
    using namespace irr::gui;

    /// convert a wide string to a normal string
    /// has reckless abandon to the wide characters, assumes all
    /// are within the range of a normal string. DESTROY!
    /// @param str16 the wide string to convert to a normal string
    /// @return the converted string
    std::string wstring_to_string( const std::wstring& str16 );

    /**
	 * A basic class for all gui elements in the scene. Provides a basic amount of functionality
	 * shared by all gui class members. Exposes the fundamental set on input callbacks necessary
	 * for a functioning system.
	*/
	class GuiBase
    {
		// let the gui manager mess with out insides
        friend class GuiManager;

    public:

        /// These are bitflags that correspond to
        /// input handler methods that this instance
        /// garauntees to respond to.
        enum ResponseFlag
        {
            // I will respond to one of the following...
            kResponse_OnMouseLeftDown,
            kResponse_OnMouseRightDown,
            kResponse_OnMouseMiddleDown,

            kResponse_OnMouseLeftClick,
            kResponse_OnMouseRightClick,
            kResponse_OnMouseMiddleClick,

            kResponse_OnMouseEnter,
            kResponse_OnMouseLeave,
            kResponse_OnMouseMove,

            kResponse_OnScrollBarChange,
            
            kResponse_OnCheckBoxChange,

            kResponse_Max
        };

    public:

        GuiBase();

		/// Dtor
        virtual ~GuiBase();

		/// Set our visibility state
	    /// @param visible true if we should become visible
		void setVisible( bool visible );

		/// @return true iff this GUI object is visible
        bool isVisible();

		/// Set our 'enabledness' state
	    /// @param enabled toggle enabledness
		void setEnabled( bool enabled );

        /// Get whether or not our element is enabled
        bool isEnabled();

        /// Notify the element that we have gained focus
        void setFocus();

        /// Notify the element that we have lost focus
        void removeFocus();

        /// Check if the element has the focus
        bool isFocused() const;

		/// Get the id of our parent
	    /// @return the id stored by irrlicht of our parent, -1 if no parent
		int32_t getParentId() const;

        /// Get our id
        /// @return our id number
        int32_t getId() const;

        /// Get our name
        /// @return the name of this gui piece
        std::string getName() const;

        /// Set the name of this element
        /// @param name the name to set
        void setName( const std::string& name );

		/// Add a child to us
	    /// @param child the child to add
		virtual void addChild( GuiBasePtr child );

		/// set a text representation of this object
        /// @param text a wide string of text
        void setText( const std::wstring& text );

		/// set a text representation of this object
        /// @param text a string of text
        void setText( const std::string&  text );

        /// get the text representation of this object
        /// @return a wide string representing this element
        std::wstring getText() const;

        /// get the text representation of this object
        /// @return a string representing this element
        std::string getSText();

        /// get the absolute position of the element on screen
        /// @return the absolute screen space rectangle of this element
        Rect2i getAbsoluteBounds() const;

        /// get the position relative to the parent
        /// @return get the screen space rectangle of this element relative to parent
        Rect2i getRelativeBounds();

        /// set the relative position of the element
        /// @param rect the rectangle position to set the element at
        void setRelativeBounds( const Rect2i& rect );

        /// set relative position
        /// @param pos relative offset
        void  setRelativePosition( const Pos2i& pos );

        /// get relative position
        Pos2i getRelativePosition();

        /// set extent
        /// @param ext size of this element
        void  setExtent( const Pos2i& ext );

        /// get extent
        Pos2i getExtent();

		/// Recalculate the absolute position after a movement or a heiarchy reorganization
		void updateAbsolutePosition();

        /// remove ourself from the scene
        void removeFromScene();

        /// @param guiElement the gui element to contain
        void setGuiElement( IGuiElement_IPtr guiElement );

        /// @name Abstract Methods
        /// Methods for later C++ or Python classes to implement

        ///@{

        /// can this element gain focus of the keyboard?
        virtual bool CanGainFocus() = 0;

        // check what this instance responds to
        // @see ResponseFlag enum above
        virtual bool RespondsTo( ResponseFlag flag ) = 0;

        /// @name mouse input callbacks
        /// @{
        virtual bool OnMouseLeftDown  ( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseRightDown ( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseMiddleDown( /*const SEvent& event*/ ) = 0;

		virtual bool OnMouseLeftClick  ( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseRightClick ( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseMiddleClick( /*const SEvent& event*/ ) = 0;

        virtual bool OnMouseEnter( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseLeave( /*const SEvent& event*/ ) = 0;
        virtual bool OnMouseMove(  /*const SEvent& event*/ ) = 0;

        virtual bool OnScrollBarChange( /* const SEvent& event */ ) = 0;

        virtual bool OnCheckBoxChange( /* const SEvent& event */ ) = 0;
        /// @}

        ///@}

    public:
        /// PyObject is a boost::python::object
        typedef boost::python::object PyObject;

    public:

        /** @name Python property interface methods
          * These methods will be linked into python to enabled the user
          * to assign and read the override function from the python object
          */

        ///@{

        void SetOnMouseLeftDown( PyObject func );
        void SetOnMouseRightDown( PyObject func );
        void SetOnMouseMiddleDown( PyObject func );
        void SetOnMouseLeftClick( PyObject func );
        void SetOnMouseRightClick( PyObject func );
        void SetOnMouseMiddleClick( PyObject func );
        void SetOnMouseEnter( PyObject func );
        void SetOnMouseLeave( PyObject func );
        void SetOnMouseMove( PyObject func );
        void SetOnScrollBarChange( PyObject func );
        void SetOnCheckBoxChange( PyObject func );

        PyObject GetOnMouseLeftDown();
        PyObject GetOnMouseRightDown();
        PyObject GetOnMouseMiddleDown();
        PyObject GetOnMouseLeftClick();
        PyObject GetOnMouseRightClick();
        PyObject GetOnMouseMiddleClick();
        PyObject GetOnMouseEnter();
        PyObject GetOnMouseLeave();
        PyObject GetOnMouseMove();
        PyObject GetOnScrollBarChange();
        PyObject GetOnCheckBoxChange();
        ///@}

    protected:

        /// Set a function for response
        void SetResponseFunction( ResponseFlag flag, PyObject func );
        /// Get the response function
        PyObject GetResponseFunction( ResponseFlag flag );

    protected:

        /// @return Get the gui element we contain
        IGuiElement_IPtr getGuiElement();
        
        /// convert a position in absolute coordinates to relative coordinates
        /// @param pos2D the position to convert
        position2di convertToRelative( const position2di& pos2D );

		/// get the irrlicht gui element we contain
        /// @return the gui element we contain
		IGUIElement*	getIrrGuiElement();

        /// set the factory we use for loading
        /// @param factory the factory to use
        void setSimFactory( SimFactoryPtr factory );

        /// @return the factory to use for loading
        SimFactoryPtr getSimFactory();

        /// @param proxy the manager proxy to use
        void setManagerProxy( GuiManagerProxyPtr proxy );

    private:

        /// the element's relative bounds
        Rect2i mElementBounds;

        /// a name for this piece
        std::string                 mGuiName;

        /// a textual representation for the object
        /// this is used mainly for storage since irrlicht doesn't store.
        std::wstring                mText;

        /// The irrlicht element which we use
        IGuiElement_IPtr mpGuiElement;

        /// The factory we can use for loading
        SimFactoryPtr mpSimFactory;

        /// A proxy to interact with the gui manager
        GuiManagerProxyPtr mpManagerProxy;

        /// is this element focused?
        bool mFocused;

    private:

        /// These are python functions that should be called if no C++ override exists
        PyObject        mResponseFunctions[kResponse_Max];
    };

    /// The PyGuiBase class is an implementation of GuiBase which looks to
    /// Python to fill in the details of abstract event handlers.
    class PyGuiBase
        : public GuiBase
        , public boost::python::wrapper<GuiBase>
    {
    public:

        /** @name Python Event Handlers
         * The following methods check for the existence of
         * an overriden python method that responds to these events.
         * If there is no such python method, it calls the corresponding
         * Default_* method below. Note: The return value says whether or
         * not the event was handled, *NOT* if the action in the handle
         * was successful.
        */
        ///@{

        // TODO : Currently the python handlers cannot use the
        // SEvent class because it is not exported!

        virtual bool CanGainFocus();

        virtual bool RespondsTo( ResponseFlag flag );

        virtual bool OnMouseLeftDown  ( /*const SEvent& event*/ );
        virtual bool OnMouseRightDown ( /*const SEvent& event*/ );
        virtual bool OnMouseMiddleDown( /*const SEvent& event*/ );

		virtual bool OnMouseLeftClick  ( /*const SEvent& event*/ );
        virtual bool OnMouseRightClick ( /*const SEvent& event*/ );
        virtual bool OnMouseMiddleClick( /*const SEvent& event*/ );

        virtual bool OnMouseEnter( /*const SEvent& event*/ );
        virtual bool OnMouseLeave( /*const SEvent& event*/ );
        virtual bool OnMouseMove(  /*const SEvent& event*/ );

        virtual bool OnScrollBarChange( /*const SEvent& event*/ );
        
        virtual bool OnCheckBoxChange( /* const SEvent& event */ );
        ///@}

        /** @name C++ Event Handlers
         * The following methods are called if the existence check
         * for a python override fails. Same rules as above apply
         * to the return value from the method.
        */
        ///@{

        virtual bool Default_CanGainFocus();

        virtual bool Default_OnMouseLeftDown  ( /*const SEvent& event*/ );
        virtual bool Default_OnMouseRightDown ( /*const SEvent& event*/ );
        virtual bool Default_OnMouseMiddleDown( /*const SEvent& event*/ );

		virtual bool Default_OnMouseLeftClick  ( /*const SEvent& event*/ );
        virtual bool Default_OnMouseRightClick ( /*const SEvent& event*/ );
        virtual bool Default_OnMouseMiddleClick( /*const SEvent& event*/ );

        virtual bool Default_OnMouseEnter( /*const SEvent& event*/ );
        virtual bool Default_OnMouseLeave( /*const SEvent& event*/ );
        virtual bool Default_OnMouseMove(  /*const SEvent& event*/ );

        virtual bool Default_OnScrollBarChange(); 
        
        virtual bool Default_OnCheckBoxChange();
        ///@}

    protected:

        /// Internal RespondsTo method for children to call into.
        /// Separated into its own method to make it more visible
        /// @param flag the flag to check for
        /// @return true if we respond to the input type
        bool RespondsTo_Internal( ResponseFlag flag );

    private:

        /// Check to see if a python override for the given over name exists
        /// @param overrideName the name of the override method to check for
        /// @return true if we found the override python method
        bool CheckForOverride( const std::string& overrideName ) const;

        /// Attempt to execute a python override that goes by the provided name.
        /// If the override does not exist, bail and return false.
        /// @param overrideName the name of the override method to search for and execute
        /// @return true if the override exists, false otherwise
        bool AttemptOverrideExecution( const std::string& overrideName );

        /// Attempt to execute a python response function. This method will both
        /// check for the existence of the function and execute it
        /// @param flag flag representing the response function to test
        /// @return true if we executed a python response function
        bool AttemptPythonExecution( ResponseFlag flag );
    };
}

/** @name Hacky Inheritance Code
 * The following two macros try to substitute for boost python's seeminly broken inheritance system.
 * If the inheritance issues get resolved, we do quickly ditch this code.
*/

///@{

/// It is really a shame that we have to do this. Until we can figure out how to get the order of
/// exportation with inheritance and the other related inheritance issues we need to "fake" inheritance
/// by including the methods of PyGuiBase in all descendant class of it.

/// Place all the typedef for our member method pointers in the code base
#define _GUI_BASE_PRE_HACK_( guiClass )     typedef void (guiClass::*SetTextPtr)( const std::string& )


/// Export all of the PyGuiBase python methods to the child class
#define _GUI_BASE_HACK_( guiClass )     .def( "getParentId", &guiClass::getParentId, "Set the id of this element's parent." ) \
                                        .def( "getId", &guiClass::getId, "Get the id of this element." ) \
                                        .def( "addChild", &guiClass::addChild, "Add a child to this element.", "addChild(child)" ) \
                                        .def( "getAbsoluteBounds", &guiClass::getAbsoluteBounds, "Gets the absolute screen space position of the element rect" ) \
                                        .def( "getRelativeBounds", &guiClass::getRelativeBounds, "Get the relative screen space bounds of the element rect" ) \
                                        .add_property( "enabled", &guiClass::isEnabled, &guiClass::setEnabled, "Determines if this element is enabled" ) \
                                        .add_property( "visible", &guiClass::isVisible, &guiClass::setVisible ) \
                                        .add_property( "name", &guiClass::getName, &guiClass::setName ) \
                                        .add_property( "text", &guiClass::getSText, (SetTextPtr)&guiClass::setText ) \
                                        .add_property( "position", &guiClass::getRelativePosition, &guiClass::setRelativePosition ) \
                                        .add_property( "extent", &guiClass::getExtent, &guiClass::setExtent ) \
                                        .add_property( "bounds", &guiClass::getRelativeBounds, &guiClass::setRelativeBounds ) \
                                        .add_property( "OnMouseLeftDown",    &guiClass::GetOnMouseLeftDown,    &guiClass::SetOnMouseLeftDown ) \
                                        .add_property( "OnMouseRightDown",   &guiClass::GetOnMouseRightDown,   &guiClass::SetOnMouseRightDown ) \
                                        .add_property( "OnMouseMiddleDown",  &guiClass::GetOnMouseMiddleDown,  &guiClass::SetOnMouseMiddleDown ) \
                                        .add_property( "OnMouseLeftClick",   &guiClass::GetOnMouseLeftClick,   &guiClass::SetOnMouseLeftClick ) \
                                        .add_property( "OnMouseRightClick",  &guiClass::GetOnMouseRightClick,  &guiClass::SetOnMouseRightClick ) \
                                        .add_property( "OnMouseMiddleClick", &guiClass::GetOnMouseMiddleClick, &guiClass::SetOnMouseMiddleClick ) \
                                        .add_property( "OnMouseEnter",       &guiClass::GetOnMouseEnter,       &guiClass::SetOnMouseEnter ) \
                                        .add_property( "OnMouseLeave",       &guiClass::GetOnMouseLeave,       &guiClass::SetOnMouseLeave ) \
                                        .add_property( "OnMouseMove",        &guiClass::GetOnMouseMove,        &guiClass::SetOnMouseMove ) \
                                        .add_property( "OnScrollBarChange",  &guiClass::GetOnScrollBarChange,  &guiClass::SetOnScrollBarChange ) \
                                        .add_property( "OnCheckBoxChange",   &guiClass::GetOnCheckBoxChange,   &guiClass::SetOnCheckBoxChange )

///@}

#endif // _GAME_GUI_CONTAINER_H_

