#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    void Sensor::toXMLParams(std::ostream& out) const
    {
        out << "ticks=\"" << ticks << "\" "
            << "types=\"" << types << "\" ";
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
    
    bool PySensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        bool result;
        TryOverride("process", result, source, target);
        return result;
    }
    
    double PySensor::getObservation(SimEntityPtr source)
    {
        double result;
        TryOverride("getObservation", result, source);
        return result;
    }

}
