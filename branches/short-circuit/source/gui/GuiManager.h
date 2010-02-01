//--------------------------------------------------------
// OpenNero : GuiManager
//  manages our gui elements
//--------------------------------------------------------

#ifndef _GAME_GUI_MANAGER_H_
#define _GAME_GUI_MANAGER_H_

#include "core/Common.h"
#include "core/IrrUtil.h"
#include "scripting/scripting.h"
#include <list>

namespace irr
{
    namespace gui
    {
        class IGUIElement;
    }
}

namespace OpenNero
{
    using namespace irr;
    using namespace gui;

    /// @cond
    BOOST_SHARED_DECL( SimFactory );
    BOOST_SHARED_DECL( GuiManager );
    BOOST_SHARED_DECL( GuiManagerProxy );
    BOOST_SHARED_DECL( GuiBase );
    BOOST_SHARED_DECL( IGuiElementFactory );
    /// @endcond

    /// These resources are for use by the Element Factories for creating,
    /// loading and instantiating gui elements.
    struct GuiElementFactoryResources
    {
        IrrHandles          mIrr;           ///< The irrlicht handles
        GuiManager*         mpManager;      ///< A pointer to the parent manager

        GuiElementFactoryResources();
    };

    /// A gui element factory is responsible for creating an instance of a child
    /// of GuiBase in some fashion. The system was designed such that external developers
    /// could add their own factories from C++ or Python
    class IGuiElementFactory
    {
    public:

        /// virtual dtor
        virtual ~IGuiElementFactory();

        /// Create an instance of a child of GuiBase
        /// @param newElementId the element to assign to the new instance
        /// @return a ptr to the newly created instance
        virtual GuiBasePtr CreateElement( uint32_t newElementId ) = 0;

        /// Register the factory resources with the factory
        /// @param facRes the resources to use in creation
        void RegisterResources( GuiElementFactoryResources* facRes );

    protected:

        /// a ptr to the resources to use in creation
        GuiElementFactoryResources* mRes;
    };

    /// The gui manager proxy is a help class designed to restrict the api view
    /// of the gui manager. Objects can still access the manager, but only in
    /// narrow paths.
    class GuiManagerProxy
    {
    public:

        /// ctor that takes in a ref to the manager
        GuiManagerProxy( GuiManager& inst );

        /// Modify the name that an element is registered as with the manager
        /// @param oldName the name the element was prior registered as
        /// @param newName the new name to register it as
        /// @return true if a modification of the registry was made
        bool ModifyRegisteredElementName( const std::string& oldName, const std::string& newName );

        /// Modify the focus status of an element.
        /// @param element the element to change the focus status of
        /// @param focused if true set the focus, if false remove the focus
        void SetFocusStatus( uint32_t element, bool focused );

    private:

        /// our ref to the manager
        GuiManager& mGuiManagerInst;
    };


    /// A Manager of the gui containers
    class GuiManager : public IEventReceiver
    {
    public:
        /// A Python execution object
        typedef  boost::python::object PythonExecObject;

    public:

        friend class GuiManagerProxy;

    public:

        /// Constructor
        /// @param handles the irrlicht handles to use for loading
        /// @param simFac the SimFactory to use for loading
        GuiManager( const IrrHandles& handles, SimFactoryPtr simFac );

        /// Dtor
        ~GuiManager();

		/// handle an event
        /// @param event the event details
        /// @return true if we handled the event
		bool OnEvent(const SEvent& event);

        /** @name Creation, Accessor, and Removal methods */
        ///@{

        /// Add a previously constructed gui base to the system
        /// @param id the id to add the base as
        /// @param base the gui base to add
        /// @return the base if successful
        GuiBasePtr addGuiBase( uint32_t id, GuiBasePtr base );

        /// Create an element out of one of our factories of the given type
        /// @param type the type of element to create
        /// @return a ptr to a new element or NULL if failed
        GuiBasePtr createElement( const std::string& type );

		/// remove all elements
		void RemoveAll();

		/// remove a given element
        /// @param guiId the id number of the element
		void Remove( uint32_t guiId );
        void Remove( const std::string& name );

        /// @return the number of elements we hold
        uint32_t getNumElements() const;

        /// methods to get an element
        GuiBasePtr getElement( int32_t id ) const;
        GuiBasePtr getElement( const std::string& name ) const;
        GuiBasePtr getElement( const std::string& name );           // Special override to allow python accesss

        ///@}

        /** @name Gui Element Factory Methods */
        ///@{

        /// Register an element factory with the system
        /// @param facKey the key type of the factory (ie: button, image, text)
        /// @param fac the factory that does the creation
        void RegisterElementFactory( const std::string& facKey, IGuiElementFactoryPtr fac );

        /// Remove an element factory from the system
        /// @param facKey the key to remove by
        void RemoveElementFactory( const std::string& facKey );

        /// Remove all element factories from the system
        void ClearElementFactories();

        ///@}

        /** @name Other Methods */
        ///@{

        /// @return true if the manager has input focus
        bool HasFocus() const;

        /// Override the transparency level of all of the gui elements.
        /// @param alpha an alpha value in [0,1] how visible the elements should be. 0 means not visible, 1 means full opaque
        void setGuiTransparency( float32_t alpha );

        /// Set the font for the current skin to use
        /// @param fontPath relative the path to the desired font
        /// @return true if the load was successful
        bool setFont( const std::string& fontPath );

        /// Open up a file chooser dialog to get a filename from the user
        /// @param dialogTitle the title to put in the dialog
        /// @param modal whether or not to function modally
        /// @param callbackFunc the python function to call when we get a filename
        void openFileChooserDialog( const std::string& dialogTitle, bool modal, PythonExecObject callbackFunc );

        /// Checks if a file chooser dialog is open
        /// @return true if a dialog is open; false otherwise
        bool isOpenFileChooserDialog() const;

        ///@}

    public:

		/// allocate an unused gui id
        static uint32_t AllocateGuiId();

    private:

        /// A dialog allowing the user to pick a file to operate on
        struct FileChooserDialog
        {
            bool                                mWaitingForFilename; ///< Determines if we have a window open are waiting for user input
            std::string                         mDialogTitle;        ///< The title to place on the dialog
            int32_t                             mDialogID;           ///< The id associated with the dialog
            PythonExecObject                    mCallback;           ///< The callback method to call when a user picks a file
            bool                                mDialogOpen;         ///< Indicates whether the dialog is open

            FileChooserDialog()
                : mWaitingForFilename(false),
                  mDialogID(0),
                  mDialogOpen(false)
            {}
        };

    private:

        // a linked list of GuiBasePtrs
        typedef std::list<GuiBasePtr> GuiBaseList;

    private:

        /// Register all of our default factories with the system
        void RegisterDefaultElementFactories();

    private:

        /// Handlers for specific GUI events
        /// @{
        bool HandleMouseClick( const SEvent& event );
        bool HandleMouseMove( const SEvent& event );
        bool HandleFileSelect( const SEvent& event );
        bool HandleFileChooseDialogCancelled( const SEvent& event );
        bool HandleMenuItemSelected( const SEvent& event );
        bool HandleScrollBarChanged( const SEvent& event );
        bool HandleCheckBoxChanged( const SEvent& event );
        /// @}

        /// Modify the way an element is registered with the system
        /// @param oldName the previous name of the element
        /// @param newName the new name of the element
        /// @return true if the registry was altered
        bool ModifyRegisteredElementName( const std::string& oldName, const std::string& newName );

        /// Modify the focus status of an element.
        /// @param element the element to change the focus status of
        /// @param focused if true set the focus, if false remove the focus
        void SetFocusStatus( GuiBasePtr element, bool focused );

        /// attempt to find a receiver for a mouse event
        /// @param event the event details
        /// @param the root gui piece to search up from
        /// @return a ptr to a proper receiver or NULL if none found
        GuiBasePtr FindProperReceiver( const SEvent& event, GuiBasePtr startGui );

        /// find the elements at this position
        /// @param pos2D the position to query
        /// @return a list of elements at that position
        GuiBaseList findContainerByPosition( const Pos2i& pos2D );

		/// find a container by its id
        /// @id the id to query for
        /// @return a ptr to the element with that id
		GuiBasePtr findContainerById( int32_t id );

        /// Retrieves the id of the gui element that best fits this point. If a parent encloses this
        /// point and so does it's child, we will return the child id, not the parent id.
        /// @param c the parent gui container to traverse
        /// @param pos2D the position to intersect with the gui elements
        /// @return the id of the gui intersected, -1 if none
        int32_t findBestCollision( GuiBasePtr c, const Pos2i& pos2D );

        /// add a container to our cache
        /// @param id the id to add it as
        /// @param base the ptr to add
        /// @return the base
        GuiBasePtr addBase( uint32_t id, GuiBasePtr base );

    private:

        // a mapping from gui ids to gui containers
        typedef std::map<uint32_t, GuiBasePtr> GuiBaseMap;

        // a mapping from names to ids
        typedef std::map< std::string, uint32_t > GuiNameMap;

        // a mapping from types to factories
        typedef std::map< std::string, IGuiElementFactoryPtr > GuiElementFactoryMap;

    private:

        GuiBaseMap                      mGuiBases;			///< The containers we have cached
        GuiNameMap                      mGuiNames;          ///< Maps from names to ids
		GuiBasePtr	                    mFocusedContainer;	///< The container that currently has focus
        GuiBasePtr                      mPressedContainer;  ///< The container currently being pressed by the mouse
        GuiManagerProxyPtr              mpProxy;            ///< A Proxy for gui elements to interact with the manager
        IrrHandles                      mIrr;				///< Handles for irrlicht related tasks
        SimFactoryPtr                   mpSimFac;			///< A factory for loading things
        GuiElementFactoryResources      mElementFacRes;     ///< The element factory resources
        GuiElementFactoryMap            mElementFactoryMap; ///< A map from types to factories
        FileChooserDialog               mFileChooser;       ///< The file chooser dialog info

    private:

        static uint32_t    mGuiIdCounter;               ///< The current static count of gui elements
    };

} //end OpenNero

#endif // _GAME_GUI_MANAGER_H_

