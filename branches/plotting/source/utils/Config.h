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

#if NERO_BUILD_PHYSICS
    /// Physics engine configuration. See
    /// <a href="http://opende.sourceforge.net/wiki/index.php/Manual_(Concepts)">ODE Manual</a>
	/// for meanings of some of these constants.
    struct PhysicsConfig
    {
        F32 tracking_radius;                ///< maximum distance from the origin that the objects are tracked
        F32 gravity;                        ///< acceleration due to gravity, m/s in -k
        F32 CFM;                            ///< ODE Constraints Force Mixing
        F32 ERP;                            ///< ODE Error Reduction Parameter
        F32 damping;                        ///< Damping force constant (proportional to linear velocity)
        F32 damping_torque;                 ///< Damping torque constant (proportional to rotational velocity)
        F32 default_mass;                   ///< default object mass (kg)
        F32 step_size;                      ///< physics step size in (sec)
        F32 slip1;                          ///< collision slip parameter
        F32 slip2;                          ///< collision slip parameter
        F32 soft_erp;                       ///< collision ERP
        F32 soft_cfm;                       ///< collision CFM
        F32 mu;                             ///< collision mu
        F32 ground_offset;                  ///< level of the floor plane (Z)

        /// Constructor
        PhysicsConfig();

        /// serialize to stream, file or string
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(tracking_radius);
            ar & BOOST_SERIALIZATION_NVP(gravity);
            ar & BOOST_SERIALIZATION_NVP(CFM);
            ar & BOOST_SERIALIZATION_NVP(ERP);
            ar & BOOST_SERIALIZATION_NVP(damping);
            ar & BOOST_SERIALIZATION_NVP(damping_torque);
            ar & BOOST_SERIALIZATION_NVP(default_mass);
            ar & BOOST_SERIALIZATION_NVP(step_size);
            ar & BOOST_SERIALIZATION_NVP(slip1);
            ar & BOOST_SERIALIZATION_NVP(slip2);
            ar & BOOST_SERIALIZATION_NVP(soft_erp);
            ar & BOOST_SERIALIZATION_NVP(soft_cfm);
            ar & BOOST_SERIALIZATION_NVP(mu);
            ar & BOOST_SERIALIZATION_NVP(ground_offset);
        }
    };

    /// output PhysicsConfig to stream
    std::ostream& operator<<(std::ostream& output, const PhysicsConfig& config);
#endif // NERO_BUILD_PHYSICS

    /// The application config stores a variety of tunable
    /// values that are used at initial startup
    struct AppConfig
    {
        std::string window_title;        ///< The window title text
        std::string log_config_file;     ///< The log config file
        std::string render_type;         ///< The type of renderer "OpenGL" or "null"
        std::string start_mod_name;      ///< The starting mod name
        std::string start_mod_dir;       ///< The starting mod directory
        uint32_t    window_width;        ///< Window width
        uint32_t    window_height;       ///< window height
        uint8_t     window_BPP;          ///< window bits per pixel
        bool        full_screen;         ///< Should we use fullscreen?
        bool        use_stencil_buffer;  ///< Should we use a stencil buffer for shadows?
        bool        use_vsync;           ///< Should we use vsync?
        std::string seeds;               ///< Random seed buffer

#if NERO_BUILD_PHYSICS
        PhysicsConfig physics_config;         ///< physics engine configuration
#endif // NERO_BUILD_PHYSICS

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
            ar & BOOST_SERIALIZATION_NVP(window_width);
            ar & BOOST_SERIALIZATION_NVP(window_height);
            ar & BOOST_SERIALIZATION_NVP(window_BPP);
            ar & BOOST_SERIALIZATION_NVP(full_screen);
            ar & BOOST_SERIALIZATION_NVP(use_stencil_buffer);
            ar & BOOST_SERIALIZATION_NVP(use_vsync);
            ar & BOOST_SERIALIZATION_NVP(seeds);
#if NERO_BUILD_PHYSICS
            ar & BOOST_SERIALIZATION_NVP(physics_config);
#endif // NERO_BUILD_PHYSICS
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
