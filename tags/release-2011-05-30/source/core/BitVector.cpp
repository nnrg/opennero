//--------------------------------------------------------
// OpenNero : BitVector
//  a vector of bit flags
//--------------------------------------------------------

#include "core/Common.h"
#include "BitVector.h"

namespace OpenNero
{
	/// bit masks
	const uint8_t BitVector::skMasks[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	/// default constructor
	BitVector::BitVector() 
	{}

	/// copy constructor
	BitVector::BitVector( const BitVector& v )
	{
		mBitString = v.mBitString;
	}

	/// constructor with initial size
	BitVector::BitVector( uint32_t bitCount )
	{
#if NERO_DEBUG
		const uint32_t kByteCount = (bitCount+7)>>3;
		Assert( !bitCount || (kByteCount > 0) );
#endif

		Resize( (bitCount+7)>>3 );
	}

	/// set all the bits in this vector to zero
	void BitVector::Clear()
	{
		if( mBitString.size() > 0 )
		{
			memset( &mBitString[0], 0, sizeof(uint8_t) * mBitString.size() );
		}
	}
	
	/// assignment operator
	BitVector& BitVector::operator=( const BitVector& v )
	{
		mBitString = v.mBitString;
		return *this;
	}

	/**
	 * Get the number of bits stored in this vector. Note that the number
	 * will be a multiple of eight.
	 * @return the number of bits rounded up to an alignment of eight
    */
	uint32_t BitVector::GetNumBits() const
	{
		// multiply the number of bytes by 8
		return (uint32_t)(mBitString.size() << 3);
	}	

	/**
	 * Set a bit to on
	 * @param bitNumber the bit to set
    */
	void BitVector::SetBit( uint32_t bitNumber )
	{
		SetVal( bitNumber, true );
	}
	
	/**
	 * Set a bit to off
	 * @param bitNumber the bit to clear
    */
	void BitVector::ClearBit( uint32_t bitNumber )
	{
		SetVal( bitNumber, false );
	}

    /** Clear our all of the bits in our vector */
    void BitVector::ClearAllBits()
    {
        Clear();
    }

    /** Set ALL the bits in our vector (even the unused high alignment bits) */
    void BitVector::SetAllBits()
    {
        if( mBitString.size() > 0 )
	    {
		    memset( &mBitString[0], 0xFF, sizeof(uint8_t) * mBitString.size() );
	    }
    } 

    /**
     * Make sure that we have a least this many bits in our vector 
     * @param numBits the number of bits we want to ensure
    */
    void BitVector::EnsureCapacity( uint32_t numBits )
    {
        const uint32_t kBitWord = numBits >> 8;

        if( kBitWord >= mBitString.size() )		
			Resize( kBitWord+1 );
    }

	/**
	 * Get the value of a given bit
	 * @param bitIndex the bit to check
	 * @return the value of the bit
    */
	bool BitVector::Get( uint32_t bitIndex ) const
	{	
		const uint32_t kBitWord = bitIndex >> 3;
		const uint32_t kBitSlot = bitIndex & 0x7;

		if( kBitWord >= mBitString.size() )
			return false;

		Assert( kBitWord < mBitString.size() );
		Assert( kBitSlot <= 7 );

		return ( (mBitString[kBitWord] & skMasks[kBitSlot]) != 0 );		
	}

	/*
	 * Returns a stream of bytes that represents the bit vector
	 * @return byte stream
    */
  /*	const std::vector<uint8_t> BitVector::GetBitstream() const
	{
		return mBitString;
		}*/

	/**
	 * Return a modifiable reference to our bitstream
	 * Yes, this is a little more dangerous, use carefully.
	 * @return RW reference to our bitstream
    */
	std::vector<uint8_t>& BitVector::GetBitstream()
	{
		return mBitString;
	}

	/**
	 * Set a given bit with a value
	 * @param bitIndex the bit number to set
	 * @param val the value to set the bit to
    */
	void BitVector::SetVal( uint32_t bitIndex, bool val )
	{
		// get the slots
		const uint32_t kBitWord = bitIndex >> 3;
		const uint32_t kBitSlot = bitIndex & 0x7;

		Assert( kBitSlot == bitIndex % 8 );

		// if this bitIndex is bigger than us, resize to fit
		if( kBitWord >= mBitString.size() )		
			Resize( kBitWord+1 );

		Assert( kBitWord < mBitString.size() );				

		// assign the value
		mBitString[kBitWord] = val? ( mBitString[kBitWord] |=  skMasks[kBitSlot] ) :
									( mBitString[kBitWord] &= ~skMasks[kBitSlot] );

		AssertMsg( val == Get(bitIndex), "Did not properly set bit flag" );
	}

	/**
	 * Resize the bit vector to a new size
	 * @param newSize the size of the vector in bytes
    */
	void BitVector::Resize( uint32_t newSize )
	{	
		const size_t kOldSize = mBitString.size();
		mBitString.resize(newSize);

		if( kOldSize < mBitString.size() )
		{
			memset( &mBitString[kOldSize], 0, mBitString.size()-kOldSize );
		}
	}	

} //end OpenNero
