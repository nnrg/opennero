#ifndef _OPENNERO_AI_SENSOR_RAYSENSOR_H_
#define _OPENNERO_AI_SENSOR_RAYSENSOR_H_

#include "core/Common.h"
#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    class RaySensor : public Sensor {
    private:
        //! direction vector of the ray (in agent's system where the heading is (1,0,0))
        Vector3f direction_;
        
        //! the radius of the ray (how far it extends)
        double radius_;
        
        //! the bitmask which is used to filter objects to be intersected by type
        U32 types_;
        
        
    public:
        //! Create a new RaySensor
        //! @param direction direction vector of the ray (in agent's system where the heading is (1,0,0))
        //! @param radius the radius of the ray (how far it extends)
        //! @param types the bitmask which is used to filter objects to be intersected by type
        RaySensor(Vector3f direction, double radius, U32 types = 0);
        
        virtual ~RaySensor();
    
        //! Get the region of interest for this sensor
        virtual BBoxf getRegionOfInterest();
        
        //! Get the types of objects this sensor needs to look at
        virtual U32 getTypesOfInterest();

        //! Decide if this sensor is interested in a particular object
        virtual bool filter(SimEntityPtr ent);
        
        //! get the minimal possible observation
        virtual double getMin();
        
        //! get the maximum possible observation
        virtual double getMax();

        //! Get the value computed for this sensor given the filtered objects
        virtual double getObservation(SimEntitySet subset);
    };
}

#endif /* _OPENNERO_AI_SENSOR_RAYSENSOR_H_ */