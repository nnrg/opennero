//--------------------------------------------------------
// OpenNero : Config
//  configuration handlers
//--------------------------------------------------------

#ifndef _OPENNERO_UTIL_CONFIG_H_
#define _OPENNERO_UTIL_CONFIG_H_

#include "core/ONTypes.h"
#include <string>
#include <iostream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

namespace OpenNero
{

    /// The application config stores a variety of tunable
    /// values that are used at initial startup
    struct AppConfig
    {
        std::string Title;              ///< The window title text
        std::string LogFile;            ///< The log file
        std::string LogConfigFile;      ///< The log config file
        std::string RenderType;         ///< The type of renderer "OpenGL" or "null"
        std::string StartMod;           ///< The starting mod name
        std::string StartModMode;       ///< The starting mod mode
        std::string StartModDir;        ///< The starting mod directory
        std::string StartCommand;       ///< The Python command to run after loading the module
        uint32_t    Width;              ///< Window width
        uint32_t    Height;             ///< window height
        uint8_t     BPP;                ///< window bits per pixel
        bool        FullScreen;         ///< Should we use fullscreen?
        bool        StencilBuffer;      ///< Should we use a stencil buffer for shadows?
        bool        VSync;              ///< Should we use vsync?
        std::string RandomSeeds;        ///< Random seed buffer
        float32_t   FrameDelay;         ///< the delay between AI frames to use for animation (in seconds)

        /// Constructor
        AppConfig();

        /// Construct from command line arguments
        bool ParseCommandLine(int argc, char** argv);

        /// serialize to stream, file or string
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(Title);
            ar & BOOST_SERIALIZATION_NVP(LogFile);
            ar & BOOST_SERIALIZATION_NVP(LogConfigFile);
            ar & BOOST_SERIALIZATION_NVP(RenderType);
            ar & BOOST_SERIALIZATION_NVP(StartMod);
            ar & BOOST_SERIALIZATION_NVP(StartModDir);
            ar & BOOST_SERIALIZATION_NVP(StartCommand);
            ar & BOOST_SERIALIZATION_NVP(Width);
            ar & BOOST_SERIALIZATION_NVP(Height);
            ar & BOOST_SERIALIZATION_NVP(BPP);
            ar & BOOST_SERIALIZATION_NVP(FullScreen);
            ar & BOOST_SERIALIZATION_NVP(StencilBuffer);
            ar & BOOST_SERIALIZATION_NVP(VSync);
            ar & BOOST_SERIALIZATION_NVP(RandomSeeds);
            ar & BOOST_SERIALIZATION_NVP(FrameDelay);
        }
    };

    /// output AppConfig to stream
    std::ostream& operator<<(std::ostream& output, const AppConfig& config);

    /// Get the current application config
    const AppConfig& GetAppConfig();

    /// specify the receivers that the logger should consider
    /// @param logFileName the python file to look for the log configuration
    /// @param argc the number of arguments
    /// @param argv array of pointers to where the argument strings are
    void LogSpecifyReceivers( int argc, char** argv, const std::string& logFileName );
};

#endif // _OPENNERO_UTIL_CONFIG_H_
