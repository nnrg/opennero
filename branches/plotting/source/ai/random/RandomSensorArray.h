#ifndef _OPENNERO_AI_RANDOM_RANDOMSENSORARRAY_H_
#define _OPENNERO_AI_RANDOM_RANDOMSENSORARRAY_H_

#include "ai/AI.h"
#include "ai/SensorArray.h"
#include "ai/AgentBrain.h"
#include "ai/AIObject.h"
#include "ai/Environment.h"

namespace OpenNero
{
    /// @cond
    BOOST_SHARED_DECL(RandomSensors);
    /// @endcond
    
    /// Random sensor array for testing
    class RandomSensors : public SensorArray
    {
    public:
        /// constructor to generate random sensors
        RandomSensors() : _info() {}
        /// constructor when given sensor infor to start with
        explicit RandomSensors(SensorInfo info) : _info(info) {}
        SensorInfo get_info();
        Sensors get_values();
        /// load sensors given a template
        bool LoadFromTemplate(ObjectTemplatePtr t, const SimEntityData& data);
    private:
        SensorInfo _info;
    };
}

#endif /*RANDOMSENSORARRAY_H_*/
