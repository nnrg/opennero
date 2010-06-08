//--------------------------------------------------------
// OpenNero : Config
//  configuration handlers
//--------------------------------------------------------

#include "core/Common.h"
#include "core/File.h"
#include "utils/Config.h"
#include "scripting/scriptIncludes.h"
#include "scripting/scripting.h"
#include "game/Kernel.h"
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
#if NERO_BUILD_PHYSICS
        , physics_config()
#endif // NERO_BUILD_PHYSICS
    {}

#if NERO_BUILD_PHYSICS
    /// output PhysicsConfig to stream
    ostream& operator<<(ostream& output, const PhysicsConfig& config)
    {
        xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(config);
        return output;
    }
#endif // NERO_BUILD_PHYSICS

    /// output AppConfig to stream
    ostream& operator<<(ostream& output, const AppConfig& config)
    {
        xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(config);
        return output;
    }

    /// Get the current application config
    const AppConfig& GetAppConfig()
    {
        return Kernel::const_instance().getAppConfig();
    }

    /// Exports AppConfig classes to Python
    PYTHON_BINDER( AppConfig )
    {
#if NERO_BUILD_PHYSICS
        class_<PhysicsConfig>("PhysicsConfig", "Configuration for the physics engine")
            .def_readwrite("tracking_radius", &PhysicsConfig::tracking_radius, "maximum distance from the origin that the objects are tracked")
            .def_readwrite("gravity", &PhysicsConfig::gravity, "acceleration due to gravity, m/s in -k" )
            .def_readwrite("CFM", &PhysicsConfig::CFM, "ODE Constraints Force Mixing")
            .def_readwrite("ERP", &PhysicsConfig::ERP, "ODE Error Reduction Parameter")
            .def_readwrite("damping", &PhysicsConfig::damping, "Damping force constant (proportional to linear velocity)")
            .def_readwrite("damping_torque", &PhysicsConfig::damping_torque, "Damping torque constant (proportional to rotational velocity)")
            .def_readwrite("default_mass", &PhysicsConfig::default_mass, "default object mass (kg)")
            .def_readwrite("step_size", &PhysicsConfig::step_size, "physics step size in (sec)")
            .def_readwrite("slip1", &PhysicsConfig::slip1, "collision slip parameter")
            .def_readwrite("slip2", &PhysicsConfig::slip2, "collision slip parameter")
            .def_readwrite("soft_erp", &PhysicsConfig::soft_erp, "collision ERP")
            .def_readwrite("soft_cfm", &PhysicsConfig::soft_cfm, "collision ERP")
            .def_readwrite("mu", &PhysicsConfig::mu, "collision CFM")
            .def_readwrite("ground_offset", &PhysicsConfig::ground_offset, "level of the ground plane (Z)")
            .def(self_ns::str(self_ns::self));
        ;
#endif // NERO_BUILD_PHYSICS

        class_<AppConfig>("AppConfig", "A configuration of the application.")
            .def_readwrite("window_title", &AppConfig::window_title)
            .def_readwrite("log_config_file", &AppConfig::log_config_file)
            .def_readwrite("renderer", &AppConfig::render_type)
            .def_readwrite("start_mod_name", &AppConfig::start_mod_name)
            .def_readwrite("start_mod_dir", &AppConfig::start_mod_dir)
            .def_readwrite("start_command", &AppConfig::start_command)
            .def_readwrite("window_width", &AppConfig::window_width)
            .def_readwrite("window_height", &AppConfig::window_height)
            .def_readwrite("window_bpp", &AppConfig::window_BPP)
            .def_readwrite("fullscreen", &AppConfig::full_screen)
            .def_readwrite("use_stencil_buffer", &AppConfig::use_stencil_buffer)
            .def_readwrite("use_vsync", &AppConfig::use_vsync)
            .def_readwrite("seeds", &AppConfig::seeds)
#if NERO_BUILD_PHYSICS
            .def_readwrite("physics", &AppConfig::physics_config)
#endif // NERO_BUILD_PHYSICS
            .def(self_ns::str(self_ns::self))
        ;

        def("get_app_config", &GetAppConfig, return_value_policy<copy_const_reference>());
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
            se.destroy();
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

            // destroy the scripting context again
            se.destroy();
        }

        else
        {
            LOG_MSG( "Cannot find the log config file, listening to all messages by default." );
        }
    }
}; // end OpenNero
