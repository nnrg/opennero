//--------------------------------------------------------
// OpenNero entrance

#include "core/Common.h"
#include "tinyxml.h"
#include "core/IrrUtil.h"
#include "core/AssertExt.h"
#include "core/ONTime.h"
#include "game/SimContext.h"
#include "game/Kernel.h"
#include "scripting/scripting.h"

#include "audio/AudioManager.h"
#include "utils/Config.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( SimContext );
    /// @endcond

    /// command line parameters
    typedef std::map<std::string, std::string> CommandLineParams;

    /// value returned on error
    static const int32_t     kErrorReturn           = -1;

    /// entrance into the OpenNero application
    /// @param argc number of arguments passed in
    /// @param argv array of char* arguments passed in
    /// @return 0 if no errors
    int32_t OpenNeroMain( int argc, char** argv )
    {
        using namespace irr;
        using namespace irr::core;

        // turn on our custom logging system
        OpenNero::Log::LogSystemInit();

		// read the application configuration
        AppConfig appConfig = ReadAppConfig(argc, argv, "appConfig.py");

        // configure our log receivers
        LogSpecifyReceivers( argc, argv, appConfig.log_config_file );

        // add loggers
        LOG_MSG( "Starting OpenNero" );

#if NERO_BUILD_AUDIO
        if( !OpenNero::AudioManager::instance().Init() )
        {
            LOG_ERROR( "Failed to initialize audio engine" );
        }
#endif // NERO_BUILD_AUDIO

        // create our video device
        irr::video::E_DRIVER_TYPE driverType = ( appConfig.render_type == "null" ) ? video::EDT_NULL : video::EDT_OPENGL;

        IrrlichtDevice_IPtr irrDevice( createDevice( driverType, dimension2d<u32>( appConfig.window_width, appConfig.window_height),
                                                     appConfig.window_BPP, appConfig.full_screen, appConfig.use_stencil_buffer, appConfig.use_vsync ), false );

    #if NERO_DEBUG
        OpenNero::AssertExt::SetDevice( irrDevice.get() );
    #endif

        Assert(irrDevice);

        // set the window title
        std::wstring wWindowName = boost::lexical_cast<std::wstring>( appConfig.window_title.c_str() );
        irrDevice->setWindowCaption(wWindowName.c_str());
        irrDevice->setResizable(true);

        // initialize our kernel and start the first mod
        OpenNero::Kernel&		kern = OpenNero::Kernel::instance();
        kern.Initialize(irrDevice, appConfig, argc, argv);
        kern.switchMod( irrDevice, appConfig.start_mod_name, appConfig.start_mod_dir );
        if (!appConfig.start_command.empty())
        {
        	ScriptingEngine::instance().Exec(appConfig.start_command);
        }

        // run the loop until the device is killed
        while(irrDevice->run())
            kern.ProcessTick();

#if NERO_BUILD_AUDIO
        // shut down the audio manager
        if( !OpenNero::AudioManager::instance().Shutdown() )
            LOG_ERROR( "Audio library did not shutdown properly" );
#endif // NERO_BUILD_AUDIO

        // flush the current loaded mod
        kern.flushCurrentMod();

        LOG_MSG( "Killing OpenNero" );

    #if NERO_DEBUG
        OpenNero::AssertExt::ReleaseDevice();
    #endif

        // shut down the logger
        OpenNero::Log::LogSystemShutdown();

        return 0;
    }

} // end OpenNero
