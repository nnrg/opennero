//--------------------------------------------------------
// OpenNero : BitVector
//  a vector of bit flags
//--------------------------------------------------------

#ifndef _OPENNERO_COMMON_BITVECTOR_H_
#define _OPENNERO_COMMON_BITVECTOR_H_

#include "Common.h"

namespace OpenNero
{
	/**
	 * A BitVector is a string of 1s and 0s. The user passes in the bit number
	 * that they want to set or clear and the vector will store the change.
     */
	class BitVector
	{
	public:

		BitVector();
		BitVector( const BitVector& v );
		BitVector( uint32_t bitCount );

		/// wipe the entire bit vector (set all to zero)
		void Clear();

		/// assignment
		BitVector& operator=( const BitVector& v );

		/// get the number of bits in this vector
		uint32_t GetNumBits() const;

		/// set a bit to on
		void SetBit( uint32_t bitNumber );

		/// clear a bit to off
		void ClearBit( uint32_t bitNumber );

        /// clear all of the bits
        void ClearAllBits();

        /// set all of the bits
        void SetAllBits();

        /// make sure we have _AT_LEAST_ this many bits
        void EnsureCapacity( uint32_t numBits );

		/// get the value of a given bit
		bool Get( uint32_t bitIndex ) const;

		// get the stream of bits
		// const std::vector<uint8_t> GetBitstream() const;

		/// get a modifiable copy of the bit stream
		std::vector<uint8_t>& GetBitstream();

	private:

		// set a given bit with a val (0/1)
		void SetVal( uint32_t bitIndex, bool val );

		/// resize the vector to a new size
		void Resize( uint32_t newSize );

	private:

		/// bit masks
		static const uint8_t	skMasks[8];

		/// the bit string that stores the values
		std::vector<uint8_t>    mBitString;
	};

	// Passed on 2-15-07
	// extern bool BitVector_Test();

} //end jbsCommon

#endif //end _OPENNERO_COMMON_BITVECTOR_H_
