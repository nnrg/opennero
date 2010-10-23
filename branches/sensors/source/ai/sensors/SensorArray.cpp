#include "ai/sensors/SensorArray.h"

namespace OpenNero
{
    std::ostream& operator<<(std::ostream& output, const SensorArray& sensor_array)
    {
        boost::archive::xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(sensor_array);
        return output;
    }

}

BOOST_CLASS_EXPORT_IMPLEMENT(SensorArray)
