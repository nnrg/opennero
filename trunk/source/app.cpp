//--------------------------------------------------------
// OpenNero entrance

#include "core/Common.h"
#include "tinyxml.h"
#include "core/IrrUtil.h"
#include "core/AssertExt.h"
#include "core/ONTime.h"
#include "game/SimContext.h"
#include "game/Kernel.h"

#include "audio/AudioManager.h"
#include "utils/Performance.h"
#include "utils/Config.h"

#include "tclap/CmdLine.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( SimContext );
    /// @endcond

    /// command line parameters
    typedef std::map<std::string, std::string> CommandLineParams;

    /// text file to output performance report to
    static const char8_t*    kDefaultPerformanceTxt = "performanceReport.txt";
    /// CSV file to output performance report to
    static const char8_t*    kDefaultPerformanceCsv = "performanceReport.csv";

    /// value returned on error
    static const int32_t     kErrorReturn           = -1;

    /// Parse a command line into key/value map
    CommandLineParams parse_command_line(int argc, char* argv[]);

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

        // write out performance report if we have flag set
        OpenNero::PerformanceMetricManager::instance().WriteReport( kDefaultPerformanceTxt, OpenNero::PerformanceMetricManager::kFormat_Report );
        OpenNero::PerformanceMetricManager::instance().WriteReport( kDefaultPerformanceCsv, OpenNero::PerformanceMetricManager::kFormat_CSV );

        // shut down the logger
        OpenNero::Log::LogSystemShutdown();

        return 0;
    }

    /**
     * Parse command line parameters
     * @param argc number of command line parameters
     * @param argv array of command line parameters including command name (argv[0])
     * @return map of parameter names and values
     */
    CommandLineParams parse_command_line(int argc, char* argv[])
    {
        CommandLineParams results;

        TCLAP::CmdLine cmd("OpenNERO (c) 2008 The University of Texas at Austin", ' ', "0.1");

        TCLAP::ValueArg<std::string> argSeeds("s","seeds","Random map seeds", false, "12345", "string");
        cmd.add(argSeeds);
        TCLAP::ValueArg<std::string> argConfig("c","config","Configuration file", false, "appConfig.py", "string");
        cmd.add(argConfig);

        // Wrap everything in a try block.  Do this every time,
        // because exceptions will be thrown for problems.
        try {
#if !NERO_PLATFORM_MAC
            // Parse the argv array, unless we are on a mac, in which case we are probably running from within a bunlde
            cmd.parse( argc, argv );
#endif
        }
        catch (TCLAP::ArgException &e)
        {
            LOG_F_ERROR("core", "Error: " << e.error() << " for arg " << e.argId());
        }

        // Get the value parsed by each arg.
        results["seeds"] = argSeeds.getValue();
        results["config"] = argConfig.getValue();

        return results;
    }

} // end OpenNero
