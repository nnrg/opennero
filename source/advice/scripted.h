/// @file
/// An interface for controlling agents scripted through advice.

#ifndef _OPENNERO_SCRIPTED_H_
#define _OPENNERO_SCRIPTED_H_

#include <vector>
#include "advice/advice_rep.h"
#include "scripting/scriptIncludes.h"


namespace OpenNero
{
    namespace py = boost::python;

    BOOST_SHARED_DECL(Scripted);

    /// An interface for controlling agents scripted through advice.
    /// It allows a sequence of advice rules to be stored and evaluated.
    class Scripted {
    public:
        /// Constructor
        /// @param numSensors number of sensors
        /// @param numActions number of actions
        Scripted(S32 numSensors, S32 numActions);
        
        /// Destructor
        ~Scripted();

        /// @brief Add new advice to parse into rules
        /// @param advice the string containing advice
        void add_advice(const char* advice);

        /// @brief Evaluate rules for the given sensors
        py::list evaluate(py::list sensors);

        /// @brief Flush the advice data structures.
        void flush();

    private:
        std::vector<Rules*>    mRulesArray;
        std::vector<F64>       mVariables;   // Values of variables
        S32                    mNumSensors;
        S32                    mNumActions;

    };

}

#endif /* _OPENNERO_SCRIPTED_H_ */
