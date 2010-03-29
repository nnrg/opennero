#include "core/Common.h"
#include "SensorArray.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    using namespace boost;
    
    /**
     * Export SensorArray-specific script components
     */
    PYTHON_BINDER( SensorArray )
    {
        // export the interface to python so that we can override its methods there
        python::class_<PySensorArray, noncopyable, PySensorArrayPtr>("SensorArray", "Abstract brain for an AI agent")
        .def("get_info", pure_virtual(&SensorArray::get_info), "Get sensor meta-information")
        .def("get_values", pure_virtual(&SensorArray::get_values), "Get sensor values");
    }
}
