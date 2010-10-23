#ifndef _OPENNERO_AI_SENSOR_RAYSENSOR_H_
#define _OPENNERO_AI_SENSOR_RAYSENSOR_H_

#include <boost/serialization/export.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include "core/Common.h"
#include "ai/sensors/Sensor.h"

namespace OpenNero
{
    BOOST_SHARED_DECL(RaySensor);

    class RaySensor : public Sensor {
    private:
        //! x-direction of the ray
        double x;

        //! y-direction of the ray
        double y;

        //! z-direction of the ray
        double z;
        
        //! the radius of the ray (how far it extends)
        double radius;
        
        friend class boost::serialization::access;        
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Sensor);
            ar & BOOST_SERIALIZATION_NVP(x);
            ar & BOOST_SERIALIZATION_NVP(y);
            ar & BOOST_SERIALIZATION_NVP(z);
            ar & BOOST_SERIALIZATION_NVP(radius);
        }
    public:
        RaySensor() : Sensor(1,0), x(0), y(0), z(0), radius(0) {}
    
        //! Create a new RaySensor
        //! @param x x-direction of the ray
        //! @param y y-direction of the ray
        //! @param z z-direction of the ray
        //! @param radius the radius of the ray (how far it extends)
        RaySensor(double x, double y, double z, double radius, U32 types = 0);
        
        virtual ~RaySensor();
        
        //! Get the region of interest for this sensor
        virtual BBoxf getRegionOfInterest();

        //! get the minimal possible observation
        virtual double getMin() { return 0.0; }
        
        //! get the maximum possible observation
        virtual double getMax() { return radius; }

        //! Process an object of interest
        virtual bool process(SimEntityPtr ent);
        
        //! Get the value computed for this sensor
        virtual double getObservation();
    };

    std::ostream& operator<<(std::ostream& output, const RaySensor& ray_sensor);
}

BOOST_CLASS_EXPORT_KEY(OpenNero::RaySensor);

#endif /* _OPENNERO_AI_SENSOR_RAYSENSOR_H_ */
