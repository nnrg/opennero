//---------------------------------------------------
// Name: OpenNero : AI
// Desc: Interfaces for an AI system controlling a set
//       of actors in the world
//---------------------------------------------------

#ifndef _OPENNERO_AI_AI_H_
#define _OPENNERO_AI_AI_H_

#include <vector>
#include <string>
#include <iostream>
#include <set>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

#include "core/Common.h"
#include "game/objects/TemplatedObject.h"

namespace OpenNero {

	/// @cond
    BOOST_PTR_DECL(Object);
    BOOST_PTR_DECL(Player);
    BOOST_PTR_DECL(AgentBrain);
    BOOST_PTR_DECL(AI);
    BOOST_PTR_DECL(AIObject);
    BOOST_PTR_DECL(Environment);
    BOOST_SHARED_STRUCT(PlayerAction);
    BOOST_SHARED_DECL(SimEntity);
    BOOST_SHARED_DECL(SimEntityData);
    /// @endcond

    /// A fixed dimension vector of real numbers
    typedef std::vector<double> FeatureVector;

    /// bound for discrete or continuous values
    struct Bound
    {
    public:
        double min; ///< minimum value
        double max; ///< maximum value
        bool discrete; ///< is this value discrete or continuous

        /// constructor
        Bound(double amin, double amax, bool adiscrete);

        /// output to stream
        friend std::ostream& operator<<(std::ostream& os, const Bound& bound);
    };

    /// Information about an array of bounded (discrete or continuous) values
    class FeatureVectorInfo
    {
    private:
        friend class boost::serialization::access;

        std::vector<double> lower; ///< lower bounds
        std::vector<double> upper; ///< upper bounds
        std::vector<bool> discreteness; ///< discreteness

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(lower);
            ar & BOOST_SERIALIZATION_NVP(upper);
            ar & BOOST_SERIALIZATION_NVP(discreteness);
        }
    public:
        FeatureVectorInfo() : lower(), upper(), discreteness() { }

        /// constructor
        explicit FeatureVectorInfo(int size) : lower(size), upper(size), discreteness(size) { }

        /// number of elements
        size_t size() const { return lower.size(); }

        /// min of ith element
        double getMin(size_t i) const { return lower[i]; }

        /// max of ith element
        double getMax(size_t i) const { return upper[i]; }

        /// discreteness of ith element
        bool isDiscrete(size_t i) const { return discreteness[i]; }

        /// change a discrete bound
        void setDiscrete(size_t i, int min, int max);

        /// change a continuous bound
        void setContinuous(size_t i, double min, double max);

        /// add a discrete bound
        size_t addDiscrete(int min, int max);

        /// add a continuous bound
        size_t addContinuous(double min, double max);

        /// add a bound
        size_t add(const Bound& bound);

        /// validate a vector of values against these bounds
        bool validate(const FeatureVector& array) const;

        /// normalize the feature vector to the range 0-1
        FeatureVector normalize(const FeatureVector& array) const;

        /// given a feature vector in the range 0-1, make a feature vector with these constraints
		FeatureVector denormalize(const FeatureVector& array) const;

        /// create a vector of the appropriate size
        FeatureVector getInstance() const;

        /// create a feature vector initialized randomly
        FeatureVector getRandom() const;

        /// get the bounds on a particular element
        Bound getBound(size_t i);

        /// write to a stream
        friend std::ostream& operator<<(std::ostream& out, const FeatureVectorInfo& obj);

        /// convert from a string
        struct Converter
        {
            bool operator()(FeatureVectorInfo& result, const std::string& typeString);
        };
    };

    typedef FeatureVectorInfo SensorInfo; ///< Sensor/feature vector descriptor
    typedef FeatureVectorInfo ActionInfo; ///< Action vector descriptor
    typedef FeatureVectorInfo RewardInfo; ///< Reward descriptor

    /// Initial information given to an agent
    struct AgentInitInfo
    {
        AgentInitInfo() : sensors(0), actions(0), reward(0) {}

        /**
        * Component constructor
        * @param s sensor description
        * @param a action description
        * @param r reward description
        */
        AgentInitInfo(const SensorInfo& s, const ActionInfo& a, const RewardInfo& r)
        : sensors(s), actions(a), reward(r)
        {
        }

        /**
        * Default constructor - everything is continous and between 0 and 1
        * @param nSensors number of sensor features
        * @param nActions number of actions
        * @param nRewards number of rewards
        */
        AgentInitInfo(U32 nSensors, U32 nActions, U32 nRewards);

        static const U32 version = 1; ///< version of this info structure (increment with every change)
        SensorInfo sensors; ///< infromation about the possible observations
        ActionInfo actions; ///< information about the possible actions
        RewardInfo reward; ///< information about the reward values

        friend std::ostream& operator<<(std::ostream& out, const AgentInitInfo& obj);

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(sensors);
            ar & BOOST_SERIALIZATION_NVP(actions);
            ar & BOOST_SERIALIZATION_NVP(reward);
        }
    };

    typedef FeatureVector Observations; ///< Observations coming from sensors

    typedef FeatureVector Actions; ///< Actions type

    typedef FeatureVector Reward; ///< Reward type

    /// set of agents
    typedef std::set<AgentBrainPtr> AgentSet;
    
    typedef std::pair<Observations, Actions> StateActionPair;

    /// hash value of a feature vector
    size_t hash_value(const FeatureVector& fv);
    
    /// hash value of a state-action pair
    size_t hash_value(const StateActionPair& sa_pair);
    
    /// addition operator for Reward, Observation and Action vectors
    FeatureVector operator+(const FeatureVector& left, const FeatureVector& right);
    
    /// addition operator for Reward, Observation and Action vectors
    FeatureVector& operator+=(FeatureVector& left, const FeatureVector& right);

    /// subtraction operator for Reward, Observation and Action vectors
    FeatureVector operator-(const FeatureVector& left, const FeatureVector& right);
    
    /// subtraction operator for Reward, Observation and Action vectors
    FeatureVector& operator-=(FeatureVector& left, const FeatureVector& right);

    /// scalar multiplication operator for Reward, Observation and Action vectors
    FeatureVector operator*(const FeatureVector& left, const double& right);
    
    /// scalar multiplication operator for Reward, Observation and Action vectors
    FeatureVector& operator*=(FeatureVector& left, const double& right);

    /// scalar division operator for Reward, Observation and Action vectors
    FeatureVector operator/(const FeatureVector& left, const double& right);

    /// scalar division operator for Reward, Observation and Action vectors
    FeatureVector& operator/=(FeatureVector& left, const double& right);

    /// An AI is a global "puppeteer" algorithm that manages the resources necessary
    /// to do learning or intelligent behavior. Examples of AI algorithms include population
    /// methods such as neuroevolution (rtNEAT), online learning methods such as Q-learning or
    /// Sarsa, or non-adaptive behaviors such as A* path finding.
    class AI : public TemplatedObject
    {
    public:
        virtual ~AI() {}

        /// update our entity base on our object access mode
        virtual void ProcessTick( float32_t incAmt ) = 0;
    };

}

#endif
