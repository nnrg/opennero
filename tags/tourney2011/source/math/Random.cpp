//--------------------------------------------------------
// OpenNero : Random
//  random number generator
//  January 26, 2007
//--------------------------------------------------------

#include "core/Common.h"
#include <cmath>
#include <cstdlib>
#include "Random.h"
#include <boost/random.hpp> 

namespace OpenNero 
{   
    using namespace boost;

    RandomNumberGenerator::RandomNumberGenerator() : _randomness()    {
    }

    RandomNumberGenerator::RandomNumberGenerator( const boost::uint32_t& seed ) : _randomness(seed)
    {
    }
    
    template <typename Distribution, typename Result>
    Result generate(boost::mt19937& gen, const Result& max)
    {
        Distribution dist(0, max);
        boost::variate_generator<boost::mt19937&, Distribution> vg(gen, dist);
        return vg();
    }

    uint32_t  RandomNumberGenerator::randI() const 
    {
        return generate<boost::uniform_int<uint32_t>, uint32_t>(_randomness, 1);
    }
    
    uint32_t  RandomNumberGenerator::randI(const uint32_t& n) const 
    {
        return generate<boost::uniform_int<uint32_t>, uint32_t>(_randomness, n);
    }

    float32_t RandomNumberGenerator::randF() const
    {
        return generate<boost::uniform_real<float32_t>, float32_t>(_randomness, 1);
    }

    float32_t RandomNumberGenerator::randF(const float32_t& n) const
    {
        return generate<boost::uniform_real<float32_t>, float32_t>(_randomness, n);
    }

    double    RandomNumberGenerator::randD() const
    {
        return generate<boost::uniform_real<float32_t>, float32_t>(_randomness, 1);
    }
    
    double    RandomNumberGenerator::randD(const double& n) const
    {
        return generate<boost::uniform_real<double>, double>(_randomness, n);
    }
    
    /// normal real number with mean and variance
    float32_t    RandomNumberGenerator::normalF(const float32_t& mu, const float32_t& sigma) const
    {
        normal_distribution<float32_t> dist(mu, sigma);
        boost::variate_generator<boost::mt19937&, normal_distribution<float32_t> > vg(_randomness, dist);
        return vg();
    }

    /// normal real number with mean and variance
    double       RandomNumberGenerator::normalD(const double& mu, const double& sigma) const
    {
        normal_distribution<double> dist(mu, sigma);
        boost::variate_generator<boost::mt19937&, normal_distribution<double> > vg(_randomness, dist);
        return vg();
    }

    RandomNumberGenerator RANDOM(55555); // random number generator with default seed
    
    /// set random seed
    void setRandomSeed( const boost::uint32_t& seed )
    {
        RANDOM.seed(seed);
    }
    
} //end OpenNero
