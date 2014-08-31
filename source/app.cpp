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

        AppConfig appConfig;
        
        if (!appConfig.ParseCommandLine(argc, argv)) {
            return -1;
        }

        OpenNero::Log::LogSystemInit(appConfig.LogFile);
        
		// read the application configuration
        //AppConfig appConfig = ReadAppConfig(argc, argv, "appConfig.py");

        // configure our log receivers
        LogSpecifyReceivers( argc, argv, appConfig.LogConfigFile );

        // add loggers
        LOG_MSG( "Starting OpenNero" );

        // create our video device
        irr::video::E_DRIVER_TYPE driverType = ( appConfig.RenderType == "null" ) ? video::EDT_NULL : video::EDT_OPENGL;

        IrrlichtDevice_IPtr irrDevice( createDevice( driverType, dimension2d<u32>( appConfig.Width, appConfig.Height),
                                                     appConfig.BPP, appConfig.FullScreen, appConfig.StencilBuffer, appConfig.VSync ), false );

    #if NERO_DEBUG
        OpenNero::AssertExt::SetDevice( irrDevice.get() );
    #endif

        Assert(irrDevice);

        // set the window title
        std::wstring wWindowName = boost::lexical_cast<std::wstring>( appConfig.Title.c_str() );
        irrDevice->setWindowCaption(wWindowName.c_str());
        irrDevice->setResizable(true);

        // initialize our kernel and start the first mod
        OpenNero::Kernel&		kern = OpenNero::Kernel::instance();
        kern.Initialize(irrDevice, appConfig, argc, argv);
        kern.switchMod( irrDevice, appConfig.StartMod, appConfig.StartModMode, appConfig.StartModDir );
        if (!appConfig.StartCommand.empty())
        {
        	ScriptingEngine::instance().Exec(appConfig.StartCommand);
        }

        // run the loop until the device is killed
        while(irrDevice->run())
            kern.ProcessTick();

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
