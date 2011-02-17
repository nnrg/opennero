#include "core/Common.h"
#include "game/SimEntityData.h"
#include "game/Kernel.h"
#include "game/SimContext.h"
#include "core/IrrSerialize.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( test_opennero )

BOOST_AUTO_TEST_CASE( test_simentity_data )
{
    using namespace OpenNero;
    using namespace std;
    SimEntityData data;
    
    // initially, all data is considered dirty
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), U32(-1) );
    
    data.ClearDirtyBits();
    
    // after clearing the dirty bits, data is considered clean
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), 0 );

    // check that setting same value does not cause to be dirty
    data.SetPosition( Vector3f(0,0,0) );
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), 0 );

    // check that setting dif value does make it dirty
    data.SetPosition( Vector3f(1,0,0) );
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), SimEntityData::kDB_Position );
    BOOST_CHECK_CLOSE( data.GetPosition().X, 1.0f, 0.1f );
    BOOST_CHECK_CLOSE( data.GetPosition().Y, 0.0f, 0.1f );
    BOOST_CHECK_CLOSE( data.GetPosition().Z, 0.0f, 0.1f );            

    // check bit clearance
    data.ClearDirtyBits();
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), 0 );

    // check flag setting
    data.SetPosition( Vector3f(2,0,0) );
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), SimEntityData::kDB_Position );
    data.ClearDirtyBits();

    data.SetVelocity( Vector3f(0,1,0) );
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), SimEntityData::kDB_Velocity );

    data.SetPosition( Vector3f(3,0,0) );
    BOOST_CHECK_EQUAL( data.GetDirtyBits(), SimEntityData::kDB_Position | SimEntityData::kDB_Velocity );
}

BOOST_AUTO_TEST_SUITE_END()
