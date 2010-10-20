#ifndef _OPENNERO_AI_SENSOR_RADARSENSOR_H_
#define _OPENNERO_AI_SENSOR_RADARSENSOR_H_

#include "core/Common.h"
#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    BOOST_SHARED_DECL(RadarSensor);

    class RadarSensor : public Sensor {
    private:
        //! least relative yaw (degrees) of objects to include
        double leftbound_;
        
        //! greatest relative yaw (degrees) of objects to include
        double rightbound_;
        
        //! least relative pitch (degrees) of objects to include
        double bottombound_;
        
        //! greatest relative pitch (degrees) of objects to include
        double topbound_;

        //! the radius of the radar sector (how far it extends)
        double radius_;
        
        //! the bitmask which is used to filter objects by type
        U32 types_;
                
    public:
        //! Create a new RadarSensor
        //! @param leftbound least relative yaw (degrees) of objects to include
        //! @param rightbound greatest relative yaw (degrees) of objects to include
        //! @param bottombound least relative pitch (degrees) of objects to include
        //! @param topbound greatest relative pitch (degrees) of objects to include
        //! @param radius the radius of the radar sector (how far it extends)
        //! @param types the bitmask which is used to filter objects by type
        RadarSensor(
            double leftbound, double rightbound, 
            double bottombound, double topbound, 
            double radius, 
            U32 types = 0);
        
        virtual ~RadarSensor();
    
        //! Get the region of interest for this sensor
        virtual BBoxf getRegionOfInterest();
        
        //! Get the types of objects this sensor needs to look at
        virtual U32 getTypesOfInterest();

        //! get the minimal possible observation
        virtual double getMin();
        
        //! get the maximum possible observation
        virtual double getMax();

        //! Process an object of interest
        virtual bool process(SimEntityPtr ent);
        
        //! Get the value computed for this sensor
        virtual double getObservation();
    };
}

#endif /* _OPENNERO_AI_SENSOR_RADARSENSOR_H_ */
