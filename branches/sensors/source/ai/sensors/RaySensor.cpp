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
        return BBoxf(0,0,0,10,10,10);
    }
    
    //! Process an object of interest
    bool RaySensor::process(SimEntityPtr source, SimEntityPtr target)
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

    //! Output this sensor in a human-readable form
    void RaySensor::toStream(std::ostream& out) const
    {
        out << "<RaySensor ";
        toXMLParams(out);
        out << " />";
    }
}
