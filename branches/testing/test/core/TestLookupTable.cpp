#include "core/Common.h"

#include "core/LookupTable.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( test_opennero )

BOOST_AUTO_TEST_CASE( test_lookup_table )
{
    using namespace OpenNero;

    LookupTable< int, float, char > lt;
    lt.add( 0, 4.5 );
    lt.add( 2, 9 );
    lt.add( 3, -1 );

    float t = 0;

    BOOST_CHECK_EQUAL( lt.find(0), 0 );
    BOOST_CHECK_EQUAL( lt.find(2), 1 );
    BOOST_CHECK_EQUAL( lt.find(3), 2 );
    BOOST_CHECK( lt.getByKey( (int)0,t) );
    BOOST_CHECK_EQUAL( t, 4.5 );
    BOOST_CHECK( lt.getByKey( (int)2,t) );
    BOOST_CHECK_EQUAL( t, 9 );
    BOOST_CHECK( lt.getByKey( (int)3,t) );
    BOOST_CHECK_EQUAL( t, -1 );
}

BOOST_AUTO_TEST_SUITE_END()
