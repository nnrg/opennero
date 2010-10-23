#ifndef _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_
#define _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_

#include <boost/serialization/export.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "ai/sensors/Sensor.h"
#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/objects/TemplatedObject.h"
#include "game/objects/SimEntityComponent.h"
#include "ai/AI.h"

namespace OpenNero {

    BOOST_SHARED_DECL(SensorArray);
    
    class SensorArray 
        : public SimEntityComponent
    {
        friend class boost::serialization::access;
        std::vector<SensorPtr> sensors;

        /// serialize to/from stream, file or string
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(sensors);
        }        
    public:
        SensorArray() : SimEntityComponent(SimEntityPtr()) {};
        SensorArray(SimEntityPtr parent) : SimEntityComponent(parent) {}
        virtual bool LoadFromTemplate(ObjectTemplatePtr t, const SimEntityData& data) { return true; }
        size_t getNumSensors() { return sensors.size(); }
        void addSensor(SensorPtr sensor) { sensors.push_back(sensor); }
        Observations getObservations();
    };

    std::ostream& operator<<(std::ostream& output, const SensorArray& sensor_array);
}

BOOST_CLASS_EXPORT_KEY(OpenNero::SensorArray);

#endif // _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_
