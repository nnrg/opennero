#include "core/Common.h"
#include "game/Kernel.h"
#include "ai/sensors/SensorArray.h"
#include "game/SimContext.h"

namespace OpenNero
{
    size_t SensorArray::addSensor(SensorPtr sensor)
    {
        LOG_F_DEBUG("sensors", "AddSensor:" << sensor);
        sensors.push_back(sensor);
        return sensors.size() - 1;
    }

    Observations SensorArray::getObservations()
    {
        std::vector<SensorPtr>::iterator sensIter;
        Observations obs;
        LOG_F_DEBUG("sensors", "SensorArray::getObservations");
        for (sensIter = sensors.begin(); sensIter != sensors.end(); ++sensIter) {
            LOG_F_DEBUG("sensors", "SensorArray::getObservations sensor" << (*sensIter));
            SimEntitySet::const_iterator entIter;
            const SimEntitySet entSet = Kernel::instance().GetSimContext()->getSimulation()->GetEntities();
            for (entIter = entSet.begin(); entIter != entSet.end(); ++entIter) {
                (*sensIter)->process(GetEntity(), (*entIter));
            }
            obs.push_back((*sensIter)->getObservation());
        }
        return obs;
    }

    std::ostream& operator<<(std::ostream& out, const SensorArray& sa)
    {
        out << "<SensorArray>";
        std::vector<SensorPtr>::const_iterator iter;
        for (iter = sa.sensors.begin(); iter != sa.sensors.end(); ++iter) {
            out << "  " << **iter << std::endl;
        }
        out << "</SensorArray>";
        return out;
    }

}
