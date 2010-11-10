#include "core/Common.h"
#include "core/IrrSerialize.h"
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
    }
    
    RadarSensor::~RadarSensor() {}

    //! Decide if this sensor is interested in a particular object
    bool RadarSensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        Vector3f vecToTarget = source->GetPosition() - target->GetPosition();
        Vector3f angleToTarget = 
            ConvertIrrlichtToNeroRotation(
                ConvertNeroToIrrlichtPosition(vecToTarget).getHorizontalAngle());
        double yawToTarget = LockAngleToCircle(angleToTarget.Z);
        double pitchToTarget = angleToTarget.X;
        if (vecToTarget.getLength() <= radius &&
            ((leftbound >= yawToTarget && yawToTarget >= rightbound) || 
            (leftbound < rightbound && (leftbound >= yawToTarget || yawToTarget >= rightbound))) &&
			(bottombound <= pitchToTarget && pitchToTarget <= topbound) )
        {
            LOG_F_DEBUG("sensors", "accept radar target: " << yawToTarget << ", " << pitchToTarget);
        } else {
            LOG_F_DEBUG("sensors", "reject radar target: " << yawToTarget << ", " << pitchToTarget);
        }
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
    double RadarSensor::getObservation(SimEntityPtr source)
    {
        // TODO: implement
        return 0;
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
