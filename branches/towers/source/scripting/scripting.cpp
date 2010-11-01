#include "core/Common.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <set>

#include "tinyxml.h"
#include "scripting/scripting.h"
#include "scripting/exports.h"
#include "core/File.h"
#include "game/Kernel.h"
#include "boost/pool/detail/singleton.hpp"
#include "game/Kernel.h"


// symbols for exporting the python module init function
#define C_EXPORT_API extern "C" __declspec(dllexport)

BOOST_PYTHON_MODULE( OpenNero )
{
    OpenNero::InitializeScriptModule();
}

namespace OpenNero
{
    using namespace std;
    
    namespace py = boost::python;

    void PrintDebuggingInfo(py::object globals, 
                            py::object locals)
    {
#if NERO_DEBUG
        try {
            py::exec("import sys\n"
                     "import os\n"
                     "print 'System path: ', ':'.join(sys.path)\n"
                     "print 'Current directory: ', os.getcwd()\n"
                     "print 'Current namespace: ', dir()\n",
                     globals, locals);
        } catch (py::error_already_set const&) {
            LOG_F_ERROR("scripting", "could not print post-error information");
        }
#endif // NERO_DEBUG
    }

    // the default module name
    const char* ScriptingEngine::kDefaultModuleName = "OpenNero";

    /// Import a Python module currently in path
    bool ScriptingEngine::ImportModule(const string& moduleName)
    {
        try {
            stringstream ss;
            ss << "import " << moduleName << endl;
            python::exec(ss.str().c_str(), _globals, _globals);
            PrintDebuggingInfo(_globals, _globals);
        }
        catch (error_already_set const&)
        {
            LOG_F_ERROR("scripting", "error importing module: " << moduleName);
            LogError();
            return false;
        }
        LOG_F_DEBUG("scripting", "imported module: " << moduleName);
        return true;
    }

    bool ScriptingEngine::ExecFile(const string &filename)
    {
        LOG_F_DEBUG("scripting", "executing script: " << filename);
        if (!FileExists(filename)) {
            LOG_F_WARNING("scripting", "Could not find Python file: " << filename);
            return false;
        }
        try {
            python::exec_file(filename.c_str(), _globals, _globals);
        }
        catch (error_already_set const&)
        {
            LOG_F_ERROR("scripting", "error executing script: " << filename);
            LogError();
            return false;
        }
        return true;
    }

    bool ScriptingEngine::Exec(const string &snippet,bool supressErrors)
    {
        try {
            python::exec(snippet.c_str(), _globals, _globals);
        }
        catch (error_already_set)
        {
            if( !supressErrors )
            {
                LogError();
            }
            else
            {
                PyErr_Clear();
            }

            return false;
        }
        return true;
    }

    ScriptingEngine::ScriptingEngine()
        : _main_module(), _globals(), _locals(), _initialized(false)
    {
    }

    ScriptingEngine::~ScriptingEngine()
    {
        destroy();
    }
    
    void ScriptingEngine::init(int argc, char** argv)
    {
        if (!_initialized)
        {
            // start the Python interpreter
            Assert(argc > 0);
            Py_SetProgramName(argv[0]);
            Py_InitializeEx(0); // no interrupt handlers
            PySys_SetArgv(argc, argv);
            Assert(Py_IsInitialized());

            // get the main module
            _main_module = python::import("__main__");

            // get the namespace of the main module
            _globals = _main_module.attr("__dict__");

            LOG_F_DEBUG("scripting", "loaded __main__ and _globals");

            // add the client and server directories of the current mod
            AddScriptDirectory( Kernel::findResource("") ); // for the mod we are loading
            AddScriptDirectory( "." ); // for other mods
            
            LOG_F_DEBUG("scripting", "initialized path");

            // Register the module with the interpreter
            if (PyImport_AppendInittab( const_cast<char*>(kDefaultModuleName), initOpenNero) == -1)
            {
                throw runtime_error("Failed to export OpenNero to Python");
            }

            // Register our own log writer as the error output
            Exec("from OpenNero import StdLogWriter, ErrLogWriter\n"
                 "import sys\n"
                 "sys.stderr = ErrLogWriter()\n"
                 "sys.stdout = StdLogWriter()\n");
            
            Exec("from plot_client import NetworkLogWriter\n"
                 "network_log_writer = NetworkLogWriter()\n");
                 
            Extract("network_log_writer", _network_log_writer);

            _initialized = true;

            LOG_F_MSG("scripting", "scripting engine initialized");
        }
    }

    void ScriptingEngine::destroy()
    {
        if( _initialized )
        {
            _initialized = false;
            _network_log_writer.attr("close")();
        }
    }

    ScriptingEngine& ScriptingEngine::instance()
    {
        return boost::details::pool::singleton_default<ScriptingEngine>::instance();
    }

    const ScriptingEngine& ScriptingEngine::const_instance()
    {
        return boost::details::pool::singleton_default<ScriptingEngine>::instance();
    }

    void ScriptingEngine::AddScriptDirectory( const string& dirPath )
    {
        stringstream ss;

        if (OpenNero::FileExists(dirPath))
        {
            ss << "import sys" << endl;
            ss << "sys.path.append(\"" << dirPath << "\")" << endl;
        }
        else
        {
            LOG_F_WARNING("scripting", "path '" << dirPath << "' does not exist");
        }

        Exec(ss.str());
    }

    Scheduler& ScriptingEngine::GetScheduler()
    {
        return _scheduler;
    }

    // initialize our default module
    void InitializeScriptModule() {
        scripting::ExportScripts();
        LOG_F_DEBUG("scripting", "OpenNero Python module initialized");
    }

    void ScriptingEngine::LogError()
    {
        PyErr_Print();
        PrintDebuggingInfo(_globals, _globals);
        AssertMsg(false, "Python error");
    }
    
    void ScriptingEngine::NetworkWrite(const std::string& message)
    {
        _network_log_writer.attr("write")(message);
    }
}
