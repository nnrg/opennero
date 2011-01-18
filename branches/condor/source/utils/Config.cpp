//--------------------------------------------------------
// OpenNero : Config
//  configuration handlers
//--------------------------------------------------------

#include "core/Common.h"
#include "core/File.h"
#include "utils/Config.h"
#include "game/Kernel.h"
#include "scripting/scripting.h"
#include "tclap/CmdLine.h"
#include <iostream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

namespace OpenNero
{
    using namespace std;
    using namespace boost::archive;

    AppConfig::AppConfig()
        : Title("OpenNero")
        , LogFile("nero_log.txt")
        , LogConfigFile("logConfig.py")
        , RenderType("OpenGL")
        , StartMod("hub")
        , StartModDir("hub/")
        , StartCommand()
        , Width(800)
        , Height(600)
        , BPP(16)
        , FullScreen(false)
        , StencilBuffer(false)
        , VSync(false)
        , RandomSeeds("12345")
    {
    }

    
    /// Construct from command line arguments
    bool AppConfig::ParseCommandLine(int argc, char** argv)
    {
        try {
            TCLAP::CmdLine cmd("OpenNERO (command line interface)", ' ', "1.1");
            
            // construct command line argument specs
            TCLAP::ValueArg<std::string> 
                argLogFile("", "log", "log file name to use", false, "nero_log.txt", "filename");
            TCLAP::ValueArg<std::string> 
                argTitle("", "title", "title for the window", false, "OpenNERO", "string");
            TCLAP::ValueArg<std::string> 
                argLogConfigFile("", "log_config", "log config file name to use", false, "logConfig.py", "string");
            TCLAP::SwitchArg 
                argHeadlessMode("", "headless", "run in headless mode without using the display");
            TCLAP::ValueArg<std::string> 
                argStartMod("", "mod", "OpenNERO mod to start", false, "hub", "string");
            TCLAP::ValueArg<std::string> 
                argStartModDir("", "modpath", "OpenNERO modpath to search for resources", false, "hub:common", "colon-separated path");
            TCLAP::ValueArg<std::string> 
                argStartCommand("", "command", "Python command to run on startup", false, "", "Python command");
            TCLAP::ValueArg<int> 
                argWidth("", "width", "Window width in pixels", false, 800, "integer");
            TCLAP::ValueArg<int> 
                argHeight("", "height", "Window height in pixels", false, 600, "integer");
            TCLAP::ValueArg<int> 
                argBPP("", "bpp", "Bitps per pixel to use", false, 16, "integer");
            TCLAP::SwitchArg 
                argFullScreen("", "fullscreen", "Use the full screen mode", false);
            TCLAP::SwitchArg 
                argStencilBuffer("", "stencil", "Use the stencil buffer", false);
            TCLAP::SwitchArg 
                argVSync("", "vsync", "Use VSync", false);
            TCLAP::ValueArg<std::string> 
                argRandomSeeds("", "random", "Random seeds to use", false, "12345", "numbers");
            
            // add them to CmdLine object
            cmd.add(argLogFile);
            cmd.add(argTitle);
            cmd.add(argLogConfigFile);
            cmd.add(argHeadlessMode);
            cmd.add(argStartMod);
            cmd.add(argStartModDir);
            cmd.add(argStartCommand);
            cmd.add(argWidth);
            cmd.add(argHeight);
            cmd.add(argBPP);
            cmd.add(argFullScreen);
            cmd.add(argStencilBuffer);
            cmd.add(argVSync);
            cmd.add(argRandomSeeds);
                
            // parse the command line
            cmd.parse( argc, argv );
        
            // set the values
            Title = argTitle.getValue();
            LogFile = argLogFile.getValue();
            LogConfigFile = argLogConfigFile.getValue();
            if (argHeadlessMode.getValue()) {
                RenderType = "null";
            }
            StartMod = argStartMod.getValue();
            StartModDir = argStartModDir.getValue();
            StartCommand = argStartCommand.getValue();
            Width = argWidth.getValue();
            Height = argHeight.getValue();
            BPP = argBPP.getValue();
            FullScreen = argFullScreen.getValue();
            StencilBuffer = argStencilBuffer.getValue();
            VSync = argVSync.getValue();
            RandomSeeds = argRandomSeeds.getValue();

            return true;
        } catch (TCLAP::ArgException& e) {
            std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
            return false;
        }
       
    }

    /// output AppConfig to stream
    ostream& operator<<(ostream& output, const AppConfig& config)
    {
        xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(config);
        return output;
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
