#ifndef _OPENNERO_AI_SENSORARRAY_H_
#define _OPENNERO_AI_SENSORARRAY_H_

#include "core/Common.h"
#include "AI.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    /// @cond
    BOOST_SHARED_DECL(SensorArray);
    BOOST_SHARED_DECL(PySensorArray);
    /// @endcond

    /// A SensorArray is a collection of individual sensors that allows agents to sense the environment
    class SensorArray
    {
        public:
            virtual ~SensorArray() = 0;
            virtual SensorInfo get_info() = 0;  ///< return info about the sensors
            virtual Sensors get_values() = 0;   ///< return values of the sensor
    };
    
    inline SensorArray::~SensorArray() {} // defined for speed
    
    /// A concrete wrapper around the abstract SensorArray class to allow overriding of the methods in Python
    class PySensorArray: public SensorArray, public boost::python::wrapper<SensorArray>
    {
        SensorInfo get_info() {
            return this->get_override("get_info")();
        }
        
        Sensors get_values() {
            return this->get_override("get_values")();
        }
    };
    
}

#endif /*SENSORARRAY_H_*/
