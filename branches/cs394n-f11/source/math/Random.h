//--------------------------------------------------------
// OpenNero : Random
//  random number generator
//  January 26, 2007
//--------------------------------------------------------

#ifndef _OPENNERO_MATH_RANDOM_H_
#define _OPENNERO_MATH_RANDOM_H_

#include "core/Common.h"
#include "core/ONTypes.h"
#include <boost/random/mersenne_twister.hpp>

namespace OpenNero 
{
    /// Custom random number generator
    class RandomNumberGenerator
    {
    public:
        RandomNumberGenerator();
        /// explicit constructor when given a seed
        explicit RandomNumberGenerator( const boost::uint32_t& seed );

        /// uniform integer in [0,2^32-1]
        uint32_t     randI() const;
        /// uniform integer in [0,n]
        uint32_t     randI(const uint32_t& n) const;
        /// uniform real number in [0,1]
        float32_t    randF() const ;
        /// uniform real number in [0,n]
        float32_t    randF(const float32_t& n) const;
        /// uniform real number in [0,1]
        double       randD() const ;
        /// uniform real number in [0,n]
        double       randD(const double& n) const;
        /// normal real number with mean and deviation
        float32_t    normalF(const float32_t& mu, const float32_t& sigma) const;
        /// normal real number with mean and deviation
        double       normalD(const double& mu, const double& sigma) const;
        /// seed with a value
        void seed(const boost::uint32_t& seed) { _randomness.seed(seed); }
    private:
        /// random number generator
        mutable boost::mt19937 _randomness;
    };
    
    extern RandomNumberGenerator RANDOM;

} //end OpenNero

#endif // _OPENNERO_MATH_RANDOM_H_
