#ifndef _OPENNERO_AI_SENSORS_SENSOR_H_
#define _OPENNERO_AI_SENSORS_SENSOR_H_

#include "core/Common.h"
#include "core/IrrUtil.h"
#include "game/SimEntity.h"
#include "scripting/scriptIncludes.h"
#include "scripting/scripting.h"
#include <iostream>

namespace OpenNero
{
    
    /// @cond
    BOOST_SHARED_DECL(Sensor);
    BOOST_SHARED_DECL(PySensor);
    /// @endcond

    /**
     * An abstract sensor class for predefined sensor types. The job of such 
     * a sensor is to return an observation given other objects. In order to
     * select the subset of the simulation that this sensor needs to compute
     * its observation, the objects are first intersected with the region of 
     * interest, then selected by type mask, and finally filtered using the 
     * filter function. The resulting subset is fed into the getObservation 
     * method.
     */
    class Sensor {
    private:
        //! every how-many ticks should this sensor be refreshed
        U32 ticks;

        //! the bitmask which is used to filter objects by type
        U32 types;

    protected:
        //! Write out XML parameters for this sensor
        virtual void toXMLParams(std::ostream& out) const;

    public:
        Sensor() : ticks(1), types(0) {}

        Sensor(U32 ticks, U32 types) : ticks(ticks), types(types) {}

        //! Get the tick count on which this sensor will process
        U32 getTicks() const { return ticks; }

        //! Get the types of objects this sensor needs to look at
        U32 getTypes() const { return types; }

        //! Get the region of interest for this sensor
        virtual BBoxf getBoundingBox() = 0;
        
        //! get the minimal possible observation
        virtual double getMin() = 0;
        
        //! get the maximum possible observation
        virtual double getMax() = 0;

        //! Process an object of interest
        virtual bool process(SimEntityPtr ent) = 0;
        
        //! Get the value computed for this sensor
        virtual double getObservation() = 0;

        friend std::ostream& operator<<(std::ostream&, const Sensor&);

    };

    class PySensor : public Sensor, public TryWrapper<Sensor>
    {
    public:
        PySensor() : Sensor() {}
        
        BBoxf getRegionOfInterest();
        
        double getMin();
        
        double getMax();
        
        bool process(SimEntityPtr ent);
        
        double getObservation();
    };
}

#endif /* _OPENNERO_AI_SENSORS_SENSOR_H_ */
