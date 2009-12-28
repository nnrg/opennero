//--------------------------------------------------------
// OpenNero : LookupTable
//  a table to map entries to
//--------------------------------------------------------

#include "core/Common.h"

#ifdef OPEN_NERO_TEST

#include "LookupTable.h"

namespace OpenNero 
{
    void LookupTable_UnitTest()
    {
        {
            LookupTable< int, float, char > lt;
            lt.add( 0, 4.5 );
            lt.add( 2, 9 );
            lt.add( 3, -1 );

            float t = 0;

            assert( lt.find(0) == 0 );
            assert( lt.find(2) == 1 );
            assert( lt.find(3) == 2 );
            assert( lt.getByKey( (int)0,t) );
            assert( t == 4.5 );
            assert( lt.getByKey( (int)2,t) );
            assert( t == 9 );
            assert( lt.getByKey( (int)3,t) );
            assert( t == -1 );
        }
    }       
    
} //end OpenNero

#endif
