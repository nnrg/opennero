#include "ai/sensors/RaySensor.h"
#include "core/IrrSerialize.h"

namespace OpenNero {
    RaySensor::RaySensor(double x, double y, double z, double radius, U32 types) :
        Sensor(1, types), x(x), y(y), z(z), radius(radius)
    {
        LOG_F_DEBUG("sensors", "new RaySensor(" << x << ", " << y << ", " << z << ", " << radius << ")");
    }
    
    RaySensor::~RaySensor() {}

    //! Get the region of interest for this sensor
    BBoxf RaySensor::getRegionOfInterest()
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

    std::ostream& operator<<(std::ostream& output, const RaySensor& ray_sensor)
    {
        boost::archive::xml_oarchive out_archive(output);
        out_archive << BOOST_SERIALIZATION_NVP(ray_sensor);
        return output;
    }
}

BOOST_CLASS_EXPORT_IMPLEMENT(OpenNero::RaySensor)
