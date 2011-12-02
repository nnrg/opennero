#ifndef _OPENNERO_AI_RL_APPROXIMATOR_H_
#define _OPENNERO_AI_RL_APPROXIMATOR_H_

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include "core/Common.h"
#include "ai/AI.h"
#include "core/HashMap.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL(Approximator);
    /// @endcond

    /// A function approximator
    class Approximator
    {
    protected:
        friend class boost::serialization::access;
        /// information about the agent for which this approximator is used
        AgentInitInfo mInfo;
    public:
        /// Constructor
        Approximator() {}
        explicit Approximator(const AgentInitInfo& info) : mInfo(info) {}
        /// Destructor
        virtual ~Approximator() {}

        /// return a copy of this approximator
        virtual ApproximatorPtr copy() const = 0;

        /// predict the value associated with a particular feature vector
        virtual double predict(const FeatureVector& sensors, const FeatureVector& actions) = 0;

        /// update the value associated with a particular feature vector
        virtual void update(const FeatureVector& sensors, const FeatureVector& actions, double target) = 0;

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(mInfo);
        }
    };

	typedef boost::unordered_map<StateActionPair, double> StateActionDoubleMap;

	/// An exact table-based approximator
    class TableApproximator : public Approximator
    {
    private:
        friend class boost::serialization::access;
        StateActionDoubleMap table;
        int action_bins;
        int state_bins;
    public:
        /// constructor
        TableApproximator() {}
        explicit TableApproximator(const AgentInitInfo& info, const int action_bins, const int state_bins);
        explicit TableApproximator(const AgentInitInfo& info)
        {
            TableApproximator(info, 3, 5);
        }


        /// copy constructor
        TableApproximator(const TableApproximator& a);

        /// destructor
        ~TableApproximator();

        /// return a copy of this approximator
        ApproximatorPtr copy() const 
            { ApproximatorPtr p(new TableApproximator(*this)); return p; }

        /// predict the value associated with a particular feature vector
        double predict(const FeatureVector& sensors, const FeatureVector& actions);

        /// update the value associated with a particular feature vector
        void update(const FeatureVector& sensors, const FeatureVector& actions, double target);

        /// quantize continuous state or action vectors
        FeatureVector quantize_action(const FeatureVector& continuous) const;
        FeatureVector quantize_state(const FeatureVector& continuous) const;

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & boost::serialization::base_object<Approximator>(*this);
            ar & BOOST_SERIALIZATION_NVP(action_bins);
            ar & BOOST_SERIALIZATION_NVP(state_bins);
            StateActionDoubleMap::iterator iter;
            for (iter = table.begin(); iter != table.end(); ++iter)
                ar & BOOST_SERIALIZATION_NVP(*iter);
        }
    };

    /// A CMAC tile coding function approximator
    class TilesApproximator : public Approximator
    {
    private:
        friend class boost::serialization::access;

        float mAlpha; ///< learning rate
        std::vector<size_t> ints_index; ///< indeces of integer features
        std::vector<size_t> floats_index; ///< indeces of real features
        std::vector<int> ints; ///< integer feature array
        std::vector<float> floats; ///< real feature array
        std::vector<int> tiles; ///< tiles array
        std::vector<float> weights; ///< weight array

        /// convert feature vector into tiles
        void to_tiles(const FeatureVector& sensors, const FeatureVector& actions);
    public:
        /// constructors
        TilesApproximator() {}
        explicit TilesApproximator(const AgentInitInfo& info, const int num_tiles, const int num_weights);
        explicit TilesApproximator(const AgentInitInfo& info)
        {
            TilesApproximator(info, 32, 1024);
        }

        /// copy constructor
        TilesApproximator(const TilesApproximator& a);

        /// destructor
        ~TilesApproximator();

        /// return a copy of this approximator
        ApproximatorPtr copy() const { ApproximatorPtr p(new TilesApproximator(*this)); return p; }

        /// predict the value associated with a particular feature vector
        double predict(const FeatureVector& sensors, const FeatureVector& actions);

        /// update the value associated with a particular feature vector
        void update(const FeatureVector& sensors, const FeatureVector& actions, double target);

        /// serialize this object to/from a Boost serialization archive
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & boost::serialization::base_object<Approximator>(*this);
            ar & BOOST_SERIALIZATION_NVP(mAlpha);
            ar & BOOST_SERIALIZATION_NVP(ints_index);
            ar & BOOST_SERIALIZATION_NVP(floats_index);
            ar & BOOST_SERIALIZATION_NVP(ints);
            ar & BOOST_SERIALIZATION_NVP(floats);
            ar & BOOST_SERIALIZATION_NVP(tiles);
            ar & BOOST_SERIALIZATION_NVP(weights);
        }
    };

}

#endif
