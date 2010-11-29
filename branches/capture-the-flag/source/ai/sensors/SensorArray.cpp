#include "ai/sensors/SensorArray.h"

namespace OpenNero
{
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
