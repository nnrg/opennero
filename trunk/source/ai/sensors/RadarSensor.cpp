#include "core/Common.h"
#include "core/IrrSerialize.h"
#include "ai/sensors/RadarSensor.h"
#include <iostream>
#include <algorithm>

namespace OpenNero
{
    const double RadarSensor::kDistanceScalar = 1.0/10.0;

    //! Create a new RadarSensor
    //! @param radius the radius of the radar sector (how far it extends)
    //! @param types the bitmask which is used to filter objects by type
    RadarSensor::RadarSensor(
        double lb, double rb, 
        double bb, double tb, 
        double radius, U32 types )
        : Sensor(1, types)
        , leftbound(LockDegreesTo180(lb))
        , rightbound(LockDegreesTo180(rb))
        , bottombound(LockDegreesTo180(bb))
        , topbound(LockDegreesTo180(tb))
        , radius(radius)
    {
    }
    
    RadarSensor::~RadarSensor() {}

    //! Decide if this sensor is interested in a particular object
    bool RadarSensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        if (observed)
        {
            observed = false;
            value = 0;
        }
        Vector3f sourcePos = source->GetPosition();
        Vector3f targetPos = target->GetPosition();
        Vector3f vecToTarget = targetPos - sourcePos;
        double distToTarget = vecToTarget.getLength();
        Matrix4 rotation;
        rotation.setRotationDegrees(source->GetRotation());
        rotation.rotateVect(vecToTarget);
        rotation = rotation.buildRotateFromTo(Vector3f(1,0,0), vecToTarget);
        Vector3f angleToTarget = rotation.getRotationDegrees();
        double yawToTarget = LockDegreesTo180(angleToTarget.Z);
        double pitchToTarget = LockDegreesTo180(angleToTarget.Y);
        if (distToTarget <= radius &&                                       // within radius
            ((leftbound <= yawToTarget && yawToTarget <= rightbound) ||     // yaw within L-R angle bounds
             (rightbound < leftbound) && (leftbound <= yawToTarget || yawToTarget <= rightbound)) && // possibly wrapping around
			(bottombound <= pitchToTarget && pitchToTarget <= topbound) )   // pitch within B-T angle bounds
        {
            if (distToTarget > 0) {
                value += kDistanceScalar * radius / distToTarget;
            } else {
                value += 1;
            }
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
        observed = true;
        return std::max(0.0, std::min(value,1.0));
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
