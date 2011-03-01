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
        double leftbound;
        
        //! greatest relative yaw (degrees) of objects to include
        double rightbound;
        
        //! least relative pitch (degrees) of objects to include
        double bottombound;
        
        //! greatest relative pitch (degrees) of objects to include
        double topbound;

        //! the radius of the radar sector (how far it extends)
        double radius;

        //! cumulative value for the observation
        double value;

        //! bookkeeping for resetting the value
        bool observed;

        //! whether or not the sensor is displayed on screen
        bool vis;
        
    public:
        RadarSensor()
            : Sensor()
            , leftbound(0), rightbound(0)
            , bottombound(0), topbound(0)
            , radius(0)
            , value(0)
            , observed(false)
            , vis(false)
        {}
    
        //! Create a new RadarSensor
        //! @param leftbound least relative yaw (degrees) of objects to include
        //! @param rightbound greatest relative yaw (degrees) of objects to include
        //! @param bottombound least relative pitch (degrees) of objects to include
        //! @param topbound greatest relative pitch (degrees) of objects to include
        //! @param radius the radius of the radar sector (how far it extends)
        //! @param types the type mask for the objects that the radar will count
        //! @param vis whether or not the sensor is displayed on screen
        RadarSensor(
            double leftbound, double rightbound, 
            double bottombound, double topbound, 
            double radius, 
            U32 types = 0,
            bool vis = false);
        
        ~RadarSensor();
    
        //! get the minimal possible observation
        double getMin();
        
        //! get the maximum possible observation
        double getMax();

        //! Process an object of interest
        bool process(SimEntityPtr source, SimEntityPtr target);
        
        //! Get the value computed for this sensor
        double getObservation(SimEntityPtr source);

        /// send a string representation of this sensor to the stream
        void toStream(std::ostream& out) const;
        
        /// Attenuation factor for the sensor values. Value is calculated as
        /// kDistanceScalar * radius / distanceToObject
        static const double kDistanceScalar;

    protected:

        /// print the XML parameters associated with this sensor to the stream.
        void toXMLParams(std::ostream& out) const;

    };
}

#endif /* _OPENNERO_AI_SENSOR_RADARSENSOR_H_ */
