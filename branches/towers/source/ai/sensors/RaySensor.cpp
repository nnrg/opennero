#include "ai/sensors/RaySensor.h"
#include "core/IrrSerialize.h"

namespace OpenNero {
    RaySensor::RaySensor(double x, double y, double z, double radius, U32 types) :
        Sensor(1, types), x(x), y(y), z(z), radius(radius)
    {
    }
    
    RaySensor::~RaySensor() {}

    //! Get the region of interest for this sensor
    BBoxf RaySensor::getBoundingBox()
    {
        // TODO: implement
        return BBoxf(0,0,0,10,10,10);
    }
    
    //! Process an object of interest
    bool RaySensor::process(SimEntityPtr ent)
    {
        return true;
    }
    
    //! Get the value computed for this sensor given the filtered objects
    double RaySensor::getObservation()
    {
        // TODO: implement
        return 0.5;
    }

    void RaySensor::toXMLParams(std::ostream& out) const
    {
        Sensor::toXMLParams(out);
        out << "x=\"" << x << "\" "
            << "y=\"" << y << "\" "
            << "z=\"" << z << "\" "
            << "radius=\"" << radius << "\" ";
    }

    std::ostream& operator<<(std::ostream& out, const RaySensor& rs)
    {
        out << "<RaySensor ";
        rs.toXMLParams(out);
        out << " />";
        return out;
    }
}
