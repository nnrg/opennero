#ifndef _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_
#define _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_

#include "ai/sensors/Sensor.h"
#include "core/Common.h"
#include "game/SimEntity.h"
#include "game/objects/SimEntityComponent.h"
#include "ai/AI.h"

namespace OpenNero {

    BOOST_SHARED_DECL(SensorArray);
    
    class SensorArray 
        : public SimEntityComponent
    {
        std::vector<SensorPtr> sensors;
    public:
        explicit SensorArray(SimEntityPtr parent) : SimEntityComponent(parent) {}
        size_t getNumSensors() { return sensors.size(); }
        size_t addSensor(SensorPtr sensor);
        void getObservations(Observations& observations);
        friend std::ostream& operator<<(std::ostream& out, const SensorArray& sa);
    };

}

#endif // _OPENNERO_AI_SENSORS_SENSOR_ARRAY_H_
