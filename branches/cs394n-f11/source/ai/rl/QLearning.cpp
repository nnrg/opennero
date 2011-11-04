#include "core/Common.h"
#include "math/Random.h"
#include "Approximator.h"
#include "QLearning.h"
#include <cfloat>
#include <vector>

namespace OpenNero
{
	double QLearningBrain::predict(const Observations& new_state) {
		double max_value = -DBL_MAX;
		std::vector< Actions >::const_iterator iter;
		for (iter = action_list.begin(); iter != action_list.end(); ++iter)
		{
			double value = mApproximator->predict(new_state, *iter);
			if (value > max_value)
			{
				max_value = value;
			}
		}
		return max_value;
	}
}
