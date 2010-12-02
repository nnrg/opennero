#include "core/Common.h"
#include "AI.h"
#include "SensorArray.h"
#include "scripting/scriptIncludes.h"
#include "core/Log.h"
#include "game/SimEntityData.h"
#include "math/Random.h"
#include <vector>
#include <list>
#include <boost/functional/hash.hpp>

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
        vector<double>::const_iterator iter;
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

    /// return an action array for python to use
    template<typename T> Actions get_vector(size_t size)
    {
        return std::vector<T>(size);
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

    size_t hash_value(const FeatureVector& v)
    {
        return boost::hash_value(v);
    }

    size_t hash_value(const StateActionPair& sa_pair)
    {
        return boost::hash_value(sa_pair);
    }
    
    static bool eq_fv(const FeatureVector& v1, const FeatureVector& v2)
    { return v1 == v2; }


    /// @brief export the OpenNERO AI script interface
    PYTHON_BINDER( AI )
    {
        // export bound info
        class_<Bound>("Bound", "Bounds on a single feature (real or discrete)", init<double, double, bool>())
            .def_readonly("min", &Bound::min, "minimum value")
            .def_readonly("max", &Bound::max, "maximum value")
            .def_readonly("discrete", &Bound::discrete, "values discrete?")
            .def(self_ns::str(self_ns::self));
        
        // export bounded array info
        class_<FeatureVectorInfo>("FeatureVectorInfo", "Describe constraints of a feature vector")
            .def("__len__", &FeatureVectorInfo::size, "Length of the feature vector")
            .def(self_ns::str(self_ns::self))
            .def("min", &FeatureVectorInfo::getMin, "Minimal value for an element")
            .def("max", &FeatureVectorInfo::getMax, "Maximal value for an element")
            .def("discrete", &FeatureVectorInfo::isDiscrete, "Is the element discrete or continuous?")
            .def("bound", &FeatureVectorInfo::getBound, "Spec for a particular feature")
            .def("set_discrete", &FeatureVectorInfo::setDiscrete, "Create a discrete element")
            .def("set_continuous", &FeatureVectorInfo::setContinuous, "Create a continuous element")
            .def("add_discrete", &FeatureVectorInfo::addDiscrete, "Add a discrete element")
            .def("add_continuous", &FeatureVectorInfo::addContinuous, "Add a continuous element")
            .def("add", &FeatureVectorInfo::add, "Add an element")
            .def("validate", &FeatureVectorInfo::validate, "Check whether a feature vector is valid")
            .def("normalize", &FeatureVectorInfo::normalize, "Normalize the feature vector given this info")
            .def("denormalize", &FeatureVectorInfo::denormalize, "Create an instance of a feature vector from a vector of values between 0 and 1")
            .def("get_instance", &FeatureVectorInfo::getInstance, "Create a feature vector based on this information")
            .def("random", &FeatureVectorInfo::getRandom, "Create a random feature vector uniformly distributed within bounds");

        // export std::vector<double>
        class_< std::vector<double> > ("DoubleVector", "A vector of real values")
            .def(self_ns::str(self_ns::self))
            .def("__eq__", &eq_fv)
            .def(python::vector_indexing_suite< std::vector<double> >());

        class_<AgentInitInfo>("AgentInitInfo", "Initialization information given to the agent", 
            init<const FeatureVectorInfo&, const FeatureVectorInfo&, const FeatureVectorInfo&>())
            .def_readonly("sensors", &AgentInitInfo::sensors, "Constraints on the agent's sensor feature vector")
            .def_readonly("actions", &AgentInitInfo::actions, "Constraints on the agent's action feature vector")
            .def_readonly("reward", &AgentInitInfo::reward, "Constraints on the agent's reward")
            .def(self_ns::str(self_ns::self));

    }

} // namespace OpenNero
