#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    void Sensor::toXMLParams(std::ostream& out) const
    {
        out << "ticks=\"" << ticks << "\" "
            << "types=\"" << types << "\" ";
    }
    
    std::ostream& operator<<(std::ostream& out, const Sensor& sensor)
    {
        out << "<Sensor ";
        sensor.toXMLParams(out);
        out << "/>";
        return out;
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
