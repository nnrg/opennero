#include "core/Common.h"
#include "core/ONTypes.h"
#include "core/IrrSerialize.h"
#include <algorithm>
#include <sstream>
#include "boost/lexical_cast.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( test_opennero )

BOOST_AUTO_TEST_CASE( test_irr_serialize )
{
    using namespace OpenNero;
    using namespace irr::core;
    using namespace irr::video;

    {
        vector2di v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<vector2di>("20 25") ), vector2di(20, 25) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "20 25" );        
    }

    {
        vector3di v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<vector3di>("20 25 100") ), vector3di(20, 25, 100) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "20 25 100" );
    }

    {
        position2di v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<position2di>("20 25") ), position2di(20, 25) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "20 25" );
    }

    {
        dimension2di v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<dimension2di>("100 200") ), dimension2di(100, 200) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "100 200" );
    }

    {
        aabbox3di v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<aabbox3di>("1 2 3 4 5 6")), aabbox3di(1, 2, 3, 4, 5, 6) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "1 2 3 4 5 6" );
    }

    {
        SColor v;
        BOOST_CHECK_EQUAL( (v = boost::lexical_cast<SColor>("255 0 255 0")), SColor(0, 255, 0, 255) );
        BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>(v), "255 0 255 0" );
    }
}

BOOST_AUTO_TEST_SUITE_END()
