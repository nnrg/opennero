#ifndef _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_
#define _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_

#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/objects/TemplatedObject.h"
#include "game/objects/SimEntityComponent.h"
#include "ai/AI.h"
#include "ai/sensors/Sensor.h"


namespace OpenNero {

    BOOST_SHARED_DECL(SensorArray);
    BOOST_SHARED_DECL(SensorArrayTemplate);
    
    class SensorArray : 
        public SimEntityComponent,
        public TemplatedObject
    {
        std::vector<SensorPtr> sensors;
    public:
        SensorArray(SimEntityPtr parent) : SimEntityComponent(parent) {}
        virtual bool LoadFromTemplate(ObjectTemplatePtr t, const SimEntityData& data) { return true; }
        size_t getNumSensors() { return sensors.size(); }
        void addSensor(SensorPtr sensor) { sensors.push_back(sensor); }
        Observations getObservations();
    };
    
    class SensorArrayTemplate :
        public ObjectTemplate
    {
    public:
        /// copy constructor
        SensorArrayTemplate(const SensorArrayTemplate& t) : ObjectTemplate(t) {}
        
        /// constructor
        SensorArrayTemplate(SimFactoryPtr f, const PropertyMap& props) : ObjectTemplate(f,props) {}
        
        /// destructor
        virtual ~SensorArrayTemplate() {}

        /// create a SensorArray from this template for the specified entity
        SensorArrayPtr CreateObject(SimEntityPtr ent);

        /// return the string representation of this type
        static const std::string TemplateType()
        {
            return "SensorArray";
        }
    };
}

#endif // _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_