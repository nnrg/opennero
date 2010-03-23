#include "core/Common.h"
#include "RandomAI.h"
#include "RandomSensorArray.h"
#include "math/Random.h"

namespace OpenNero
{
    SensorInfo RandomSensors::get_info()
    {
        return _info;
    }
    Sensors RandomSensors::get_values()
    {
        Sensors ans(3);
        ans.push_back(RANDOM.randF());
        ans.push_back(RANDOM.randF());
        ans.push_back(RANDOM.randF());
        return ans;
    }

    bool RandomSensors::LoadFromTemplate(ObjectTemplatePtr objTempl, const SimEntityData& data)
    {
        _info = shared_static_cast<RandomAITemplate, ObjectTemplate>(objTempl)->getSensorInfo();
        return true;
    }
}
