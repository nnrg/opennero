#ifndef _OPENNERO_AI_RL_BACKAPPROXIMATOR_H_
#define _OPENNERO_AI_RL_BACKAPPROXIMATOR_H_

#include "core/Common.h"
#include "ai/AI.h"

namespace OpenNero
{

    /// A CMAC tile coding function approximator
    class BackpropApproximator : public Approximator
    {
            SNetwork network;

        public:
            /// constructor
            explicit BackpropApproximator(const AgentInitInfo& info);

            /// copy constructor
            BackpropApproximator(const BackpropApproximator& a);

            /// destructor
            ~BackpropApproximator();

            /// return a copy of this approximator
            ApproximatorPtr copy() const { ApproximatorPtr p(new BackpropApproximator(*this)); return p; }

            /// predict the value associated with a particular feature vector
            double predict(const FeatureVector& sensors, const FeatureVector& actions);

            /// update the value associated with a particular feature vector
            void update(const FeatureVector& sensors, const FeatureVector& actions, double target);
    };
}

#endif
