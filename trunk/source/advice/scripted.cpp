#include "advice/scripted.h"

/// yacc/lex parsing function defined in advice/advice.y
extern Rules* yyParseAdvice(const char* advice, U32 numSensors, U32 numActions, Agent::Type type);

namespace OpenNero
{
    using namespace NEAT;

    Scripted::Scripted(S32 numSensors, S32 numActions) :
            mRulesArray(), mVariables(numSensors+numActions, 0.0),
            mNumSensors(numSensors), mNumActions(numActions)
    {
    }


    Scripted::~Scripted() {
        for (unsigned int i = 0; i < mRulesArray.size(); i++) {
            delete mRulesArray[i];
        }
    }


    void Scripted::add_advice(const char* advice) {
        // Parse the advice.
        Rules* parse_result = yyParseAdvice(advice, mNumSensors, mNumActions, Agent::eScripted);
        if (parse_result != NULL && parse_result->size() > 0) {
            mRulesArray.push_back(parse_result);
        }
    }


    py::list Scripted::evaluate(py::list sensors) {
        AssertMsg(py::len(sensors) <= mNumSensors, "more sensors given than available");
        for (py::ssize_t i = 0; i < py::len(sensors); i++) {
            mVariables[i] = py::extract<F64>(sensors[i]);
        }
        
        for (unsigned int i = 0; i < mRulesArray.size(); i++) {
            mRulesArray[i]->evaluate(mVariables);
        }

        py::list actions;
        for (unsigned int i = mNumSensors; i < mNumSensors+mNumActions; i++) {
            actions.append(mVariables[i]);
        }
        
        return actions;
    }


    void Scripted::flush() {
        mVariables.clear();
        mVariables.resize(mNumSensors+mNumActions, 0.0);
    }
}
