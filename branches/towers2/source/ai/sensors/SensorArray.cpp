#include "core/Common.h"
#include "game/Kernel.h"
#include "ai/sensors/SensorArray.h"
#include "game/SimContext.h"

namespace OpenNero
{
    size_t SensorArray::addSensor(SensorPtr sensor)
    {
        sensors.push_back(sensor);
        return sensors.size() - 1;
    }

    void SensorArray::getObservations(Observations& observations)
    {
        std::vector<SensorPtr>::iterator sensIter;
        size_t i = 0;
        for (sensIter = sensors.begin(); sensIter != sensors.end(); ++sensIter) 
        {
            AssertMsg(i < observations.size(), "There are more built-in sensors than observations in AgentInitInfo");
            SimEntitySet::const_iterator entIter;
            const SimEntitySet entSet = Kernel::instance().GetSimContext()->getSimulation()->GetEntities((*sensIter)->getTypes());
            for (entIter = entSet.begin(); entIter != entSet.end(); ++entIter) 
            {
                (*sensIter)->process(GetEntity(), (*entIter));
            }
            observations[i] = (*sensIter)->getObservation(GetEntity());
            i++;
        }
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
