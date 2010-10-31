/**
@file 

\brief OpenNERO unit testing

This is an example unit test file for OpenNERO. We use Boost.Test as
the unit test framework. See other source files under test/, the
testOpenNERO target, and <a href="http://www.boost.org/libs/test/doc/html">Boost.Test documentation</a>
for additional information. These tests should be run with the ctest command.
*/
#define BOOST_TEST_MODULE TestOpenNERO
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( test_opennero )

BOOST_AUTO_TEST_CASE( test_example  )
{
    // this is just an example
    BOOST_TEST_MESSAGE("Testing OpenNERO...");
    
    // report and continue
    BOOST_CHECK(2*2 == 4);
    
    // report and stop
    BOOST_REQUIRE(2*2 == 4);
    
    // custom report and continue
    if (2*2 != 4)
        BOOST_ERROR("Unlikely error");
    
    // custom report and stop
    if (2*2 != 4)
        BOOST_FAIL("Unlikely error");
    
    // throw also works
    if (2*2 != 4) throw "Unlikely error";
    
    // custom report and continue
    BOOST_CHECK_MESSAGE(2*2 == 4, "2*2 was actually:" << 2*2);
    
    // report and continue, preferred method
    BOOST_CHECK_EQUAL(2*2,4);
}

BOOST_AUTO_TEST_SUITE_END()