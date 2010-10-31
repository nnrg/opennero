#include "core/Common.h"
#include "ai/sensors/RadarSensor.h"

namespace OpenNero
{
    //! Create a new RadarSensor
    //! @param leftbound least relative yaw (degrees) of objects to include
    //! @param rightbound greatest relative yaw (degrees) of objects to include
    //! @param bottombound least relative pitch (degrees) of objects to include
    //! @param topbound greatest relative pitch (degrees) of objects to include
    //! @param radius the radius of the radar sector (how far it extends)
    //! @param types the bitmask which is used to filter objects by type
    RadarSensor::RadarSensor(
        double leftbound, double rightbound, 
        double bottombound, double topbound, 
        double radius, U32 types )
        : Sensor(1, types)
        , leftbound(leftbound)
        , rightbound(rightbound)
        , bottombound(bottombound)
        , topbound(topbound)
        , radius(radius)
    {
        LOG_F_DEBUG("sensors", "new RadarSensor(" 
                    << leftbound << ", " 
                    << rightbound << ", "
                    << bottombound << ", "
                    << topbound << ", "
                    << radius << ")");
    }
    
    RadarSensor::~RadarSensor() {}

    //! Get the region of interest for this sensor
    BBoxf RadarSensor::getBoundingBox()
    {
        // TODO: implement
        return BBoxf(0,0,0,10,10,10);
    }
    
    //! Decide if this sensor is interested in a particular object
    bool RadarSensor::process(SimEntityPtr ent)
    {
        // TODO: implement
        return true;
    }
    
    //! get the minimal possible observation
    double RadarSensor::getMin()
    {
        // TODO: implement
        return 0;
    }
    
    //! get the maximum possible observation
    double RadarSensor::getMax()
    {
        // TODO: implement
        return 1;
    }

    //! Get the value computed for this sensor given the filtered objects
    double RadarSensor::getObservation()
    {
        // TODO: implement
        return 0.5;
    }
    
    void RadarSensor::toXMLParams(std::ostream& out) const
    {
        Sensor::toXMLParams(out);
        out << "leftbound=\"" << leftbound << "\" "
            << "rightbound=\"" << rightbound << "\" "
            << "topbound=\"" << topbound << "\" "
            << "bottombound=\"" << bottombound << "\" "
            << "radius=\"" << radius << "\" ";
    }

    std::ostream& operator<<(std::ostream& out, const RadarSensor& rs)
    {
        out << "<RadarSensor ";
        rs.toXMLParams(out);
        out << " />";
        return out;
    }
}
