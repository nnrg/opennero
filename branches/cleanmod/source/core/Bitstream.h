//---------------------------------------------------
// Name: OpenNero : Bitstream
// Desc:  a stream of bits
//---------------------------------------------------

#ifndef _CORE_BITSTREAM_H_
#define _CORE_BITSTREAM_H_

#include <vector>
#include "core/ONTypes.h"

namespace OpenNero
{
	// TODO: Implement endianess for talking cross platform
    /// A stream that can be accessed at bit level
	class Bitstream
	{

	public:

		Bitstream();
        Bitstream( uint8_t* stream, uint32_t streamSize );
		Bitstream( const Bitstream& stream );
		~Bitstream();

		// clear out the stream (remove all entries)
		void Clear();

		// do we have anything let in the stream?
		// Note: This can return true if we have either not
		// put anything in the stream or if we have already
		// moved everything out of it with >> calls
		bool IsEmpty() const;

		// returns the size of our stream in bytes
		uint32_t ByteLength() const;

        // rewind the stream back to the beginning
        void Rewind( uint32_t offset = 0 );

		// returns a ptr to the beginning of our stream
		uint8_t* Stream( ) const;

		// operators
		Bitstream& operator=( const Bitstream& stream );
    /// equivalence operator
    bool operator==( const Bitstream& stream ) const;

		// friends for moving streams to streams
		friend Bitstream& operator<<( Bitstream& stream,  const Bitstream& rhs );
		friend Bitstream& operator>>( Bitstream& stream,  Bitstream& rhs );

		// remove the byte at the front of the 'queue'
		inline uint8_t PopByte();

		// push a byte into the front of the stream
		inline void PushByte( uint8_t b );

	private:

		// we need access to the array stream, hence we did
		// not used std::queue, but simulated with vector

		std::vector<uint8_t>	mStream; ///< The container for the stream data
		uint32_t				mFront;	 ///< The index in the vector of the front entry

	}; // end Bitstream

	// convenience operators
	Bitstream& operator<<( Bitstream& stream, const uint8_t& val);
	Bitstream& operator<<( Bitstream& stream, const uint16_t& val);
	Bitstream& operator<<( Bitstream& stream, const uint32_t& val);
	Bitstream& operator<<( Bitstream& stream, const int8_t& val);
	Bitstream& operator<<( Bitstream& stream, const int16_t& val);
	Bitstream& operator<<( Bitstream& stream, const int32_t& val);
	Bitstream& operator<<( Bitstream& stream, const float32_t& val);
	Bitstream& operator<<( Bitstream& stream, const float64_t& val);
    Bitstream& operator<<( Bitstream& stream, const std::string& val);

	// convenience operators
	Bitstream& operator>>( Bitstream& stream, uint8_t& val);
	Bitstream& operator>>( Bitstream& stream, uint16_t& val);
	Bitstream& operator>>( Bitstream& stream, uint32_t& val);
	Bitstream& operator>>( Bitstream& stream, int8_t& val);
	Bitstream& operator>>( Bitstream& stream, int16_t& val);
	Bitstream& operator>>( Bitstream& stream, int32_t& val);
	Bitstream& operator>>( Bitstream& stream, float32_t& val);
	Bitstream& operator>>( Bitstream& stream, float64_t& val);
    Bitstream& operator>>( Bitstream& stream, std::string& val);

    /// represent a stream on standard output
    std::ostream& operator<< (std::ostream& os, const Bitstream& bitstream);

    /**
     * Input a vector to out stream, must have a << operator for T to this stream
     * @param stream the bitstream to move data into
     * @param buffer the vector to get the data from
     * @return the resultant stream
    */
    template< typename T >
    Bitstream& operator<<( Bitstream& stream, const std::vector<T>& buffer )
    {
        stream << (uint32_t)buffer.size();

        typename std::vector<T>::const_iterator itr = buffer.begin();
        typename std::vector<T>::const_iterator end = buffer.end();
        for( ; itr != end; ++itr )
            stream << *itr;

        return stream;
    }

    /**
     * Output the contents of the vector t a stream, must have a >> operator for T to this stream.
     * The first parameter it reads from the string is the size, so this only works on vectors
     * put into the stream with <<
     * @param stream the bitstream to move data out of
     * @param buffer the vector put the data in
     * @return the resultant stream
    */
    template <typename T>
	Bitstream& operator>>( Bitstream& stream, std::vector<T>& buffer )
    {
        uint32_t size;
		stream >> size;

		if( size )
		{
			buffer.resize(size);

			for( uint32_t i = 0; i < (uint32_t)buffer.size(); ++i )
				stream >> buffer[i];
		}

        return stream;
    }

    // utiltity functions for packing a stream of 1-6 parameters

    /// create a stream from 1 parameter
    template< typename A >
    Bitstream MakeStream( const A& p1 ) { Bitstream s; s << p1; return s; }

    /// create a stream from 2 parameters
    template< typename A, typename B >
    Bitstream MakeStream( const A& p1, const B& p2 ) { Bitstream s; s << p1 << p2; return s; }

    /// create a stream from 3 parameters
    template< typename A, typename B, typename C >
    Bitstream MakeStream( const A& p1, const B& p2, const C& p3 ) { Bitstream s; s << p1 << p2 << p3; return s; }

    /// create a stream from 4 parameters
    template< typename A, typename B, typename C, typename D >
    Bitstream MakeStream( const A& p1, const B& p2, const C& p3, const D& p4 ) { Bitstream s; s << p1 << p2 << p3 << p4; return s; }

    /// create a stream from 5 parameters
    template< typename A, typename B, typename C, typename D, typename E >
    Bitstream MakeStream( const A& p1, const B& p2, const C& p3, const D& p4, const E& p5 ) { Bitstream s; s << p1 << p2 << p3 << p4 << p5; return s; }

    /// create a stream from 6 parameters
    template< typename A, typename B, typename C, typename D, typename E, typename F >
    Bitstream MakeStream( const A& p1, const B& p2, const C& p3, const D& p4, const E& p5, const F& p6 ) { Bitstream s; s << p1 << p2 << p3 << p4 << p5 << p6; return s; }

} //end OpenNero

#endif //end _CORE_BITSTREAM_H_
