#include "ai/sensors/Sensor.h"
#include "ai/sensors/RadarSensor.h"
#include "ai/sensors/RaySensor.h"
#include "ai/sensors/SensorArray.h"
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE( test_opennero )

BOOST_AUTO_TEST_CASE( test_sensors )
{
    using namespace OpenNero;
    RadarSensorPtr sensor1(new RadarSensor(-30,30,-45,45,100));
    RadarSensorPtr sensor2(new RadarSensor(-30,30,-45,45,100,0xFFFFFFFF));
    RadarSensorPtr sensor3(new RadarSensor(30,-30,-45,45,100));
    RaySensorPtr sensor4(new RaySensor(1,0,0, 100));
    RaySensorPtr sensor5(new RaySensor(1,1,1, 100, 0xFFFFFFFF));
    BOOST_CHECK(sensor1);
    BOOST_CHECK(sensor2);
    BOOST_CHECK(sensor3);
    BOOST_CHECK(sensor4);
    BOOST_CHECK(sensor5);
    std::stringstream ss(std::stringstream::in | std::stringstream::out);    
    ss << (*sensor1);
    ss << (*sensor2);
    ss << (*sensor3);
    ss << (*sensor4);
    ss << (*sensor5);
    SimEntityPtr ent;
    SensorArrayPtr sensor_array(new SensorArray(ent));
    sensor_array->addSensor(sensor1);
    sensor_array->addSensor(sensor2);
    sensor_array->addSensor(sensor3);
    sensor_array->addSensor(sensor4);
    sensor_array->addSensor(sensor5);
    BOOST_CHECK(sensor_array);
    BOOST_CHECK_EQUAL(sensor_array->getNumSensors(), 5);
    ss << (*sensor_array) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()