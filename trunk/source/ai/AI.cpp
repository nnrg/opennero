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
        Assert(validate(array));
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
            	LOG_F_DEBUG("ai", "value in position " << i << " was " << array[i] << " which is not between " << lower[i] << " and " << upper[i]);
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
        for (iter = lower.begin(); iter != lower.end(); ++iter)
        {
            result.push_back(*iter);
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
    
    /// enumerate all possible feature vectors (if all-discrete)
    std::list< FeatureVector > FeatureVectorInfo::enumerate() const
    {
        std::list< FeatureVector > result;
        // enumerate all possible actions
        result.push_back(getInstance());
        for (size_t i = 0; i < size(); ++i)
        {
            AssertMsg(isDiscrete(i), "Cannot enumerate continuous features");
            std::list< Actions > new_action_list;
            std::list< Actions >::const_iterator iter;
            for (iter = result.begin(); iter != result.end(); ++iter)
            {
                for (int a = (int)getMin(i); a <= (int)getMax(i); ++a)
                {
                    FeatureVector v = *iter;
                    v[i] = a;
                    new_action_list.push_back(v);
                }
            }
            result = new_action_list;
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

} // namespace OpenNero
