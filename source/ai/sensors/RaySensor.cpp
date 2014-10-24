#include "ai/sensors/RaySensor.h"
#include "core/IrrSerialize.h"
#include "game/SimContext.h"

namespace OpenNero {
    RaySensor::RaySensor(double x, double y, double z, double radius, U32 types, bool vis) :
        Sensor(1, types), x(x), y(y), z(z), radius(radius), vis(vis)
    {
    }
    
    RaySensor::~RaySensor() {}

    //! Process an object of interest
    bool RaySensor::process(SimEntityPtr source, SimEntityPtr target)
    {
        return true;
    }
    
    //! Get the value computed for this sensor given the filtered objects
    double RaySensor::getObservation(SimEntityPtr source)
    {
        SimEntityData hitEntity;
        Vector3f hitPos;
        Vector3f sourcePos(source->GetPosition());
        Vector3f toTarget(x,y,z);
        toTarget = ConvertNeroToIrrlichtPosition(toTarget);
        Matrix4 rotation;
        rotation.setRotationDegrees(ConvertNeroToIrrlichtRotation(source->GetRotation()));
        rotation.rotateVect(toTarget);
        toTarget = ConvertIrrlichtToNeroPosition(toTarget);
        toTarget.setLength(radius);
        Vector3f targetPos = sourcePos + toTarget;
        if (Kernel::GetSimContext()->FindInRay(hitEntity, hitPos, sourcePos, targetPos, getTypes(), vis))
        {
            Vector3f toHit = hitPos - sourcePos;
            //return toHit.getLength()/radius;
            double retval = radius / (toHit.getLength() * 10);
            if (retval < 0)
                retval = 0;
            if (retval > 1.0)
                retval = 1.0;
            return retval;
        }
        else
        {
            return 0.0;
        }        
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
