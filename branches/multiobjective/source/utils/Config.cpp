//--------------------------------------------------------
// OpenNero : Config
//  configuration handlers
//--------------------------------------------------------

#include "core/Common.h"
#include "core/File.h"
#include "utils/Config.h"
#include "game/Kernel.h"
#include "scripting/scripting.h"
#include <iostream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

namespace OpenNero
{
    using namespace std;
    using namespace boost::archive;

    // magic defaults
    AppConfig::AppConfig()
        : window_title("OpenNero")
        , log_config_file("logConfig.py")
        , render_type("OpenGL")
        , start_mod_name("hub")
        , start_mod_dir("hub/")
        , start_command()
        , window_width(800)
        , window_height(600)
        , window_BPP(16)
        , full_screen(false)
        , use_stencil_buffer(false)
        , use_vsync(false)
        , seeds("12345")
    {}

    /// output AppConfig to stream
    ostream& operator<<(ostream& output, const AppConfig& config)
    {
        xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(config);
        return output;
    }

    AppConfig ReadAppConfig( int argc, char** argv, const string& appConfigFile )
    {
        AppConfig appConfig;

        if( OpenNero::FileExists(appConfigFile) )
        {
            // Note: This should be run before the scripting engine is setup for the game
            ScriptingEngine& se = ScriptingEngine::instance();
            se.init(argc, argv);
            if( se.ExecFile(appConfigFile) )
            {
                // get the app config defined in the file
                se.Extract<AppConfig>("app_config", appConfig);
            }
        }

        return appConfig;
    }

    /// specify the receivers that the logger should consider
    /// @param argc number of arguments passed in in argv
    /// @param argv array of char* arguments
    /// @param logFileName the python file to look for the log configuration
    void LogSpecifyReceivers( int argc, char** argv, const string& logFileName )
    {
        // check for existence of the file
        if( OpenNero::FileExists(logFileName) )
        {
            // Note: This should be run before the scripting engine is setup for the game
            ScriptingEngine& se = ScriptingEngine::instance();
            se.init(argc, argv);
            if( se.ExecFile(logFileName) )
            {
                // get the list of receivers and the size of the list
            	boost::python::list pyReceiverList;
                se.Extract<boost::python::list>("ignore_types", pyReceiverList);
                if( !pyReceiverList.ptr() )
                {
                    LOG_ERROR( "Failed to get the log types variable from the log config file." );
                    return;
                }

                const OpenNero::uint32_t listSize = boost::python::extract<OpenNero::uint32_t>(pyReceiverList.attr("__len__")());

                // fill the receiver list
                OpenNero::Log::FilterList filterList;

                for( OpenNero::uint32_t i = 0; i < listSize; ++i )
                {
                    string str = boost::python::extract<string>(pyReceiverList[i]);
                    filterList.push_back(str);
                    LOG_MSG( "Ignoring messages of type: " << str );
                }

                // set the receiver list in the log
                OpenNero::Log::LogSystemSpecifyFilters( filterList );
            }
        }

        else
        {
            LOG_MSG( "Cannot find the log config file, listening to all messages by default." );
        }
    }
}; // end OpenNero
