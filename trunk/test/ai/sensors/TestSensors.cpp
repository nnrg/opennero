#include "ai/sensors/Sensor.h"
#include "ai/sensors/RadarSensor.h"
#include "ai/sensors/RaySensor.h"
#include "ai/sensors/SensorArray.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SensorTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( sensor_test )
{
    using namespace OpenNero;
    RadarSensorPtr sensor1(new RadarSensor(-30,30,-45,45,100));
    RadarSensorPtr sensor2(new RadarSensor(-30,30,-45,45,100,0xFFFFFFFF));
    RadarSensorPtr sensor3(new RadarSensor(30,-30,-45,45,100));
    RaySensorPtr sensor4(new RaySensor(Vector3f(1,0,0), 100));
    RaySensorPtr sensor5(new RaySensor(Vector3f(1,1,1), 100, 0xFFFFFFFF));
    BOOST_CHECK(sensor1);
    BOOST_CHECK(sensor2);
    BOOST_CHECK(sensor3);
    BOOST_CHECK(sensor4);
    BOOST_CHECK(sensor5);
    SimEntityPtr ent;
    SensorArrayPtr sensor_array(new SensorArray(ent));
    sensor_array->addSensor(sensor1);
    sensor_array->addSensor(sensor2);
    sensor_array->addSensor(sensor3);
    sensor_array->addSensor(sensor4);
    sensor_array->addSensor(sensor5);
    BOOST_CHECK(sensor_array);
    BOOST_CHECK_EQUAL(sensor_array->getNumSensors(), 5);
}
