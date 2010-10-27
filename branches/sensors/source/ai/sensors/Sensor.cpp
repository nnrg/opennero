#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    std::ostream& operator<<(std::ostream& output, const Sensor& sensor)
    {
        boost::archive::xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(sensor);
        return output;
    }
    
    BBoxf PySensor::getRegionOfInterest()
    {
        BBoxf result;
        TryOverride("getRegionOfInterest", result);
        return result;
    }
    
    double PySensor::getMin()
    {
        double result;
        TryOverride("getMin", result);
        return result;
    }
    
    double PySensor::getMax()
    {
        double result;
        TryOverride("getMax", result);
        return result;
    }
    
    bool PySensor::process(SimEntityPtr ent)
    {
        bool result;
        TryOverride("process", result, ent);
        return result;
    }
    
    double PySensor::getObservation()
    {
        double result;
        TryOverride("getObservation", result);
        return result;
    }

}

BOOST_CLASS_EXPORT_IMPLEMENT(OpenNero::Sensor);

