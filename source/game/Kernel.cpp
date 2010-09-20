//--------------------------------------------------------
// OpenNero : Kernel
//  main kernel of the application
//--------------------------------------------------------

#include "core/Common.h"
#include "game/Kernel.h"
#include "game/SimContext.h"
#include "game/Mod.h"
#include "scripting/scriptIncludes.h"
#include "gui/GuiEditBox.h"
#include "gui/GuiManager.h"
#include "core/Preprocessor.h"

namespace OpenNero
{


	/// singleton accessor
	Kernel& Kernel::instance()
	{
		return boost::details::pool::singleton_default<Kernel>::instance();
	}

	/// const singleton accessor
	const Kernel& Kernel::const_instance()
	{
		return boost::details::pool::singleton_default<Kernel>::instance();
	}

	/**
     * Convert a mod-relative path to its filesystem path (not necessarily absolute)
	 * @param path the path to mangle
     * @param ensure_existence make sure the file is present
	 * @return the filesystem-relative path for the resource or empty string
	*/
	std::string Kernel::findResource( const std::string& path, bool ensure_existence)
	{
		std::string result;
        if (ensure_existence) {
            if (!Kernel::instance().getMod()->FindResource(path, result)) {
                AssertMsg(false, "Could not locate resource " << path);
                LOG_F_ERROR("game", "Could not locate user resource " << path);
                return result;
            }
        }
        else
        {
            if (!Kernel::instance().getMod()->FindUserResource(path, result)) {
                AssertMsg(false, "Could not locate user resource " << path);
                LOG_F_ERROR("game", "Could not locate user resource " << path);
                return result;
            }
        }
		return result;
	}

    /// set the resource search path of the current mod ( separated by ':' )
    void Kernel::setModPath( const std::string& path )
    {
        mCurMod->SetPath(path);
    }

    /// get the resource search path of the current mod ( separated by ':' )
    std::string Kernel::getModPath()
    {
        return mCurMod->GetPath();
    }

    /// Get the current mod
    /// @return reference to current mod
    ModPtr Kernel::GetMod()
    {
        return instance().getMod();
    }

    /// Get the current sim context
    /// @return shared ptr to current sim context
    SimContextPtr Kernel::GetSimContext()
    {
        return GetMod()->context;
    }

	/// ctor
	Kernel::Kernel()
        : mIrrDevice()
        , mCurMod(new Mod())
        , mTransitionInfo()
#if NERO_ALLOW_DEV_CONSOLE
        , mConsole()
        , mIOState()
        , mBacktrackIdx()
        , mBacktrackVector()
        , mArgc(0)
        , mArgv(NULL)
#endif
        , mAppConfig()
    {}

	/// dtor - flush the current mod
	Kernel::~Kernel()
	{
		flushCurrentMod();
	}

    // handle an event
    bool Kernel::OnEvent(const irr::SEvent& event)
    {
#if NERO_ALLOW_DEV_CONSOLE
        if( CheckConsoleAction(event) )
            return true;
#endif

        if( mCurMod->context )
            return mCurMod->context->HandleEvent(event);

        return false;
    }

    bool Kernel::Initialize( IrrlichtDevice_IPtr device, const AppConfig& appConfig, int argc, char** argv )
    {
        Assert(device);
        setIrrDevice(device);
        mAppConfig = appConfig;
        mArgc = argc;
        mArgv = argv;
        return true;
    }

	/// current mod accessor
    ModPtr Kernel::getMod()
	{
		return mCurMod;
	}

	/// const current mod accessor
	const ModPtr Kernel::getMod() const
	{
		return mCurMod;
	}

    void Kernel::ProcessTick()
    {
        // do we need to perform a transition?
        if( mTransitionInfo.mActive )
        {
            mTransitionInfo.mActive = false;
            switchMod( mTransitionInfo.mDevice, mTransitionInfo.mNewModName, mTransitionInfo.mNewModDir );
        }

        // tick the current context
        if( mCurMod->context )
        {
            Assert( mIrrDevice );

            // get the current time
            static uint32_t prevTime = GetStaticTimer().getMilliseconds();
            uint32_t        curTime  = GetStaticTimer().getMilliseconds();

	        mCurMod->context->ProcessTick( (curTime - prevTime)/1000.0f );

            prevTime = curTime;

            // yield to other processes
            mIrrDevice->yield();
        }

#if NERO_ALLOW_DEV_CONSOLE
        mIOState.PrepareForNextSimFrame();
#endif
    }


	/// clear the current mod, calling onPop on the context
	void Kernel::flushCurrentMod()
	{
        // clear the context if needed
		if( mCurMod->context )
		{
			mCurMod->context->onPop();
			mCurMod->context.reset();
		}

		mCurMod->name = "";
		mCurMod->SetPath("");
	}

    /// Switch to a new context
    SimContextPtr Kernel::switchMod( const std::string& name, const std::string& path )
    {
        Assert( mIrrDevice );
        return switchMod( mIrrDevice, name, path );
    }

	/**
	 * Switch from one context to another, calls onPush on the context
	 * @param device the irrlicht device to use in the context
	 * @param name the name of the mod
	 * @param path the mod path for loading files
	 * @return a ptr to the new sim context
	*/
	SimContextPtr Kernel::switchMod( IrrlichtDevice_IPtr device, const std::string& name, const std::string& path )
	{
        // save the device
        setIrrDevice(device);

		// kill the old mod if loaded
		flushCurrentMod();

        if( mCurMod->context )
        {
            LOG_F_MSG( "game", "Leaving mod " << mCurMod->name );
        }

		// setup our new mod
        SimContextPtr simContext( new SimContext );
        if( !simContext->Initialize(device) )
            simContext.reset();

		mCurMod->context = simContext;
		mCurMod->name = name;
		mCurMod->SetPath(path);

        // import as a scripting module
        bool modImported = ScriptingEngine::instance().ImportModule(name);
        AssertMsg( modImported, "Could not import mod " << name << " into the scripting engine" );

        AssertMsg( mCurMod->context, "Failed to create SimContext" );
        AssertMsg( device, "Failed to initialize rendering device" );

		// save the device
		mIrrDevice    = device;

        if( mCurMod->context )
        {
            LOG_F_MSG( "game", "Switching to mod " << name );

		    // let the context initialize itself (and pass along the command line arguments)
		    mCurMod->context->onPush(mArgc, mArgv);

#if NERO_ALLOW_DEV_CONSOLE
            // inject our console to the gui manager
            InjectConsole();
#endif
        }

		// return the new context
		return mCurMod->context;
	}

    void Kernel::RequestModSwitch( const std::string& name, const std::string& path )
    {
        Assert( mIrrDevice );
        RequestModSwitch( mIrrDevice, name, path );
    }

    void Kernel::RequestModSwitch( IrrlichtDevice_IPtr device, const std::string& name, const std::string& path )
    {
        mTransitionInfo.mActive = true;
        mTransitionInfo.mDevice = device;
        mTransitionInfo.mNewModName = name;
        mTransitionInfo.mNewModDir = path;
    }

    /// get the irrlicht device
	IrrlichtDevice_IPtr Kernel::getIrrDevice() const
	{
        Assert( mIrrDevice );
		return mIrrDevice;
	}

    /// set the irrlicht device
    void Kernel::setIrrDevice( IrrlichtDevice_IPtr dev )
    {
        Assert( dev );
        mIrrDevice = dev;
        mIrrDevice->setEventReceiver(this);
    }

#if NERO_ALLOW_DEV_CONSOLE
    void Kernel::InjectConsole()
    {
        Assert( mIrrDevice );

        if( mCurMod->context && mCurMod->context->GetGuiManager() )
        {
            // save the visibility state and text from the prev mod
            std::string sText = "";
            bool visible = false;
            bool focused = false;

            if( mConsole )
            {
                sText = mConsole->getSText();
                visible = mConsole->isVisible();
                focused = mConsole->isFocused();
            }

            // create a console piece to interject into the scene
            IGUIEnvironment* guiEnv = mIrrDevice->getGUIEnvironment();
            IGUIEditBox* irrEditBox = guiEnv->addEditBox( L"", Rect2i( 0,0, 450, 25 ), true, NULL, GuiManager::AllocateGuiId() );
            mConsole.reset( new GuiEditBox );
            mConsole->setGuiElement( irrEditBox );

            // restore the state
            mConsole->setVisible(visible);
            mConsole->setText(sText);

            // add it to the current gui manager
            mCurMod->context->GetGuiManager()->addGuiBase( mConsole->getId(), mConsole );

            // set the focused if previously was
            if( focused )
                mConsole->setFocus();
        }
    }

    bool Kernel::CheckConsoleAction( irr::SEvent event)
    {
        // check for console action
        if( event.EventType == EET_GUI_EVENT )
        {
            if( mConsole && event.GUIEvent.EventType == EGET_EDITBOX_ENTER )
            {
                if( mConsole->getId() == event.GUIEvent.Caller->getID() )
                {
                    // clear out the console
                    std::string cmd = mConsole->getSText();
                    mConsole->setText( "" );

                    // assumes scripting engine is initialized
                    ScriptingEngine::instance().Exec(cmd);

                    // reset the backtrack index
                    mBacktrackVector.push_back(cmd);
                    mBacktrackIdx = static_cast<int32_t>( mBacktrackVector.size() );

                    return true;
                }
            }
        }

        if( event.EventType == EET_KEY_INPUT_EVENT )
        {
            // track keyboard events
            mIOState.onIrrEvent(event);

            // Note: jbsheblak 15Nov2007
            // Irrlicht doesn't have a key code for Tilde so we need to
            // use an alternative method here
        #if NERO_PLATFORM_WINDOWS
            wchar_t tildeKey = L'`';
        #elif NERO_PLATFORM_LINUX
            wchar_t tildeKey = 96;
        #elif NERO_PLATFORM_MAC
            wchar_t tildeKey = L'`';
        #else
            #error "Unsupported platform, cannot assign key."
        #endif

            if( event.KeyInput.Char == tildeKey && event.KeyInput.PressedDown )
            {
                // toggle visibility
                mConsole->setVisible( !mConsole->isVisible() );

                // if it is now visible, set the focus
                if( mConsole->isVisible() )
                    mConsole->setFocus();

                else
                    mConsole->removeFocus();

                return true;
            }

            if( mIOState.WasKeyPressedLastFrame( (KEY)(KEY_UP) ) )
            {
                if( mConsole->isVisible() && mConsole->isFocused() && mBacktrackIdx > 0 )
                {
                    --mBacktrackIdx;
                    mConsole->setText( mBacktrackVector[mBacktrackIdx] );
                    return true;
                }
            }

            if( mIOState.WasKeyPressedLastFrame( (KEY)(KEY_DOWN) ) )
            {
                if( mConsole->isVisible() && mConsole->isFocused() && mBacktrackIdx < (uint32_t)mBacktrackVector.size()-1 )
                {
                    ++mBacktrackIdx;
                    mConsole->setText( mBacktrackVector[mBacktrackIdx] );
                    return true;
                }
            }
        }

        // no console action
        return false;
    }
#endif // end NERO_ALLOW_DEV_CONSOLE

    Kernel::TransitionInfo::TransitionInfo()
        : mActive(false)
    {}

    /// request a switch to a new mod with the specified mod path
    void switchMod( const std::string& modName, const std::string& modDir )
    {
        Kernel::instance().RequestModSwitch(modName,modDir);
    }

    /// convert mod-relative path to filesystem path
    std::string findResource(const std::string& path)
    {
        return Kernel::instance().findResource(path);
    }

    std::string getModPath()
    {
        return Kernel::instance().getModPath();
    }

    void setModPath(const std::string& path)
    {
        Kernel::instance().setModPath(path);
    }

    PYTHON_BINDER( Kernel )
    {
        def( "switchMod", &switchMod, "Switch the kernel to a new mod");
        def( "findResource", &findResource, "Convert mod-relative path to filesystem path");
        def( "getModPath", &getModPath, "get the resource search path of the current mod ( separated by ':' )");
        def( "setModPath", &setModPath, "set the resource search path of the current mod ( separated by ':' )");
    }

} //end OpenNero
