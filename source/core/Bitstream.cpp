//---------------------------------------------------
// Name: OpenNero : Bitstream
// Desc:  a stream of bits
//---------------------------------------------------

#include "core/Common.h"
#include "Bitstream.h"
#include "Error.h"

namespace OpenNero
{
    using namespace std;

	/// Default constructor
	Bitstream::Bitstream() : mFront(0) {}

    /// Take in a stream
    Bitstream::Bitstream( uint8_t* stream, uint32_t streamSize ) : 
        mStream(streamSize),
        mFront(0)
    {        
        mStream.insert( mStream.begin(), stream, stream + streamSize );
    }

	/// Copy Constructor
	Bitstream::Bitstream(const OpenNero::Bitstream &stream)
	{		
		mStream = stream.mStream;
		mFront  = stream.mFront;
	}
	
	/// Destructor
	Bitstream::~Bitstream()
	{}

	/// Remove all entries from the stream
	void Bitstream::Clear()
	{
		mFront = 0;
		mStream.clear();
	}

	/**
	 * Tells us if we have any bytes left in our stream
	 * @return false if we have anything inside our stream
    */
	bool Bitstream::IsEmpty() const
	{
		return mFront >= mStream.size();
	}

	/// Returns the size of the stream in bytes
	uint32_t Bitstream::ByteLength() const
	{
		return (uint32_t)mStream.size() - mFront;
	}

    /// Move the stream back to the beginning
    void Bitstream::Rewind( uint32_t offset )
    {
        mFront = offset;
        if( mFront > mStream.size() )
            mFront = (uint32_t)mStream.size();
    }
	
	/// Returns a ptr to the beginning of the stream
	uint8_t* Bitstream::Stream() const
	{
		return (uint8_t*)&mStream[mFront];
	}

	/// asignment operator
	Bitstream& Bitstream::operator=( const Bitstream& stream )
	{
		mStream = stream.mStream;
		mFront  = stream.mFront;
		return *this;
	}

    // equals operator
    bool Bitstream::operator==( const Bitstream& stream ) const
    {
        if( ByteLength() != stream.ByteLength() )
            return false;

        if( mFront != stream.mFront )
            return false;

        std::vector<uint8_t>::const_iterator my_itr     = mStream.begin();
        std::vector<uint8_t>::const_iterator my_end     = mStream.end();
        std::vector<uint8_t>::const_iterator stream_itr = stream.mStream.begin();

        for( ; my_itr != my_end; ++my_itr, ++stream_itr )
        {
            if( *my_itr != *stream_itr )
                return false;
        }

        return true;
    }

	/**
	 * Pop a byte off of the front of the stream
	 * @return the popped byte
    */
	uint8_t Bitstream::PopByte()
	{
		Assert( !IsEmpty() );
		return mStream[mFront++];
	}

	/**
	 * Push a single byte into our stream
	 * @param b the byte to push
	*/
	void Bitstream::PushByte( uint8_t b )
	{
		mStream.push_back(b);
		Assert( !IsEmpty() );
	}

    ostream& operator<<( ostream& os, const Bitstream& bitstream)
    {
        return os << "<Bitstream address\"" << &bitstream << "\" length=\"" << bitstream.ByteLength() << "\" />";
    }

	//-------------- INPUT FUNCTIONS ------------------

	/// move a uint8_t into the stream
	Bitstream& operator<<( Bitstream& stream, const uint8_t& val)
	{
		stream.PushByte(val);
		return stream;
	}

	/// move a uint16_t into the stream
	Bitstream& operator<<( Bitstream& stream, const uint16_t& val)
	{
		// push high byte, then low byte
		stream.PushByte( val << 8 );		
		stream.PushByte( val & 0xFF );	
		return stream;
	}

	/// move a uint32_t into the stream
	Bitstream& operator<<( Bitstream& stream, const uint32_t& val)
	{
		// push high byte to low byte
		stream.PushByte( ( val >> 24 ) & 0xFF );
		stream.PushByte( ( val >> 16 ) & 0xFF );
		stream.PushByte( ( val >>  8 ) & 0xFF );
		stream.PushByte( ( val >>  0 ) & 0xFF );
		return stream;
	}

	/// move a int8_t into the stream
	Bitstream& operator<<( Bitstream& stream, const int8_t& val)
	{
		return stream << (uint8_t)val;
	}

	/// move a int16_t into the stream
	Bitstream& operator<<( Bitstream& stream, const int16_t& val)
	{
		return stream << (uint16_t)val;
	}

	/// move a int32_t into the stream
	Bitstream& operator<<( Bitstream& stream, const int32_t& val)
	{
		return stream << (uint32_t)val;
	}

	/// move a float32_t into the stream
	Bitstream& operator<<( Bitstream& stream, const float32_t& val)
	{
		stream << *(uint32_t*)&val;		
		return stream;
	}

	/// move a float64_t into the stream
	Bitstream& operator<<( Bitstream& stream, const float64_t& val)
	{
		uint32_t* ptr = (uint32_t*)&val;
		stream << ptr[0];
		stream << ptr[1];

		return stream;
	}

	/// move a stream into the stream
    Bitstream& operator<<( Bitstream& stream, const Bitstream& rhs )
    {
        std::vector<uint8_t>::const_iterator itr     = rhs.mStream.begin();
        std::vector<uint8_t>::const_iterator end     = rhs.mStream.end();

        for( ; itr != end; ++itr )
            stream << *itr;
        
        return stream;
    }

    // TODO: STL strings don't have to terminate on 0 character. ours do.
    /// move a string to a stream
    Bitstream& operator<<( Bitstream& stream, const string& val)
    {
        for (string::const_iterator iter = val.begin(); iter != val.end(); ++iter)
        {
            stream.PushByte(*iter);
        }
        stream.PushByte('\0');
        return stream;
    }

	//-------------- OUTPUT FUNCTIONS ------------------

	/// move a uint8_t out of the stream
	Bitstream& operator>>(Bitstream& stream, uint8_t& val)
	{
		Assert( !stream.IsEmpty() );

		val = stream.PopByte();
		return stream;
	}

	/// move a uint16_t out of the stream
	Bitstream& operator>>(Bitstream& stream, uint16_t& val)
	{
		Assert( !stream.IsEmpty() );

		val = ( stream.PopByte() << 8 ) +
			  ( stream.PopByte() << 0 );
		return stream;
	}

	/// move a uint32_t out of the stream
	Bitstream& operator>>(Bitstream& stream, uint32_t& val)
	{
		Assert( !stream.IsEmpty() );

		val = ( stream.PopByte() << 24 ) +
			  ( stream.PopByte() << 16 ) +
			  ( stream.PopByte() <<  8 ) +
			  ( stream.PopByte() <<  0 );
		return stream;
	}

	/// move a int8_t out of the stream
	Bitstream& operator>>( Bitstream& stream, int8_t& val)
	{
		uint8_t uval;
		stream >> uval;
		val = (uint8_t)uval;
		return stream;
	}

	/// move a int16_t out of the stream
	Bitstream& operator>>( Bitstream& stream, int16_t& val)
	{
		uint16_t uval;
		stream >> uval;
		val = (uint16_t)uval;
		return stream;
	}

	/// move a int32_t out of the stream
	Bitstream& operator>>( Bitstream& stream, int32_t& val)
	{
		uint32_t uval;
		stream >> uval;
		val = (uint32_t)uval;
		return stream;		
	}

	/// move a float32_t out of the stream
	Bitstream& operator>>(Bitstream& stream, float32_t& val)
	{
		Assert( !stream.IsEmpty() );

		uint32_t uVal;
		stream >> uVal;
		val = *(float32_t*)&uVal;
		
		return stream;
	}

	/// move a float64_t out of the stream
	Bitstream& operator>>(Bitstream& stream, float64_t& val)
	{
		Assert( !stream.IsEmpty() );

		uint32_t uVals[2];
		stream >> uVals[0];
		stream >> uVals[1];

		val = *(float64_t*)uVals;
		return stream;
	}

	/// move a stream out of the stream
    Bitstream& operator>>(Bitstream& stream,  Bitstream& rhs )
    {
        std::vector<uint8_t>::const_iterator itr     = stream.mStream.begin();
        std::vector<uint8_t>::const_iterator end     = stream.mStream.end();

        for( ; itr != end; ++itr )
            rhs << *itr;
        
        return stream;
    }

    // TODO: STL strings don't have to terminate on 0 character. ours do.
    /// move a string out of a stream
    Bitstream& operator>>( Bitstream& stream, std::string& val)
    {
        val.clear();
        char c = static_cast<char>(stream.PopByte());
        while (c != '\0' && !stream.IsEmpty())
        {
            val += c;
            c = static_cast<char>(stream.PopByte());
        }
        return stream;
    }

} //end OpenNero
