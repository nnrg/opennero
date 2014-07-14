//--------------------------------------------------------
// OpenNero : GuiManager
//  manages our gui elements
//--------------------------------------------------------

#include "core/Common.h"
#include <fstream>

#include "core/Log.h"
#include "core/IrrSerialize.h"
#include "core/File.h"

#include "boost/lexical_cast.hpp"

#include "game/objects/PropertyMap.h"
#include "game/factories/SimFactory.h"
#include "game/Kernel.h"

#include "gui/GuiBase.h"
#include "gui/GuiManager.h"
#include "gui/GuiImage.h"
#include "gui/GuiEditBox.h"
#include "gui/GuiCheckBox.h"
#include "gui/GuiText.h"
#include "gui/GuiComboBox.h"
#include "gui/GuiScrollBar.h"
#include "gui/GuiWindow.h"
#include "gui/GuiContextMenu.h"
#include "gui/GuiButton.h"

namespace OpenNero
{
    GuiElementFactoryResources::GuiElementFactoryResources()
        : mpManager(NULL)
    {}



    // default position for elements to go
    static Rect2i sDefaultGuiElementPosition = Rect2i( 0, 0, 100, 100 );



    IGuiElementFactory::~IGuiElementFactory()
    {}

    void IGuiElementFactory::RegisterResources( GuiElementFactoryResources* facRes )
    {
        mRes = facRes;
    }



    /// A very generic element factory which takes care of the
    /// creation of the intrusive ptr and the setting of the element
    template <typename Type, typename IrrType>
    class GenericElementFactory : public IGuiElementFactory
    {
    public:

        typedef boost::intrusive_ptr<IrrType> IrrType_IPtr;   ///< IrrType_IPtr is a boost::intrusive_ptr
        typedef boost::shared_ptr<Type> TypePtr;              ///< TypePtr is a boost::shared_ptr

    protected:
        /// Generic construction given an intrusive pointer
        GuiBasePtr GenericConstruction( IrrType_IPtr irrPtr )
        {
            Assert( irrPtr );

            TypePtr elem( new Type );
            elem->setGuiElement(irrPtr);

            return elem;
        }
    };


    /// A factory for making EditBoxElement
    class GuiEditBoxElementFactory : public GenericElementFactory< GuiEditBox, IGUIEditBox>
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            Assert( mRes );
            return GenericConstruction( mRes->mIrr.getGuiEnv()->addEditBox( L"", sDefaultGuiElementPosition, true, NULL, newElemId ) );
        }     
    };

    /// A factory for making EditBoxElement
    class GuiCheckBoxElementFactory : public GenericElementFactory< GuiCheckBox, IGUICheckBox>
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            Assert( mRes );
            return GenericConstruction( mRes->mIrr.getGuiEnv()->addCheckBox( false, sDefaultGuiElementPosition, NULL, newElemId ) );
        }     
    };

    /// A factory for making ScrollBar
    class GuiScrollBarElementFactory : public GenericElementFactory< GuiScrollBar, IGUIScrollBar>
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            Assert( mRes );
            return GenericConstruction( mRes->mIrr.getGuiEnv()->addScrollBar( false, sDefaultGuiElementPosition, NULL, newElemId ) );
        }
    };

    /// A factory for making horizontal ScrollBar
    class GuiHorizontalScrollBarElementFactory : public GenericElementFactory< GuiScrollBar, IGUIScrollBar>
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            Assert( mRes );
            return GenericConstruction( mRes->mIrr.getGuiEnv()->addScrollBar( true, sDefaultGuiElementPosition, NULL, newElemId ) );
        }
    };

    /// A factory for making TextElement
    class GuiTextElementFactory : public GenericElementFactory< GuiText, IGUIStaticText>
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            Assert( mRes );
            return GenericConstruction( mRes->mIrr.getGuiEnv()->addStaticText( L"", sDefaultGuiElementPosition, false, true, NULL, newElemId ) );
        }     
    };

    /// a factory for making GuiWindow
    class GuiWindowElementFactoryBase : public IGuiElementFactory
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId, bool modal )
        {
            // ok, so here is the deal. Irrlicht provides us with a close button (x)
            // natively on the gui windows. However, they dont give us any message that
            // the window is closing. So, clicking on the close button will kill the gui piece
            // on the irrlicht side, but not ours. Therefore, we need to wrap the close button
            // in this class to work properly.
            class CloseButton : public GuiButton
            {
            private:

                GuiManager*     mManager;       ///< The manager of the gui
                GuiWindowPtr    mWindow;        ///< The parent window

            public:
                CloseButton( GuiManager* man, GuiWindowPtr parent) 
                    : mManager(man), mWindow(parent)  {}

                bool OnMouseLeftClick  ( /*const SEvent& event*/ )
                {
                    // kill the window
                    assert( mWindow && mManager );
                    mManager->Remove( mWindow->getId() );
                    return false;
                }
            };

            Assert( mRes );

            // create the irr window
            IGUIWindow* irrWin = mRes->mIrr.getGuiEnv()->addWindow( sDefaultGuiElementPosition, modal, L"", NULL, newElemId );
            Assert( irrWin );

            // create our window
            GuiWindowPtr win( new GuiWindow );
            Assert( win );

            // save the irr window
            win->setGuiElement(irrWin);

            // wrap up the close button so that we can get callbacks from the close click                
            IGUIButton* close = irrWin->getCloseButton();
            Assert( close && close->getParent() == irrWin );                                
            
            // assign it one of our ids                
            uint32_t childId = GuiManager::AllocateGuiId();
            close->setID( childId );

            // create our close button
            CloseButton* cb = new CloseButton( mRes->mpManager,win);
            GuiButtonPtr closeButton( static_cast<GuiButton*>(cb) );
            closeButton->setGuiElement(close);

            Assert( mRes->mpManager );
            mRes->mpManager->addGuiBase( childId, closeButton );

            // add the window to our sim
            return win;		    
        }
    };

    /// A factory for making a WindowElement
    class GuiWindowElementFactory : public GuiWindowElementFactoryBase
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            return GuiWindowElementFactoryBase::CreateElement( newElemId, false );
        }
    };

    /// a factory for creating GuiWindow
    class GuiWindowElementFactoryModal : public GuiWindowElementFactoryBase
    {
    public:
        GuiBasePtr CreateElement( uint32_t newElemId )
        {
            return GuiWindowElementFactoryBase::CreateElement( newElemId, true );
        }
    };



    // macro to use the creation of repetitive element factories
#define MAKE_GENERIC_ELEMENT_FACTORY( Type, IrrType, Ctor ) BOOST_PTR_DECL( Type##ElementFactory ); \
    class Type##ElementFactory : public GenericElementFactory< Type, IrrType >    \
    {                                                                       \
    public:                                                                 \
        GuiBasePtr CreateElement( uint32_t newElemId )                      \
        {                                                                   \
            Assert( mRes );                                                 \
            return GenericConstruction( mRes->mIrr.getGuiEnv()->Ctor( sDefaultGuiElementPosition, NULL, newElemId ) ); \
        }                                                                   \
    };



    // create our element factories
    MAKE_GENERIC_ELEMENT_FACTORY( GuiButton, IGUIButton, addButton );
    MAKE_GENERIC_ELEMENT_FACTORY( GuiComboBox, IGUIComboBox, addComboBox );    
    MAKE_GENERIC_ELEMENT_FACTORY( GuiImage, IGUIImage, addImage ); 
    MAKE_GENERIC_ELEMENT_FACTORY( GuiContextMenu, IGUIContextMenu, addContextMenu );



    GuiManagerProxy::GuiManagerProxy( GuiManager& inst )
        : mGuiManagerInst(inst)
    {}

    bool GuiManagerProxy::ModifyRegisteredElementName( const std::string& oldName, const std::string& newName )
    {
        return mGuiManagerInst.ModifyRegisteredElementName(oldName,newName);
    }

    void GuiManagerProxy::SetFocusStatus( uint32_t elementID, bool focused )
    {
        // try to get the element
        GuiBasePtr pElem = mGuiManagerInst.getElement(elementID);
        if( pElem )
        {
            mGuiManagerInst.SetFocusStatus(pElem,focused);
        }
    }



    /// The unique counter
    uint32_t GuiManager::mGuiIdCounter = 1;

    /// Ctor
    /// @param handles the irrlicht handles to use
    /// @param simFac the sim factory to use for loading
    GuiManager::GuiManager( const IrrHandles& handles, SimFactoryPtr simFac ) 
        : mIrr(handles)
        , mpSimFac(simFac)        
    {
        mElementFacRes.mIrr = mIrr;
        mElementFacRes.mpManager = this;
        mpProxy.reset( new GuiManagerProxy(*this) );

        // register all of the default element factories
        RegisterDefaultElementFactories();
    }

    /// Dtor
    /// Go through our cache and remove all containers from the scene
    GuiManager::~GuiManager()
    {
        // remove all of the gui containers from the scene
		RemoveAll();        
    }

	/**
	 * OnEvent will handle events coming from the irrlicht system. The purpose of this
	 * handler is to get the calling gui piece id from the event, do a lookup on the id,
	 * and pass the event to the appropriate gui container piece we have cached.
	 * @param event the event to pass
	 * @return true if we find a container and it processes the event, false otherwise
	*/
	bool GuiManager::OnEvent(const SEvent& event)
	{
		GuiBasePtr gui;		

		// get the gui container
        if( event.EventType == EET_MOUSE_INPUT_EVENT )
        {	
            // look at the type of mouse input
            switch( event.MouseInput.Event )
            {

            // Note: Let this pass down!
            case EMIE_LMOUSE_PRESSED_DOWN:
            case EMIE_RMOUSE_PRESSED_DOWN:
            case EMIE_MMOUSE_PRESSED_DOWN:                    
            case EMIE_LMOUSE_LEFT_UP:
			case EMIE_RMOUSE_LEFT_UP:
			case EMIE_MMOUSE_LEFT_UP:
                   return HandleMouseClick(event);

			case EMIE_MOUSE_MOVED:
                   return HandleMouseMove(event);
            default:
                return false;
            }
        }

        // otherwise it is a special irrlicht gui event hack
        else if( event.EventType == EET_GUI_EVENT )
        {
            switch( event.GUIEvent.EventType )
            {
            case EGET_MENU_ITEM_SELECTED:
                return HandleMenuItemSelected(event);

            case EGET_FILE_SELECTED:
                return HandleFileSelect(event);
                
            case EGET_SCROLL_BAR_CHANGED:
                return HandleScrollBarChanged(event);
                
            case EGET_CHECKBOX_CHANGED:
                return HandleCheckBoxChanged(event);
                
            case EGET_FILE_CHOOSE_DIALOG_CANCELLED:
                return HandleFileChooseDialogCancelled(event);
                
            default:
                return false;
            }
        }
        else
        {
            return false;
        }
	}
   

	/// Remove all the gui containers from our manager
	void GuiManager::RemoveAll()
	{
		GuiBaseMap::iterator itr = mGuiBases.begin();
        GuiBaseMap::iterator end = mGuiBases.end();
		
		// remove each from the scene
		// Note: We can't do this in the destructor due to
		// pure virtual calls (static binding in ctor/dtor)
		for( ; itr != end; ++itr )
			itr->second->removeFromScene();

        // now clear out the map
		mGuiBases.clear();
        mGuiNames.clear();
	}

	/**
	 * Remove an individual element from the list of containers
	 * @param guiId the id of the element to remove
	*/
	void GuiManager::Remove( uint32_t guiId )
	{            
		GuiBaseMap::iterator itr = mGuiBases.find(guiId);
		if( itr != mGuiBases.end() )
		{
            Assert( itr->second );

            // remove any children
            const core::list<IGUIElement*>& children = itr->second->getGuiElement()->getChildren();
            core::list<IGUIElement*>::ConstIterator citr = children.begin();
            core::list<IGUIElement*>::ConstIterator cend = children.end();

            for( ; citr != cend; ++citr )
            {   
                Remove( (*citr)->getID() );
            }            

            // remove ourselves
            itr->second->removeFromScene();
			
            // remove from name and id map
            mGuiNames.erase(itr->second->getName());
			mGuiBases.erase(itr);
		}
	}

    void GuiManager::Remove( const std::string& name )
    {
        GuiBasePtr elem = getElement(name);
        if( elem ) Remove( elem->getId() );        
    }

    /// Determines if the gui system has input focus currently
    /// @return true if the gui system has focus
    bool GuiManager::HasFocus() const
    {
        return mFocusedContainer != NULL;
    }


    /// Get the number of elements contained in the manager
    /// @return number of container elements
    uint32_t GuiManager::getNumElements() const
    {
        return (uint32_t)mGuiBases.size();
    }

    /// Get an element by its id
    /// @param id the id to search for
    /// @return a ptr to the element or NULL if invalid
    GuiBasePtr GuiManager::getElement( int32_t id ) const
    {
        GuiBaseMap::const_iterator itr = mGuiBases.find(id);
        return ( itr != mGuiBases.end() ) ? itr->second : GuiBasePtr();
    }

    /// Get an element by its name
    /// @param name of the element
    /// @return a ptr to the element or NULL if invalid
    GuiBasePtr GuiManager::getElement( const std::string& name ) const
    {
        GuiManager* pThis = const_cast<GuiManager*>(this);
        return pThis->getElement(name);
    }

    GuiBasePtr GuiManager::getElement( const std::string& name )
    {
        GuiNameMap::const_iterator itr = mGuiNames.find(name);
        int32_t id = ( itr == mGuiNames.end() ) ? -1 : itr->second;
        return ( id < 0 ) ? GuiBasePtr() : getElement(id);        
    }

    // Override the transparency level of all of the gui elements.
    // @param alpha an alpha value in [0,1] how visible the elements should be. 0 means not visible, 1 means full opaque
    void GuiManager::setGuiTransparency( float32_t alpha )
    {
        // calculate the alpha value
        const u32 uAlpha = (u32)( alpha * 255 );

        // set the alpha value
        for (u32 i=0; i<EGDC_COUNT ; ++i)
		{
			SColor col = mIrr.getGuiEnv()->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
			col.setAlpha(uAlpha);
			this->mIrr.getGuiEnv()->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
		}  
    }

    // Set the font for the current skin to use
    // @param fontPath relative the path to the desired font
    // @return true if the load was successful
    bool GuiManager::setFont( const std::string& fontPath )
    {   
        // try to load the font
        IGUIFont* newFont = mIrr.getGuiEnv()->getFont( Kernel::findResource(fontPath).c_str() );

        // set it if load was successful
        if( newFont )
        {
            // get the skin to modify
            IGUISkin* skin = mIrr.getGuiEnv()->getSkin();
            if( skin )
            {
                skin->setFont( newFont );
                return true;
            }
        }

        // no luck
        return false;            
    }
	
    // Open up a file chooser dialog to get a filename from the user
    // @param dialogTitle the title to put in the dialog
    // @param execObj the python function to call when we get a filename
    void GuiManager::openFileChooserDialog( const std::string& dialogTitle, bool modal, PythonExecObject execObj )
    {
        // setup our callback structure
        mFileChooser.mWaitingForFilename    = true;
        mFileChooser.mDialogID              = AllocateGuiId();
        mFileChooser.mDialogTitle           = dialogTitle;
        mFileChooser.mCallback              = execObj;        
        mFileChooser.mDialogOpen            = true;

        // convert the title
        std::wstring wTitle = boost::lexical_cast<std::wstring>(dialogTitle.c_str());

        // open the dialog
        mIrr.getGuiEnv()->addFileOpenDialog( wTitle.c_str(), modal, 0, mFileChooser.mDialogID );
    }

    // Checks if a file chooser dialog is open
    // @return true if a dialog is open; false otherwise
    bool GuiManager::isOpenFileChooserDialog() const
    {
        return mFileChooser.mDialogOpen;
    }

    // Create an element out of one of our factories of the given type
    // @param type the type of element to create
    // @return a ptr to a new element or NULL if failed
    GuiBasePtr GuiManager::createElement( const std::string& type )
    {
        // check if we have a factory for this element type
        GuiElementFactoryMap::iterator itr = mElementFactoryMap.find(type);

        if( itr != mElementFactoryMap.end() )
        {
            // get the factory
            IGuiElementFactoryPtr pFac = itr->second;
            Assert( pFac );

            // create the element
            GuiBasePtr guiElem = pFac->CreateElement( AllocateGuiId() );
            Assert( guiElem );

            // add the element to our system
            addBase( guiElem->getId(), guiElem );

            // return the new element
            return guiElem;
        }

        else
        {
            LOG_ERROR( "Failed to find factory to generate gui element of type " << type );
            return GuiBasePtr();
        }
    }

    // Register an element factory with the system
    // @param facKey the key type of the factory (ie: button, image, text)
    // @param fac the factory that does the creation
    void GuiManager::RegisterElementFactory( const std::string& facKey, IGuiElementFactoryPtr fac )
    {
        Assert(fac);
        mElementFactoryMap[facKey] = fac;
        fac->RegisterResources( &mElementFacRes );
    }

    // Remove an element factory from the system
    // @param facKey the key to remove by
    void GuiManager::RemoveElementFactory( const std::string& facKey )
    {
        GuiElementFactoryMap::iterator itr = mElementFactoryMap.find(facKey);
        if( itr != mElementFactoryMap.end() )
            mElementFactoryMap.erase(itr);
    }

    void GuiManager::ClearElementFactories()
    {
        mElementFactoryMap.clear();
    }   

    GuiBasePtr GuiManager::addGuiBase( uint32_t id, GuiBasePtr base )
    {
        return addBase(id,base);        
    }    

    /// Get the next element from the gui id container to assure that all elements are unique
    /// @return next unique identifier
    uint32_t GuiManager::AllocateGuiId()
    {
		Assert( mGuiIdCounter > 0 );
        return mGuiIdCounter++;
    }

    void GuiManager::RegisterDefaultElementFactories()
    {
        RegisterElementFactory( "button",       IGuiElementFactoryPtr( new GuiButtonElementFactory ) );
        RegisterElementFactory( "scroll bar",   IGuiElementFactoryPtr( new GuiScrollBarElementFactory ) );
        RegisterElementFactory( "horizontal scroll bar",   IGuiElementFactoryPtr( new GuiHorizontalScrollBarElementFactory ) );
        RegisterElementFactory( "combo box",    IGuiElementFactoryPtr( new GuiComboBoxElementFactory ) );
        RegisterElementFactory( "edit box",     IGuiElementFactoryPtr( new GuiEditBoxElementFactory ) );
        RegisterElementFactory( "check box",    IGuiElementFactoryPtr( new GuiCheckBoxElementFactory ) );
        RegisterElementFactory( "text",         IGuiElementFactoryPtr( new GuiTextElementFactory ) );
        RegisterElementFactory( "image",        IGuiElementFactoryPtr( new GuiImageElementFactory ) );
        RegisterElementFactory( "window",       IGuiElementFactoryPtr( new GuiWindowElementFactory ) );
        RegisterElementFactory( "modal window", IGuiElementFactoryPtr( new GuiWindowElementFactoryModal ) );
        RegisterElementFactory( "context menu", IGuiElementFactoryPtr( new GuiContextMenuElementFactory ) );
    }

    /// Attempt to handle a mouse click by looking for receivers of click events
    /// @param event the event passed to us by irrlicht
    /// @return true if we were able to handle the event
    bool GuiManager::HandleMouseClick( const SEvent& event )
    {
        Assert( event.EventType == EET_MOUSE_INPUT_EVENT );

        // try to intersect the point with the environment
        const position2d<int32_t> pos2D( event.MouseInput.X, event.MouseInput.Y );

        // get a list of intersecting elements
        GuiBaseList guiList = findContainerByPosition( pos2D );        
        
        // handle mouse down events
        switch( event.MouseInput.Event )
        {

        // Note: Let this pass down!
        case EMIE_LMOUSE_PRESSED_DOWN:
        case EMIE_RMOUSE_PRESSED_DOWN:
        case EMIE_MMOUSE_PRESSED_DOWN:
            {
                // clear our focused container
                if( mFocusedContainer )
                {
                    mFocusedContainer->removeFocus();
                    mFocusedContainer.reset();
                }

                // if we did not hit anything gui related, lose focus
                if( guiList.size() == 0 )
                    return false;                

                GuiBaseList::iterator itr = guiList.begin();
                GuiBaseList::iterator end = guiList.end();

                // look at each of the intersected elements
                for( ; itr != end; ++itr )
                {
                    // assign the current gui to the current itr
                    GuiBasePtr gui = *itr;

                    if( mFocusedContainer )
                        mFocusedContainer->removeFocus();

                    // can this base be focused on?                    
                    mFocusedContainer = gui->CanGainFocus() ? gui : mFocusedContainer;

                    if( mFocusedContainer )
                        mFocusedContainer->setFocus();

                    // check if we have pressed on any receivers
                    mPressedContainer = FindProperReceiver(event,gui);

                    if( mPressedContainer )                    
                        return true;
                }

                // did not find any valid press-able containers
                mPressedContainer.reset();
                return false;
            }
        default: break;
        }        
        
        // handle mouse up events if the pressed container is under the mouse
        if( std::find( guiList.begin(), guiList.end(), mPressedContainer ) != guiList.end() )
        {
            GuiBasePtr gui = mPressedContainer;

            switch( event.MouseInput.Event )
		    {
            case EMIE_LMOUSE_LEFT_UP: mPressedContainer.reset(); return gui->OnMouseLeftClick(/*event*/);
		    case EMIE_RMOUSE_LEFT_UP: mPressedContainer.reset(); return gui->OnMouseRightClick(/*event*/);
		    case EMIE_MMOUSE_LEFT_UP: mPressedContainer.reset(); return gui->OnMouseMiddleClick(/*event*/);
		    default: break;
		    }
        }		

        // nothing received
        return false;
    }
    
    /// Attempt to handle a mouse movement by looking for receivers of move events
    /// @param event the event passed to us by irrlicht
    /// @return true if we were able to handle the event
    bool GuiManager::HandleMouseMove( const SEvent& event )
    {
        Assert( event.EventType == EET_MOUSE_INPUT_EVENT );

        switch( event.MouseInput.Event )
        {        
        case EMIE_MOUSE_MOVED:
	        {
                // try to intersect the point with the environment
	            const position2d<int32_t> pos2D( event.MouseInput.X, event.MouseInput.Y );        

                // find all the elements that intersect this element
                GuiBaseList guiList = findContainerByPosition( pos2D );
                GuiBaseList::iterator itr = guiList.begin();
                GuiBaseList::iterator end = guiList.end();

                // look through all of the found elements searching for one that works
                for( ; itr != end; ++itr )
                {
                    if( (*itr)->RespondsTo( GuiBase::kResponse_OnMouseMove ) )
                        return (*itr)->OnMouseMove( /*event*/ );                    
                }
	        }
        default:
            return false;
        }
    }

    bool GuiManager::HandleFileSelect( const SEvent& event )
    {
        Assert( event.EventType == EET_GUI_EVENT );
        Assert( event.GUIEvent.EventType == EGET_FILE_SELECTED );             
		
        // get the calling file dialog
		IGUIFileOpenDialog* dialog = (IGUIFileOpenDialog*)event.GUIEvent.Caller;
        Assert( dialog );

        // if we are waiting for a file, process it
        if( mFileChooser.mWaitingForFilename && mFileChooser.mDialogID == dialog->getID() )
        {
            mFileChooser.mWaitingForFilename = false;
            mFileChooser.mDialogOpen = false;

            // attempt to execute the callback with the filename
            try
            {
                if( mFileChooser.mCallback.ptr() != Py_None )
                {   
                    boost::python::call<void,std::string>(mFileChooser.mCallback.ptr(), wstring_to_string(dialog->getFileName()));
                }
            }
            catch (boost::python::error_already_set const &)
            {
                ScriptingEngine::instance().LogError();
                return false;
            }

            return true;
        }

        return false;
    }

    bool GuiManager::HandleFileChooseDialogCancelled( const SEvent& event )
    {
        Assert( event.EventType == EET_GUI_EVENT );
        Assert( event.GUIEvent.EventType == EGET_FILE_CHOOSE_DIALOG_CANCELLED );             
		
        // get the calling file dialog
		IGUIFileOpenDialog* dialog = (IGUIFileOpenDialog*)event.GUIEvent.Caller;
        Assert( dialog );

        // if we are waiting for a file, set flag indicating the dialog as closed.
        if( mFileChooser.mWaitingForFilename && mFileChooser.mDialogID == dialog->getID() )
        {
            mFileChooser.mWaitingForFilename = false;
            mFileChooser.mDialogOpen = false;
            return true;
        }

        return false;
    }

    bool GuiManager::HandleMenuItemSelected( const SEvent& event )
    {
        Assert( event.EventType == EET_GUI_EVENT );
        Assert( event.GUIEvent.EventType == EGET_MENU_ITEM_SELECTED );
        Assert( event.GUIEvent.Caller );

        IGUIContextMenu* menu = static_cast<IGUIContextMenu*>(event.GUIEvent.Caller);
        s32 id = menu->getItemCommandId(menu->getSelectedItem());

        // get the element we clicked on
        GuiBasePtr elem = findContainerById( id );

        if( elem && elem->RespondsTo( GuiBase::kResponse_OnMouseLeftClick ) )
        {
            // execute it's action
            elem->OnMouseLeftClick( /*event*/ );
        }

        return true;
    }

    bool GuiManager::HandleScrollBarChanged( const SEvent& event )
    {
        Assert( event.EventType == EET_GUI_EVENT );
        Assert( event.GUIEvent.EventType == EGET_SCROLL_BAR_CHANGED );
        Assert( event.GUIEvent.Caller );

        IGUIScrollBar* scroll_bar = static_cast<IGUIScrollBar*>(event.GUIEvent.Caller);
        s32 id = scroll_bar->getID();

        // get the target element
        GuiBasePtr elem = findContainerById( id );

        if( elem && elem->RespondsTo( GuiBase::kResponse_OnScrollBarChange ) )
        {
            // execute it's action
            elem->OnScrollBarChange( /*event*/ );
        }

        return true;
    }
    
    bool GuiManager::HandleCheckBoxChanged( const SEvent& event )
    {
        Assert( event.EventType == EET_GUI_EVENT );
        Assert( event.GUIEvent.EventType == EGET_CHECKBOX_CHANGED );
        Assert( event.GUIEvent.Caller );
        
        IGUICheckBox* check_box = static_cast<IGUICheckBox*>(event.GUIEvent.Caller);
        s32 id = check_box->getID();
        
        GuiBasePtr elem = findContainerById(id);
        
        if (elem && elem->RespondsTo(GuiBase::kResponse_OnCheckBoxChange))
        {
            elem->OnCheckBoxChange( /* event */ );
        }
        else
        {
            return false;
        }
        
        return true;
    }

    bool GuiManager::ModifyRegisteredElementName( const std::string& oldName, const std::string& newName )
    {
        // try to lookup the old name and new name checking
        // for existence and collision
        GuiNameMap::iterator oldItr = mGuiNames.find(oldName);
        GuiNameMap::iterator newItr = mGuiNames.find(newName);

        if( oldItr != mGuiNames.end() && newItr == mGuiNames.end() )
        {
            // if it exists, replace it with the new name
            mGuiNames[newName] = oldItr->second;
            mGuiNames.erase(oldItr);
            return true;
        }

        // did not meet the requirements
        return false;        
    }    

    void GuiManager::SetFocusStatus( GuiBasePtr element, bool focused )
    {
        Assert(element);
        Assert(element->getIrrGuiElement());
        Assert(mIrr.getGuiEnv());

        if( focused )
        {
            mIrr.getGuiEnv()->setFocus(element->getIrrGuiElement());
            mFocusedContainer = element;
        }
        else
        {
            mIrr.getGuiEnv()->removeFocus(element->getIrrGuiElement());
            mFocusedContainer.reset();
        }
    }

    /// Try to find a receiver for an event that can properly handle it by traversing a gui tree
    /// @param event the event to pass to our gui elements
    /// @param startGui the first node in the gui tree to test
    /// @return a ptr to the receiver or NULL if none found
    GuiBasePtr GuiManager::FindProperReceiver( const SEvent& event, GuiBasePtr startGui )
    {
        GuiBasePtr gui = startGui;

        // make sure this is a mouse event
        Assert( event.EventType == EET_MOUSE_INPUT_EVENT );

        typedef bool (GuiBase::*ResponseFunctionPtr)( /*SEvent*/ );

        GuiBase::ResponseFlag flagDown = GuiBase::kResponse_OnMouseLeftDown;
        GuiBase::ResponseFlag flagClick = GuiBase::kResponse_OnMouseLeftClick;
        ResponseFunctionPtr   response = NULL;

        // determine the kind of receiver we are looking for
        switch( event.MouseInput.Event )
        {
            case EMIE_LMOUSE_PRESSED_DOWN:
                flagDown  = GuiBase::kResponse_OnMouseLeftDown;
                flagClick = GuiBase::kResponse_OnMouseLeftClick;
                response  = &GuiBase::OnMouseLeftDown;
                break;

            case EMIE_RMOUSE_PRESSED_DOWN:
                flagDown  = GuiBase::kResponse_OnMouseRightDown;
                flagClick = GuiBase::kResponse_OnMouseRightClick;
                response  = &GuiBase::OnMouseRightDown;
                break;

            case EMIE_MMOUSE_PRESSED_DOWN:
                flagDown  = GuiBase::kResponse_OnMouseMiddleDown;
                flagClick = GuiBase::kResponse_OnMouseMiddleClick;
                response  = &GuiBase::OnMouseMiddleDown;
                break;

            default: AssertMsg( false, "FindProperReceiver() called with improper eventType - " << event.MouseInput.Event );

        }

		// loop until we find someone to handle this
		while( gui )
		{
            if( gui->RespondsTo( flagDown ) || gui->RespondsTo( flagClick ) )
            {   
                // call the response function on the instance
                ((*gui).*response)();
                return gui;
            }            
            
            // we did not find a proper receiver, climb the tree
			gui = findContainerById( gui->getParentId() );							
		}

        // did not find a receiver
        return GuiBasePtr();
    }

    /**
     * Add a gui base to the cache
     * @param id the id of the container to use
     * @param base the gui base to add
     * @return a ptr to the container added, NULL if failed
    */
	GuiBasePtr GuiManager::addBase( uint32_t id, GuiBasePtr base )    
    {
		if( !base )
			return GuiBasePtr();

        // assign the manager proxy
        base->setManagerProxy(mpProxy);

        // set the factory
        base->setSimFactory(mpSimFac);
        
        // add to the id map
        mGuiBases[id] = base;

        // if no name was provided, give a default name
        if( base->getName() == "" )
        {
            std::stringstream sstr;
            sstr << "unnamed_" << id;
            base->setName( sstr.str() );
        }
        
        // add that name to the map for referral
        mGuiNames[base->getName()] = id;
        
        return base;
    }    

	/**
	 * Find a gui container by a position. This position should be inside the rectangle defined
	 * by the container or inside one of the container's children. We can then use this id
	 * to look up the child container. Since everything should be mapped through this manager,
	 * not finding the id would be A Very Bad Thing (tm).
	 *
	 * @param pos2D the point to check for intersection with our containers
	 * @return a list of elements that best intersect this position
	*/
    GuiManager::GuiBaseList GuiManager::findContainerByPosition( const Pos2i& pos2D )
    {	    
	    GuiBaseMap::iterator itr = mGuiBases.begin();
        GuiBaseMap::iterator end = mGuiBases.end();		

        GuiBaseList resultList;

		// iterate through all of our elements look for something
		// that intersects this position
        for( ; itr != end; ++itr )
        {
            // get the id of the collided container
            int32_t id = findBestCollision( itr->second, pos2D );

			// if we get a valid id back from the container
            if( id > 0 )                        
			{
				// find the container that corresponds to the id
				GuiBaseMap::iterator citr = mGuiBases.find( (uint32_t)id );                				
                
                // somehow a gui container has been mapped with an id that is not stored
			    // in our cache, this is not good.
			    AssertMsg( citr != mGuiBases.end(), "Hit an id that is not mapped: " << id );                
				
				// add this element to the list of result elements
				AssertMsg( citr->second, "We are storing an invalid gui container ptr!" );
                resultList.push_back(citr->second);
			}
        }

		// nothing intersected with this position, give up
        return resultList;
    }

	/**
	 * Find a container object by its assigned id
	 * @param id the id to search for
	 * @return a ptr to the element or NULL if not found
	*/
	GuiBasePtr GuiManager::findContainerById( int32_t id )
	{
		GuiBaseMap::iterator itr = mGuiBases.find(id);
		if( itr != mGuiBases.end() )
			return itr->second;

		return GuiBasePtr();
	}

    // Retrieves the id of the gui element that best fits this point. If a parent encloses this
    // point and so does it's child, we will return the child id, not the parent id. 
    // @param c the parent gui container to traverse
    // @param pos2D the position to intersect with the gui elements
    // @return the id of the gui intersected, -1 if none
    int32_t GuiManager::findBestCollision( GuiBasePtr c, const Pos2i& pos2D )
    {
        Assert( c );
		Assert( c->getIrrGuiElement() );
        
        // ask irrlicht
		IGUIElement* elem = c->getIrrGuiElement()->getElementFromPoint( pos2D );
		if( elem )
			return elem->getID();		

        // no luck
        return -1;
    }


} //end OpenNero

