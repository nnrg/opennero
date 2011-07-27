//---------------------------------------------------
// Name: OpenNero : IrrSerialize
//  Serialization functions for Irr things
//---------------------------------------------------

#ifndef _CORE_IRRLICHT_SERIALIZE_H_
#define _CORE_IRRLICHT_SERIALIZE_H_

#include <tinyxml.h>    // keeps TinyXML from complaining about isspace()

#include <iostream>
#include <irrlicht.h>

namespace irr
{
    namespace core
    {

        /// input a 2D vector from a stream
        template< typename T >
        std::istream& operator>>( std::istream& stream, vector2d<T>& obj )
        {
            stream.setf( std::ios_base::skipws );
            stream >> obj.X >> obj.Y;
            return stream;
        }
        
        /// input a 3d vector from a stream
        template< typename T >
        std::istream& operator>>( std::istream& stream, vector3d<T>& obj )
        {
            stream.setf( std::ios_base::skipws );
            stream >> obj.X >> obj.Y >> obj.Z;
            return stream;
        }
        
        /// input a 2d dimension from a stream
        template< typename T >
        std::istream& operator>>( std::istream& stream, dimension2d<T>& obj )
        {
            stream.setf( std::ios_base::skipws );
            stream >> obj.Width >> obj.Height;
            return stream;
        }
        
        /// input a bounding box from a stream
        template< typename T >
        std::istream& operator>>( std::istream& stream, aabbox3d<T>& obj )
        {
            stream.setf( std::ios_base::skipws );
            stream >> obj.MinEdge.X >> obj.MinEdge.Y >> obj.MinEdge.Z
                   >> obj.MaxEdge.X >> obj.MaxEdge.Y >> obj.MaxEdge.Z;
            return stream;
        } 
                
        /// output a 2D vector to a stream
        template< typename T >
        std::ostream& operator<<( std::ostream& stream, const vector2d<T>& obj )
        {
            stream << obj.X << " " << obj.Y;
            return stream;
        }
        
        /// output a 3D vector to a stream
        template< typename T >
        std::ostream& operator<<( std::ostream& stream, const vector3d<T>& obj )
        {
            stream << obj.X << " " << obj.Y << " " << obj.Z;
            return stream;
        }
        
        /// output a dimension to a stream
        template< typename T >
        std::ostream& operator<<( std::ostream& stream, const dimension2d<T>& obj )
        {
            stream << obj.Width << " " << obj.Height;
            return stream;
        }
        
        /// output a bounding box to a stream
        template< typename T >
        std::ostream& operator<<( std::ostream& stream, const aabbox3d<T>& obj )
        {
            stream << obj.MinEdge.X << " " << obj.MinEdge.Y << " " << obj.MinEdge.Z << " " 
                   << obj.MaxEdge.X << " " << obj.MaxEdge.Y << " " << obj.MaxEdge.Z;
            return stream;
        }

        /// output a quaternion to a stream
        std::ostream& operator<<( std::ostream& stream, const quaternion& q);


    } // namespace core

    namespace video
    {
        /// input a color from a stream
        std::istream& operator>>( std::istream& stream, SColor& obj );

        /// output a color to a stream
        std::ostream& operator<<( std::ostream& stream, const SColor& obj );

    } // namespace video
} // namespace irr


#endif //end _CORE_IRRLICHT_UTIL_H_
