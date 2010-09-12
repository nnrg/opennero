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
        std::string window_title;        ///< The window title text
        std::string log_config_file;     ///< The log config file
        std::string render_type;         ///< The type of renderer "OpenGL" or "null"
        std::string start_mod_name;      ///< The starting mod name
        std::string start_mod_dir;       ///< The starting mod directory
        std::string start_command;       ///< The Python command to run after loading the module
        uint32_t    window_width;        ///< Window width
        uint32_t    window_height;       ///< window height
        uint8_t     window_BPP;          ///< window bits per pixel
        bool        full_screen;         ///< Should we use fullscreen?
        bool        use_stencil_buffer;  ///< Should we use a stencil buffer for shadows?
        bool        use_vsync;           ///< Should we use vsync?
        std::string seeds;               ///< Random seed buffer

        /// Constructor
        AppConfig();

        /// serialize to stream, file or string
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(window_title);
            ar & BOOST_SERIALIZATION_NVP(log_config_file);
            ar & BOOST_SERIALIZATION_NVP(render_type);
            ar & BOOST_SERIALIZATION_NVP(start_mod_name);
            ar & BOOST_SERIALIZATION_NVP(start_mod_dir);
            ar & BOOST_SERIALIZATION_NVP(start_command);
            ar & BOOST_SERIALIZATION_NVP(window_width);
            ar & BOOST_SERIALIZATION_NVP(window_height);
            ar & BOOST_SERIALIZATION_NVP(window_BPP);
            ar & BOOST_SERIALIZATION_NVP(full_screen);
            ar & BOOST_SERIALIZATION_NVP(use_stencil_buffer);
            ar & BOOST_SERIALIZATION_NVP(use_vsync);
            ar & BOOST_SERIALIZATION_NVP(seeds);
        }
    };

    /// output AppConfig to stream
    std::ostream& operator<<(std::ostream& output, const AppConfig& config);

    /// Get the application config out of a python file
    AppConfig ReadAppConfig( int argc, char** argv, const std::string& appConfigFile );

    /// Get the current application config
    const AppConfig& GetAppConfig();

    /// specify the receivers that the logger should consider
    /// @param logFileName the python file to look for the log configuration
    /// @param argc the number of arguments
    /// @param argv array of pointers to where the argument strings are
    void LogSpecifyReceivers( int argc, char** argv, const std::string& logFileName );
};

#endif // _OPENNERO_UTIL_CONFIG_H_
