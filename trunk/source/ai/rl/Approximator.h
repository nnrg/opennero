#ifndef _OPENNERO_AI_RL_APPROXIMATOR_H_
#define _OPENNERO_AI_RL_APPROXIMATOR_H_

#include "core/Common.h"
#include "ai/AI.h"
#include "core/HashMap.h"
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL(Approximator);
    /// @endcond

    /// A function approximator
    class Approximator
    {
    protected:
        /// information about the agent for which this approximator is used
        AgentInitInfo mInfo;
    public:
        /// Constructor
        explicit Approximator(const AgentInitInfo& info) : mInfo(info) {}
        /// Destructor
        virtual ~Approximator() {}

        /// return a copy of this approximator
        virtual ApproximatorPtr copy() const = 0;

        /// predict the value associated with a particular feature vector
        virtual double predict(const FeatureVector& sensors, const FeatureVector& actions) = 0;

        /// update the value associated with a particular feature vector
        virtual void update(const FeatureVector& sensors, const FeatureVector& actions, double target) = 0;
    };

	typedef boost::unordered_map<StateActionPair, double> StateActionDoubleMap;

	/// An exact table-based approximator
    class TableApproximator : public Approximator
    {
    private:
        StateActionDoubleMap table;
    public:
        /// constructor
        explicit TableApproximator(const AgentInitInfo& info);

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
    };

    /// A CMAC tile coding function approximator
    class TilesApproximator : public Approximator
    {
        float mAlpha; ///< learning rate
        std::vector<size_t> ints_index; ///< indeces of integer features
        std::vector<size_t> floats_index; ///< indeces of real features
        size_t num_tilings; ///< number of tiles used
        size_t num_weights; ///< number of weights used (memory size)
        mutable int* ints; ///< integer feature array
        mutable float* floats; ///< real feature array
        mutable int* tiles; ///< tiles array
        float* weights; ///< weight array

        /// convert feature vector into tiles
        void to_tiles(const FeatureVector& sensors, const FeatureVector& actions);
    public:
        /// constructor
        explicit TilesApproximator(const AgentInitInfo& info);

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
    };
}

#endif
