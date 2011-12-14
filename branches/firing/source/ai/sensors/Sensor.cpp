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
        double result = 0;
        TryOverride("getMin", result);
        return result;
    }
    
    double PySensor::getMax()
    {
        double result = 1;
        TryOverride("getMax", result);
        return result;
    }
    
    bool PySensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        bool result = false;
        TryOverride("process", result, source, target);
        return result;
    }
    
    double PySensor::getObservation(SimEntityPtr source)
    {
        double result = 0;
        TryOverride("getObservation", result, source);
        return result;
    }

}
