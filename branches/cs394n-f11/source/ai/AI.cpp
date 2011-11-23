#include "core/Common.h"
#include "AI.h"
#include "game/SimEntityData.h"
#include "math/Random.h"
#include <vector>
#include <list>

namespace OpenNero
{
    /// constructor
    Bound::Bound(double amin, double amax, bool adiscrete) 
        : min(amin)
        , max(amax)
        , discrete(adiscrete)
    {
    }

    /// output to stream
    std::ostream& operator<<(std::ostream& os, const Bound& bound)
    {
        os << "<bound min=\""<< bound.min << "\" max=\"" << bound.max
           << "\" discrete=\""<< bound.discrete << "\" />";
        return os;
    }

    void FeatureVectorInfo::setContinuous(size_t i, double min, double max)
    {
        Assert(min < max);
        lower[i] = min;
        upper[i] = max;
        discreteness[i] = false;
    }

    size_t FeatureVectorInfo::addContinuous(double min, double max)
    {
        Assert(min < max);
        lower.push_back(min);
        upper.push_back(max);
        discreteness.push_back(false);
        return size() - 1;
    }

    void FeatureVectorInfo::setDiscrete(size_t i, int min, int max)
    {
        Assert(min < max);
        lower[i] = min;
        upper[i] = max;
        discreteness[i] = true;
    }

    size_t FeatureVectorInfo::addDiscrete(int min, int max)
    {
        Assert(min < max);
        lower.push_back(min);
        upper.push_back(max);
        discreteness.push_back(true);
        return size() - 1;
    }

    /// add a bound
    size_t FeatureVectorInfo::add(const Bound& bound)
    {
        if (bound.discrete)
        {
            return addDiscrete(static_cast<int>(bound.min), static_cast<int>(bound.max));
        }
        else
        {
            return addContinuous(bound.min, bound.max);
        }
    }

    /// normalize the feature vector to the range 0-1
    FeatureVector FeatureVectorInfo::normalize(const FeatureVector& array) const
    {
        AssertMsg(validate(array), "the feature vector " << array << " is invalid in normalize()");
        FeatureVector result(array);
        for (size_t i = 0; i < size(); ++i)
        {
            double d = upper[i] - lower[i];
            if (d > 0 && d != 1)
            {
                result[i] = (array[i] - lower[i]) / d;
            }
        }
        return result;
    }

    /// given a feature vector in the range 0-1, make a feature vector with these constraints
    FeatureVector FeatureVectorInfo::denormalize(const FeatureVector& array) const
	{
    	FeatureVector result(array);
    	for (size_t i = 0; i < size(); ++i)
    	{
    		// 0 will be = lower, 1 will be = upper
    		double d = upper[i] - lower[i];
    		result[i] = array[i] * d + lower[i];
    	}
    	return result;
	}

    /// validate a vector of values against these bounds
    bool FeatureVectorInfo::validate(const FeatureVector& array) const
    {
        if (array.size() != size()) return false;
        for (size_t i = 0; i < size(); ++i)
        {
            if (array[i] < lower[i] || array[i] > upper[i])
            {
            	//LOG_F_DEBUG("ai", "value in position " << i << " was " << array[i] << " which is not between " << lower[i] << " and " << upper[i]);
                return false;
            }
        }
        return true;
    }

    /// create a feature vector based on this FeatureVectorInfo
    FeatureVector FeatureVectorInfo::getInstance() const
    {
        FeatureVector result;
        FeatureVector::const_iterator iter;
        for (size_t i = 0; i < size(); ++i)
        {
            // if 0 is an in-bounds value, use it
            if (lower[i] <= 0 && 0 <= upper[i]) {
                result.push_back(0);
            }
            else // otherwise, use the lower bound
            {
                result.push_back(lower[i]);
            }
        }
        return result;
    }
    
    FeatureVector FeatureVectorInfo::getRandom() const
    {
        FeatureVector result;
        for (size_t i = 0; i < size(); ++i)
        {
            if (isDiscrete(i))
            {
                result.push_back(RANDOM.randI( (uint32_t)(getMax(i) - getMin(i)) ) + getMin(i));
            }
            else
            {
                result.push_back(RANDOM.randD(getMax(i) - getMin(i)) + getMin(i));
            }
        }
        return result;
    }
    
    /// get a bounded array info from a string
    bool FeatureVectorInfo::Converter::operator()(FeatureVectorInfo& result, const std::string& typeString)
    {
        std::stringstream ss(typeString);
        bool discrete;
        while (ss >> discrete)
        {
            if (discrete)
            {
                int min, max;
                if ( !(ss >> min) || !(ss >> max) ) 
                {
                    return false;
                }
                result.addDiscrete(min, max);
            }
            else
            {
                double min, max;
                if ( !(ss >> min) || !(ss >> max) ) 
                {
                    return false;
                }
                result.addContinuous(min, max);
            }
        }
        return true;
    }
    
    /// get the bounds on a particular element
    Bound FeatureVectorInfo::getBound(size_t i)
    {
        return Bound(getMin(i), getMax(i), isDiscrete(i));
    }

    AgentInitInfo::AgentInitInfo(U32 nSensors, U32 nActions, U32 nRewards) :
    sensors(nSensors), actions(nActions), reward(nRewards)
    {
        for (size_t i = 0; i < nSensors; ++i) sensors.setContinuous(i,0,1);
        for (size_t i = 0; i < nActions; ++i) actions.setContinuous(i,0,1);
        for (size_t i = 0; i < nRewards; ++i) reward.setContinuous(i,0,1);
    }

    /// print the bounded array info to stream
    std::ostream& operator<<(std::ostream& out, const FeatureVectorInfo& obj)
    {
        assert(obj.lower.size() == obj.upper.size() && obj.discreteness.size() == obj.upper.size());
        for (size_t i = 0; i < obj.lower.size(); ++i)
        {
            out << (obj.discreteness[i] ? 'd' : 'c') << "[" << obj.lower[i] << ", " << obj.upper[i] << "] ";
        }
        return out;
    }

    /// print the agent init info to stream
    std::ostream& operator<<(std::ostream& out, const AgentInitInfo& obj)
    {
        out << "<AgentInitInfo sensors=\"" << obj.sensors
            << "\" actions=\"" << obj.actions
            << "\" reward=\"" << obj.reward << "\" />";
        return out;
    }
    
    FeatureVector operator+(const FeatureVector& left, const FeatureVector& right)
    {
        Assert(left.size() == right.size());
        FeatureVector result(left);
        FeatureVector::const_iterator ileft = left.begin();
        FeatureVector::const_iterator iright = right.begin();
        for (size_t i = 0; i < left.size(); ++i)
        {
            result[i] = *ileft + *iright;
            ++ileft;
            ++iright;
        }
        return result;
    }
    
    FeatureVector& operator+=(FeatureVector& left, const FeatureVector& right)
    {
        Assert(left.size() == right.size());
        FeatureVector::iterator ileft = left.begin();
        FeatureVector::const_iterator iright = right.begin();
        while (ileft != left.end() && iright != right.end())
        {
            *ileft += *iright;
            ++ileft;
            ++iright;
        }
        return left;
    }
    
    FeatureVector operator-(const FeatureVector& left, const FeatureVector& right)
    {
        Assert(left.size() == right.size());
        FeatureVector result(left);
        FeatureVector::const_iterator ileft = left.begin();
        FeatureVector::const_iterator iright = right.begin();
        for (size_t i = 0; i < left.size(); ++i)
        {
            result[i] = *ileft - *iright;
            ++ileft;
            ++iright;
        }
        return result;
    }
    
    FeatureVector& operator-=(FeatureVector& left, const FeatureVector& right)
    {
        Assert(left.size() == right.size());
        FeatureVector::iterator ileft = left.begin();
        FeatureVector::const_iterator iright = right.begin();
        while (ileft != left.end() && iright != right.end())
        {
            *ileft -= *iright;
            ++iright;
            ++ileft;
        }
        return left;
    }
    
    FeatureVector operator*(const FeatureVector& left, const double& right)
    {
        FeatureVector result(left);
        FeatureVector::const_iterator ileft = left.begin();
        for (size_t i = 0; i < left.size(); ++i)
        {
            result[i] = *ileft * right;
            ++ileft;
        }
        return result;
    }
    
    FeatureVector& operator*=(FeatureVector& left, const double& right)
    {
        FeatureVector::iterator ileft = left.begin();
        while (ileft != left.end())
        {
            *ileft *= right;
            ++ileft;
        }
        return left;
    }
    
    FeatureVector operator/(const FeatureVector& left, const double& right)
    {
        FeatureVector result(left);
        FeatureVector::const_iterator ileft = left.begin();
        for (size_t i = 0; i < left.size(); ++i)
        {
            result[i] = *ileft / right;
            ++ileft;
        }
        return result;
    }
    
    FeatureVector& operator/=(FeatureVector& left, const double& right)
    {
        FeatureVector::iterator ileft = left.begin();
        while (ileft != left.end())
        {
            *ileft /= right;
            ++ileft;
        }
        return left;
    }

} // namespace OpenNero
