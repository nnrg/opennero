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
		static Kernel kernel;
		return kernel;
	}

	/// const singleton accessor
	const Kernel& Kernel::const_instance()
	{
		return instance();
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
                LOG_F_ERROR("game", "Could not locate resource " << path);
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
        , mAppConfig()
    {}

	/// dtor - flush the current mod
	Kernel::~Kernel()
	{
		flushCurrentMod();
        ScriptingEngine::instance().destroy();
	}

    // handle an event
    bool Kernel::OnEvent(const irr::SEvent& event)
    {

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
            switchMod( mTransitionInfo.mDevice, mTransitionInfo.mNewModName, mTransitionInfo.mNewModMode, mTransitionInfo.mNewModDir );
            mTransitionInfo.mDevice.reset();
        }

        // tick the current context
        if( mCurMod->context )
        {
            Assert( mIrrDevice );

            // get the current time
            static uint32_t prevTime = GetStaticTimer().getMilliseconds();
            static uint32_t prevFullFrameTime = GetStaticTimer().getMilliseconds();
            uint32_t curTime  = GetStaticTimer().getMilliseconds();
            float32_t dt = (curTime - prevTime)/1000.0f; // frame length in seconds
            float32_t fullDT = (curTime - prevFullFrameTime)/1000.0f; // full frame length
            float32_t frameDelay = mCurMod->context->GetFrameDelay(); // expected frame delay
            
            if (fullDT >= frameDelay) {
                mCurMod->context->ProcessTick(dt);
                prevFullFrameTime = curTime;
            } else {
                mCurMod->context->ProcessAnimationTick(dt, fullDT/frameDelay);
            }
            prevTime = curTime;
        }

    }

    /// @param caption the part of the window title after OpenNero - ModName
    void Kernel::SetWindowCaption(const std::string& caption) 
    {
        if (mCurMod) {
            std::string full_caption;
            if (caption.empty()) {
                full_caption = GetAppConfig().Title + " - " + mCurMod->name;
            } else {
                full_caption = GetAppConfig().Title + " - " + mCurMod->name + " - " + caption;
            }
            mIrrDevice->setWindowCaption(core::stringw(full_caption.c_str()).c_str());
        } else {
            LOG_ERROR("Could not SetWindowCaption, no mod loaded!");
        }
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
    SimContextPtr Kernel::switchMod( const std::string& name, const std::string& mode, const std::string& path )
    {
        Assert( mIrrDevice );
        return switchMod( mIrrDevice, name, mode, path );
    }

	/**
	 * Switch from one context to another, calls onPush on the context
	 * @param device the irrlicht device to use in the context
	 * @param name the name of the mod
	 * @param path the mod path for loading files
	 * @return a ptr to the new sim context
	*/
	SimContextPtr Kernel::switchMod( IrrlichtDevice_IPtr device, const std::string& name, const std::string& mode, const std::string& path )
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
		mCurMod->mode = mode;
		mCurMod->SetPath(path);

        AssertMsg( mCurMod->context, "Failed to create SimContext" );
        AssertMsg( device, "Failed to initialize rendering device" );

        if( mCurMod->context )
        {
            LOG_F_MSG( "game", "Switching to mod " << name );

		    // let the context initialize itself (and pass along the command line arguments)
		    mCurMod->context->onPush(mArgc, mArgv);
            
            // import as a scripting module
            bool modImported = ScriptingEngine::instance().ImportModule(name);
            AssertMsg( modImported, "Could not import mod " << name << " into the scripting engine" );

        }
        
        SetWindowCaption("");

		// return the new context
		return mCurMod->context;
	}

    void Kernel::RequestModSwitch( const std::string& name, const std::string& mode, const std::string& path )
    {
        Assert( mIrrDevice );
        RequestModSwitch( mIrrDevice, name, mode, path );
    }

    void Kernel::RequestModSwitch( IrrlichtDevice_IPtr device, const std::string& name, const std::string& mode, const std::string& path )
    {
        mTransitionInfo.mActive = true;
        mTransitionInfo.mDevice = device;
        mTransitionInfo.mNewModName = name;
        mTransitionInfo.mNewModMode = mode;
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

    Kernel::TransitionInfo::TransitionInfo()
        : mActive(false)
    {}

} //end OpenNero
