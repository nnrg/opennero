#ifndef _OPENNERO_SCRIPTING_SCRIPTING_H_
#define _OPENNERO_SCRIPTING_SCRIPTING_H_

#include "core/Common.h"
#include "scripting/scriptIncludes.h"
#include "scripting/Scheduler.h"

namespace OpenNero
{
    using namespace boost;
    using namespace std;

    /// @cond
    BOOST_SHARED_DECL(ScriptingEngine);
    /// @endcond

    /**
     * This class defines functions for embedding (Python) script in our application.
     * It uses Boost.Python to expose C++ classes to Python, load python modules, and call Python scripts.
     * Useful resources:
     * <ul>
     * <li><a href="http://www.firaxis.com/games/game_detail.php?gameid=6">Civilization IV</a>, which uses a similar
     *     embedding technique in their SDK</li>
     * <li><a href="http://www.boost.org/libs/python/doc/tutorial/doc/html/python/embedding.html">Boost.Python
     * embedding tutorial</a></li>
     * <li><a href="http://www.codeproject.com/useritems/py_embed.asp">Embedding Python In Your C++ Application</a></li>
     * <li><a href="http://www.boost.org/libs/python/doc/v2/callbacks.html">Boost.Python: Calling Functions and Methods</a></li>
     * <li><a href="http://wiki.python.org/moin/boost.python/HowTo">boost.python HowTo Wiki</a></li>
     * </ul>
     * @author Igor Karpov (ikarpov@cs.utexas.edu)
     */
    class ScriptingEngine
    {
    private:
        python::object _main_module;        ///< main module
        python::dict _globals;              ///< global namespace
        python::object _network_log_writer; ///< network log writer object
        bool _initialized;                  ///< flag to mark if this scripting engine is initialized
        Scheduler _scheduler;               ///< the event scheduler for scripts

    public:

        static const char* kDefaultModuleName;  ///< default module to add methods to

    public:

        ScriptingEngine();
        ~ScriptingEngine();
        
        /**
         * This is called every simulation tick. If a ModTick function is defined,
         * it is called.
         */
        void Tick(float32_t dt);

        /**
         * Get the singleton for the scripting engine
         * @return a reference to the engine
         */
        static ScriptingEngine& instance();

        /**
         * Get a const singleton of the scripting engine
         * @return a const reference to the engine
         */
        static const ScriptingEngine& const_instance();

        /**
         * Initialize the scripting engine. Turn on python
         */
        void init(int argc, char** argv);
        
        /**
         * Re-initialize the scripting engine after calling destroy()
         */
        void reinitialize();

        /**
         * Shut down the engine. Disable python and free resources
        */
        void destroy();

        /**
         * Log the last Python error to the OpenNERO log
         */
        void LogError();
        
        /**
         * Write a string message to a socket over the network
         */
        void NetworkWrite(const std::string& message);

        /// Import a Python module currently in path
        bool ImportModule(const std::string& moduleName);

        /**
         * Call the script contained in fileName
         * @param fileName name of the script file to run
         * @return true iff successful
         */
        bool ExecFile(const std::string& fileName);
        
        /**
         * Execute the statements contained in the parameter string
         * @param statement snippet code to execute
         * @param supressErrors if true do not fail on errors
         * @return true iff successful
         */
        bool Exec(const std::string& statement, bool supressErrors = false );

        /**
         * Add a directory for the engine to search for script files in
         * @param dirPath the full directory path to search inside
        */
        void AddScriptDirectory( const std::string& dirPath );

        /// Get the script event scheduler
        Scheduler& GetScheduler();

        /// extract a typed value from a python object
        template <typename Result>
        bool Extract(const std::string& name, Result& result)
        {
            python::object o = _globals[name.c_str()];

            python::extract<Result> extraction(o);
            result = extraction();
            return true;
        }

        /**
         * Evaluate the expression and return its result, assuming it can be converted to type T
         * @param expression a Python expression
         * @param result the result of the expression
         * @return the result of evaluating the Python expression
         */
        template <typename Result>
        bool Eval( const std::string& expression, Result& result)
        {
            python::object o;
            try {
                o = python::eval(expression.c_str(), _globals, _globals);
            }
            catch (python::error_already_set const &)
            {
                LogError();
                return false;
            }
            python::extract<Result> extraction(o);
            result = extraction();
            return true;
        }

        /// Call a script method with no parameters and no return value
        void Call( const std::string& methodName )
        {
            try
            {
                python::object method = _globals[methodName];
                AssertMsg(method.ptr(), "Failed to find method '" << methodName << "'");
                method();
            }
            catch (python::error_already_set const &)
            {
                LogError();
            }
        }

        /// Call a script method with no parameters and a return value
        template <typename Result>
        Result Call( const std::string& methodName )
        {
            python::object res;
            try
            {
                python::object method = _globals[methodName];
                AssertMsg(method.ptr(), "Failed to find method '" << methodName << "'");
                res = method();
            }
            catch (python::error_already_set const &)
            {
                LogError();
                return Result();
            }
            python::extract<Result> extraction(res);
            return extraction();
        }

        /// Call a script method with 1 parameter and no return value
        template <typename Param0>
        void Call( const std::string& methodName, const Param0& p0 )
        {
            try
            {
                python::object method = _globals[methodName];
                AssertMsg(method.ptr(), "Failed to find method '" << methodName << "'");
                method(p0);
            }
            catch (python::error_already_set const &)
            {
                LogError();
            }
        }

        /// Call a script method with 1 parameter
        template <typename Result, typename Param0>
        Result Call( const std::string& methodName, const Param0& p0 )
        {
            python::object res;
            try
            {
                python::object method = _globals[methodName];
                AssertMsg(method.ptr(), "Failed to find method '" << methodName << "'");
                res = method(p0);
            }
            catch (python::error_already_set const &)
            {
                LogError();
                return Result();
            }
            python::extract<Result> extraction(res);
            return extraction();
        }

        /// Call a python method with 2 parameters
        template <typename Result, typename Param0, typename Param1>
        Result Call( const std::string& methodName, const Param0& p0, const Param1& p1 )
        {
            python::object res;
            try
            {
                python::object method = _globals[methodName];
                AssertMsg(method.ptr(), "Failed to find method '" << methodName << "'");
                res = method(p0,p1);
            }
            catch (python::error_already_set const &)
            {
                LogError();
                return Result();
            }
            python::extract<Result> extraction(res);
            return extraction();
        }
    };

    /**
     * Attempt the execution of a given executable object. The executable
     * object is expected to have a () operator defined which returns a
     * python::object instance that can be extracted into a Result
     * value. If the execution fails, this method will handle the
     * error_already_set exception.
     * @tparam Result the type of the result value
     * @tparam ExecType the type of the executable object
     * @param execObj the ExecType instance to be executed
     * @return the resulting extraction
    */
    template<typename ExecType, typename Result>
    Result TryCall( ExecType execObj )
    {
        python::object res;
        try
        {
            // execute the override
            #if BOOST_WORKAROUND(BOOST_MSVC, <= 1300) // Workaround for vc6/vc7
                res = call<Result>(execObj.ptr());
            #else
                res = execObj();
            #endif
        }
        catch (python::error_already_set const &)
        {
            ScriptingEngine::instance().LogError();
            return Result();
        }
        python::extract<Result> extraction(res);
        return extraction();
    }

    /**
     * Attempt the execution of a given executable object. The executable
     * object is expected to have a () operator defined If the execution fails,
     * this method will handle the error_already_set exeception.
     * @tparam ExecType the type of the executable object
     * @param execObj the ExecType instance to be executed
    */
    template<typename ExecType>
    void TryCall( ExecType execObj )
    {
        try
        {
            // execute the override
            #if BOOST_WORKAROUND(BOOST_MSVC, <= 1300) // Workaround for vc6/vc7
                call<Result>(execObj.ptr());
            #else
                execObj();
            #endif
        }
        catch (python::error_already_set const &)
        {
            ScriptingEngine::instance().LogError();
        }
    }

    /// Python wrapper for C++ classes with pure virtual member functions
    template<typename WrappedType>
    class TryWrapper : public python::wrapper<WrappedType>
    {
    protected:
        /// Find the Python override method and call it.
        inline
        void TryOverride(const char* name)
        {
            try {
                if (python::override f = this->get_override(name)) {
                    f();
                } else {
                    AssertMsg(false, "Unable to find Python override for method: " << name);
                }
            } catch (python::error_already_set const &) {
                ScriptingEngine::instance().LogError();
            }
        }

        /// Find the Python override method and call it.
        template<typename Result>
        inline
        void TryOverride(const char* name, Result& result)
        {
            try {
                if (python::override f = this->get_override(name)) {
                    python::object res = f();
                    python::extract<Result> extraction(res);
                    result = extraction();
                } else {
                    LOG_F_WARNING("scripting", "Unable to find Python override for method: " << name);
                }
            } catch (python::error_already_set const &) {
                ScriptingEngine::instance().LogError();
            }
        }

        /// Find the Python override method and call it.
        template<typename Result, typename Param0>
        inline
        void TryOverride(const char* name, Result& result,
                         Param0& param0)
        {
            try {
                if (python::override f = this->get_override(name)) {
                    python::object res = f(param0);
                    python::extract<Result> extraction(res);
                    result = extraction();
                } else {
                    AssertMsg(false, "Unable to find Python override for method: " << name);
                }
            } catch (python::error_already_set const &) {
                ScriptingEngine::instance().LogError();
            }
        }

        /// Find the Python override method and call it.
        template<typename Result, typename Param0, typename Param1>
        inline
        void TryOverride(const char* name, Result& result,
                         Param0& param0, Param1& param1)
        {
            try {
                if (python::override f = this->get_override(name)) {
                    python::object res = f(param0, param1);
                    python::extract<Result> extraction(res);
                    result = extraction();
                } else {
                    AssertMsg(false, "Unable to find Python override for method: " << name);
                }
            } catch (python::error_already_set const &) {
                ScriptingEngine::instance().LogError();
            }
        }

        /// Find the Python override method and call it.
        template<typename Result, typename Param0, typename Param1, typename Param2>
        inline
        void TryOverride(const char* name, Result& result,
                           Param0& param0, Param1& param1, Param2& param2)
        {
            try {
                if (python::override f = this->get_override(name)) {
                    python::object res = f(param0, param1, param2);
                    python::extract<Result> extraction(res);
                    result = extraction();
                } else {
                    AssertMsg(false, "Unable to find Python override for method: " << name);
                }
            } catch (python::error_already_set const &) {
                ScriptingEngine::instance().LogError();
            }
        }
    };

}

#endif // _OPENNERO_SCRIPTING_SCRIPTING_H_
