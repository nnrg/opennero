#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    std::ostream& operator<<(std::ostream& output, const Sensor& sensor)
    {
        boost::archive::xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(sensor);
        return output;
    }

    BOOST_CLASS_EXPORT_IMPLEMENT(Sensor)
}

