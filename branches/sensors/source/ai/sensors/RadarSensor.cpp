#include "core/Common.h"
#include "core/IrrSerialize.h"
#include "ai/sensors/RadarSensor.h"

namespace OpenNero
{
    const double kDistanceScalar = 1.0/10.0;

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
    }
    
    RadarSensor::~RadarSensor() {}

    //! Decide if this sensor is interested in a particular object
    bool RadarSensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        Vector3f vecToTarget = source->GetPosition() - target->GetPosition();
        Vector3f angleToTarget = 
            ConvertIrrlichtToNeroRotation(
                ConvertNeroToIrrlichtPosition(vecToTarget).getHorizontalAngle());
        double yawToTarget = LockDegreesTo180(angleToTarget.Z);
        double pitchToTarget = LockDegreesTo180(angleToTarget.X);
        double distToTarget = vecToTarget.getLength();
        if (distToTarget <= radius &&                                       // within radius
            (leftbound <= yawToTarget && yawToTarget <= rightbound) &&      // yaw within L-R angle bounds
			(bottombound <= pitchToTarget && pitchToTarget <= topbound) )   // pitch within B-T angle bounds
        {
            distances.push_back(vecToTarget.getLength());
            LOG_F_DEBUG("sensors", "accept radar target: " << distToTarget << ", " << yawToTarget << ", " << pitchToTarget);
        } else {
            LOG_F_DEBUG("sensors", "reject radar target: " << distToTarget << ", " << yawToTarget << ", " << pitchToTarget);
        }
        return true;
    }
    
    //! get the minimal possible observation
    double RadarSensor::getMin()
    {
        return 0;
    }
    
    //! get the maximum possible observation
    double RadarSensor::getMax()
    {
        return 1;
    }

    //! Get the value computed for this sensor given the filtered objects
    double RadarSensor::getObservation(SimEntityPtr source)
    {
        double value = 0;
        // Iterate over all objects within the sensor's bounds, and for each, increase the value of the sensor by
        // radius_of_sensor / (distance_to_the_object * 10)
        // (i.e., the closer the object is to the sensor's origin, the more it adds to the value)
        for (size_t i = 0; i < distances.size(); ++i)
        {
            double distance = distances[i];
            if (distance != 0) {
                value += kDistanceScalar * (radius - distance) / radius;
            }
        }
        if (value > 1) {
            value = 1.0;
        }
        return value;
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

    void RadarSensor::toStream(std::ostream& out) const
    {
        out << "<RadarSensor ";
        toXMLParams(out);
        out << " />";
    }
}
