#include "ai/sensors/Sensor.h"
#include "ai/sensors/RadarSensor.h"
#include "ai/sensors/RaySensor.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SensorTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( sensor_test )
{
    using namespace OpenNero;
    RadarSensorPtr sensor1(new RadarSensor(-30,30,-45,45,100));
    RadarSensorPtr sensor2(new RadarSensor(-30,30,-45,45,100,0xFFFFFFFF));
    RadarSensorPtr sensor3(new RadarSensor(30,-30,-45,45,100));
    BOOST_CHECK(sensor1);
    BOOST_CHECK(sensor2);
    BOOST_CHECK(sensor3);
    
}
