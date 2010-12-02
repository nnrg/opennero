//---------------------------------------------------
// Name: OpenNero : IrrSerialize
//  Serialization functions for Irr things
//---------------------------------------------------

#include "core/Common.h"
#include "core/ONTypes.h"
#include "IrrSerialize.h"
#include <algorithm>
#include <sstream>

namespace irr
{
    namespace video
    {
        std::istream& operator>>( std::istream& stream, SColor& obj )
        {
            stream.setf( std::ios_base::skipws );
            OpenNero::uint32_t col[4] = { 0, 0, 0, 0 };
            stream >> col[0] >> col[1] >> col[2];
            if( !stream.eof() )
                stream >> col[3];

            obj.set( col[3], col[0], col[1], col[2] );
            return stream;
        }

        std::ostream& operator<<( std::ostream& stream, const SColor& obj )
        {
            stream << obj.getRed() << " " << obj.getGreen() << " " << obj.getBlue() << " " << obj.getAlpha();    
            return stream;
        }
    }
}

#include "boost/lexical_cast.hpp"
#include "core/Error.h"

namespace OpenNERO
{
    namespace test
    {
        using namespace irr::core;
        using namespace irr::video;
        
        void IrrSerialize_UnitTest()
        {
            using namespace boost;    

            {
                vector2di v;
                Assert( (v = boost::lexical_cast<vector2di>("20 25") ) == vector2di(20, 25) );
                Assert( boost::lexical_cast<std::string>(v) == "20 25" );        
            }

            {
                vector3di v;
                Assert( (v = boost::lexical_cast<vector3di>("20 25 100") ) == vector3di(20, 25, 100) );
                Assert( boost::lexical_cast<std::string>(v) == "20 25 100" );
            }

            {
                position2di v;
                Assert( (v = boost::lexical_cast<position2di>("20 25") ) == position2di(20, 25) );
                Assert( boost::lexical_cast<std::string>(v) == "20 25" );
            }

            {
                dimension2di v;
                Assert( (v = boost::lexical_cast<dimension2di>("100 200") ) == dimension2di(100, 200) );
                Assert( boost::lexical_cast<std::string>(v) == "100 200" );
            }

            {
                aabbox3di v;
                Assert( (v = boost::lexical_cast<aabbox3di>("1 2 3 4 5 6")) == aabbox3di(1, 2, 3, 4, 5, 6) );
                Assert( boost::lexical_cast<std::string>(v) == "1 2 3 4 5 6" );
            }

            {
                SColor v;
                Assert( (v = boost::lexical_cast<SColor>("255 0 255 0")) == SColor(0, 255, 0, 255) );
                Assert( boost::lexical_cast<std::string>(v) == "255 0 255 0" );
            }    
        }
    }
}


