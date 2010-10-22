#include "ai/sensors/RaySensor.h"
#include "core/IrrSerialize.h"

namespace OpenNero {
    //! Create a new RaySensor
    //! @param direction direction vector of the ray (in agent's system where the heading is (1,0,0))
    //! @param radius the radius of the ray (how far it extends)
    //! @param types the bitmask which is used to filter objects to be intersected by type
    RaySensor::RaySensor(Vector3f direction, double radius, U32 types) :
        direction_(direction), radius_(radius), types_(types)
    {
        LOG_F_DEBUG("sensors", "new RaySensor(" << direction_ << ", " << radius_ << ", " << types_ << ")");
    }
    
    RaySensor::~RaySensor() {}

    //! Get the region of interest for this sensor
    BBoxf RaySensor::getRegionOfInterest()
    {
        // TODO: implement
        return BBoxf(0,0,0,10,10,10);
    }
    
    //! Get the types of objects this sensor needs to look at
    U32 RaySensor::getTypesOfInterest()
    {
        return types_;
    }

    //! Process an object of interest
    bool RaySensor::process(SimEntityPtr ent)
    {
        return true;
    }
    
    //! get the minimal possible observation
    double RaySensor::getMin()
    {
        // TODO: implement
        return 0.0;
    }
    
    //! get the maximum possible observation
    double RaySensor::getMax()
    {
        // TODO: implement
        return 1.0;
    }

    //! Get the value computed for this sensor given the filtered objects
    double RaySensor::getObservation()
    {
        // TODO: implement
        return 0.5;
    }
}
