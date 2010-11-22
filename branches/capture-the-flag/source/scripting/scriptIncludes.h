//--------------------------------------------------------
// OpenNero : scriptIncludes
//  minimal includes to use the script environment
//--------------------------------------------------------

#ifndef _OPENNERO_SCRIPTING_INCLUDE_H_
#define _OPENNERO_SCRIPTING_INCLUDE_H_

#include "core/Common.h"
#include "core/Preprocessor.h"

// this is a hack to temporarily disable certain warnings on MS compilers
#if NERO_PLATFORM_WINDOWS
    #pragma warning( push )
    #pragma warning( disable : 4267 4244 )
#endif

// this is a hack to work around a warning caused by a Python 2.4 config.h bug
#if defined(_POSIX_C_SOURCE)
#define NERO_POSIX_C_SOURCE _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#if defined(snprintf) && !defined(HAVE_SNPINTF)
#define HAVE_SNPRINTF
#endif
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

// this is a hack to work around a warning caused by a Python 2.4 config.h bug
#if !defined(_POSIX_C_SOURCE) && defined(NERO_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE NERO_POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

// this is a hack to temporarily disable certain warnings on MS compilers
#if NERO_PLATFORM_WINDOWS
    #pragma warning( pop )
#endif

// useful includes
#include <string>
#include <vector>

namespace OpenNero
{
    // useful namespaces
    using namespace std;
    using namespace boost;
    using namespace boost::python;

    /**
     * @brief extend this interface every time you need to export things to script
     * You would normally use the PYTHON_BINDER macro to export interfaces to Python,
     * which creates an instance of the exporter class and registers it with Boost.Python.
     * Warning: there is no initialization order guarantee between different exporters
     * that PYTHON_BINDER generates. If you need to export a base class before exporting
     * a class that extends it, put them into a single PYTHON_BINDER. Otherwise, you will
     * have a 50/50 chance of a runtime error.
     * @code
     *   /// @brief export the OpenNERO AI script interface
     *   PYTHON_BINDER( AI )
     *   {
     *       // export bounded array info
     *       class_<FeatureVectorInfo>("FeatureVectorInfo", "Describe constraints of a feature vector", init<>())
     *           .def(init<int>())
     *           .def("__len__", &FeatureVectorInfo::size, "Length of the feature vector")
     *           .def(self_ns::str(self_ns::self))
     *           .def("min", &FeatureVectorInfo::getMin, "Minimal value for an element")
     *           .def("max", &FeatureVectorInfo::getMax, "Maximal value for an element")
     *           .def("discrete", &FeatureVectorInfo::isDiscrete, "Is the element discrete or continuous?")
     *           .def("set_discrete", &FeatureVectorInfo::setDiscrete, "Create a discrete element")
     *           .def("set_continuous", &FeatureVectorInfo::setContinuous, "Create a continuous element")
     *           .def("add_discrete", &FeatureVectorInfo::addDiscrete, "Add a discrete element")
     *           .def("add_continuous", &FeatureVectorInfo::addContinuous, "Add a continuous element")
     *           .def("validate", &FeatureVectorInfo::validate, "Check whether a feature vector is valid")
     *           .def("get_instance", &FeatureVectorInfo::getInstance, "Create a feature vector based on this information");

     *       // export std::vector<double>
     *       class_< std::vector<double> > ("DoubleVector", "A vector of real values")
     *           .def(self_ns::str(self_ns::self))
     *           .def(python::vector_indexing_suite< std::vector<double> >());

     *       class_<AgentInitInfo>("AgentInitInfo", "Initialization information given to the agent",
     *           init<const FeatureVectorInfo&, const FeatureVectorInfo&, const FeatureVectorInfo&>())
     *           .def_readonly("sensors", &AgentInitInfo::sensors, "Constraints on the agent's sensor feature vector")
     *           .def_readonly("actions", &AgentInitInfo::actions, "Constraints on the agent's action feature vector")
     *           .def_readonly("reward", &AgentInitInfo::reward, "Constraints on the agent's reward")
     *           .def(self_ns::str(self_ns::self));
     *   }
     * @endcode
     * @see PYTHON_BINDER
     */
    class ScriptExporter
    {
        public:
            ScriptExporter();
            virtual ~ScriptExporter() {}
            /// this method will get executed to actually export the Python API
            virtual void PyBind() = 0;
    };

    /// called on script module initialization
    extern void InitializeScriptModule();

    /// Macro for exporting C++ methods and classes to Python
    /// @see ScriptExporter
#define PYTHON_BINDER( binderName )                     \
    class binderName##_Exporter : public ScriptExporter \
    {                                                   \
    public:                                             \
        void PyBind();                                  \
    } binderName##_instance;                            \
    void binderName##_Exporter::PyBind()

} // end OpenNero

#endif // _OPENNERO_SCRIPTING_INCLUDE_H_
