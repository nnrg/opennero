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
    namespace core
    {
        std::ostream& operator<<( std::ostream& stream, const quaternion& q)
        {
            stream << q.X << " " << q.Y << " " << q.Z << " " << q.W;
            return stream;
        }
    
    }

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
