#include "core/Common.h"
#include "core/IrrSerialize.h"
#include "ai/sensors/RadarSensor.h"
#include "render/LineSet.h"
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
        double radius, U32 types,
        bool vis)
        : Sensor(1, types)
        , leftbound(LockDegreesTo180(lb))
        , rightbound(LockDegreesTo180(rb))
        , bottombound(LockDegreesTo180(bb))
        , topbound(LockDegreesTo180(tb))
        , radius(radius)
        , value(0)
        , observed(false)
        , vis(vis)
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
        // the position of the source of the sensor
        Vector3f sourcePos = source->GetPosition();
        
        // the position of the target
        Vector3f targetPos = target->GetPosition();
        
        // the vector from the 
        Vector3f vecToTarget = targetPos - sourcePos;
        double distToTarget = vecToTarget.getLength();
        double myHeading = source->GetRotation().Z;
        double tgtAngle = RAD_2_DEG * atan2(vecToTarget.Y, vecToTarget.X); // [-180, 180]
        double yawToTarget = LockDegreesTo180(tgtAngle - myHeading);
        double pitchToTarget = 0; // TODO: for now

        // within radius
        bool within_range = (distToTarget <= radius);
        
        // yaw within R-L angle bounds
        bool within_yaw = (leftbound >= yawToTarget && yawToTarget >= rightbound);
        
        // yaw is within L-R angle bounds
        bool within_reverse_yaw = 
            (leftbound < rightbound && 
            (leftbound >= yawToTarget || rightbound <= yawToTarget));
        
        // pitch is within B-T bounds
        bool within_pitch = (bottombound <= pitchToTarget && pitchToTarget <= topbound);
        
        if (within_range && (within_yaw || within_reverse_yaw) && within_pitch)
        {
            if (distToTarget > 0) {
                value += kDistanceScalar * radius / distToTarget;
            } else {
                value = 1;
            }
            if (vis) {
                Vector3f r = source->GetRotation();
                double h = DEG_2_RAD * myHeading;
                double y = DEG_2_RAD * yawToTarget;
                double lb = DEG_2_RAD * leftbound;
                double rb = DEG_2_RAD * rightbound;
                double c = lb + (rb - lb) / 2.0;
                Vector3f tp(sourcePos.X + distToTarget * cos(y + h), sourcePos.Y + distToTarget * sin(y + h), sourcePos.Z);
                Vector3f hp(sourcePos.X + distToTarget * cos(h), sourcePos.Y + distToTarget * sin(h), sourcePos.Z);
                Vector3f cp(sourcePos.X + distToTarget * cos(c + h), sourcePos.Y + distToTarget * sin(c + h), sourcePos.Z);
                Vector3f lp(sourcePos.X + distToTarget * cos(lb + h), sourcePos.Y + distToTarget * sin(lb + h), sourcePos.Z);
                Vector3f rp(sourcePos.X + distToTarget * cos(rb + h), sourcePos.Y + distToTarget * sin(rb + h), sourcePos.Z);
                LineSet::instance().AddSegment(sourcePos, tp, SColor(255,255,0,0));
                LineSet::instance().AddSegment(sourcePos, lp, SColor(255,255,255,255));
                LineSet::instance().AddSegment(sourcePos, rp, SColor(255,255,255,255));
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
