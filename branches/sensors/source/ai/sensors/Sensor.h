#ifndef _OPENNERO_AI_SENSORS_SENSOR_H_
#define _OPENNERO_AI_SENSORS_SENSOR_H_

#include <boost/serialization/export.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include "core/Common.h"
#include "core/IrrUtil.h"
#include "game/SimEntity.h"

namespace OpenNero
{
    
    BOOST_SHARED_DECL(Sensor);

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
        U32 types_;

        friend class boost::serialization::access;        
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(ticks);
            ar & BOOST_SERIALIZATION_NVP(types_);
        }
    public:
        Sensor() : ticks(1), types_(0) {}

        Sensor(U32 ticks, U32 types) : ticks(ticks), types_(types) {}

        //! Get the region of interest for this sensor
        virtual BBoxf getRegionOfInterest() = 0;
        
        //! Get the types of objects this sensor needs to look at
        virtual U32 getTypesOfInterest() { return types_; }

        //! get the minimal possible observation
        virtual double getMin() = 0;
        
        //! get the maximum possible observation
        virtual double getMax() = 0;

        //! Process an object of interest
        virtual bool process(SimEntityPtr ent) = 0;
        
        //! Get the value computed for this sensor
        virtual double getObservation() = 0;
    };

    std::ostream& operator<<(std::ostream& output, const Sensor& sensor);
}

BOOST_CLASS_EXPORT_KEY(Sensor)

#endif /* _OPENNERO_AI_SENSORS_SENSOR_H_ */
