#ifndef _OPENNERO_ADVICE_REP_H_
#define _OPENNERO_ADVICE_REP_H_

#include "rtneat/genome.h"
#include "rtneat/population.h"
#include "ai/AI.h"
#include <cstdio>
#include <vector>

// The advice is converted to neural networks using techniques from
// KBANN (towell:phd91; see chapter 2) and RATLE (maclin:phd95; see
// chapter 6):
// http://ftp.cs.wisc.edu/machine-learning/shavlik-group/towell.thesis.1-2.pdf
// http://www.d.umn.edu/~rmaclin/publications/maclin.thesis.firsthalf.pdf
// http://www.d.umn.edu/~rmaclin/publications/maclin.thesis.secondhalf.pdf

// abstract syntax tree for the advice grammar

using namespace NEAT;

struct ANode {
public:
    virtual ~ANode() {}
    virtual void print() const = 0;
};


// Abstract class for a term
struct Term : public ANode {
    virtual ~Term() {}

    /// @brief This method creates the neural network representation of the term
    ///     in the specified genome.  When it returns, the node representing the
    ///     output of the term is the last node in the node vector of the genome.
    /// @param population the population being evolved
    /// @param biasnode the bias node of the genome representation
    /// @param genome the advice representation being built
    virtual void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const = 0;


    /// @brief This method evaluates the term and produces a Boolean result.
    /// @param variables the vector of variables
    /// @return the Boolean result of evaluating the term
    virtual bool evaluate(std::vector<F64>& variables) const = 0;
};


// Class for terms in an expression.
struct Eterm : public ANode {
    explicit Eterm(F64 val) : mValue(val), mVariable(0), mType(eValue), mWeight1(0.0), mWeight2(mOmega*(val-2.5)), mWeight3(2*mOmega) {}
    explicit Eterm(U32 var, F64 val=1.0) : mValue(val), mVariable(var), mType(eVariable), mWeight1(mOmega), mWeight2(-mOmega*2.5), mWeight3(2*mOmega) {}
    ~Eterm() {}

    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    F64 evaluate(std::vector<F64>& variables) const;

    void negate() {
        mValue = -mValue;
        if (mType == eValue) {
            mWeight2 = mOmega*(mValue-2.5);
        }
    }

    F64 getScale() const {
        if (mType == eValue) return 1.0;
        else return mValue;
    }

private:
    // Enum to denote the type of the term.
    enum Type {eValue, eVariable};

    F64 mValue;
    U32 mVariable;
    Type mType;

    // Approximate y = x with y = sigmoid(4*x - 2), which is exact for x = 0.5.
    // Note that this works because the sigmoid has slope 1/4 at y = 0.5.
    // Since we use this rule as the consequent of a conditional, we modify it
    // as y = sigmoid(4*x - 2 - 8 + 8*cond), i.e. if cond ~ 0, then y ~ 0 and
    // if cond ~ 1, then y ~ sigmoid(4*x - 2).
    static const F64 mOmega;
    F64 mWeight1; // weight of connection from var node
    F64 mWeight2; // weight of connection from bias node
    F64 mWeight3; // weight of connection from condition node
};


// Class for an expression, containing a summation of Eterms.
struct Expr : public ANode {
    Expr() : mEterms() {}
    Expr(const std::vector<Eterm*>& eterms) : mEterms(eterms) {}
    ~Expr() {
        for (std::vector<Eterm*>::iterator i = mEterms.begin(); i != mEterms.end(); ++i) {
            delete (*i);
        }
    }
    void append(Eterm* e) {
        mEterms.push_back(e);
    }
    const std::vector<Eterm*>& getEterms() {
        return mEterms;
    }
    void print() const;
    F64 evaluate(std::vector<F64>& variables) const;

private:
    std::vector<Eterm*> mEterms;
};


// Class for setting values of variables
struct SetVar : public ANode {
    SetVar(U32 var, Expr* expr) : mVariable(var), mExpr(expr), mWeight1(mOmega), mWeight2(-mOmega*0.5) {}
    ~SetVar() {
        delete mExpr;
    }

    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    void evaluate(std::vector<F64>& variables) const;

private:
    U32 mVariable;
    Expr* mExpr;

    // Approximate y = x with y = sigmoid(4*x - 2), which is exact for x = 0.5.
    // Note that this works because the sigmoid has slope 1/4 at y = 0.5.
    static const F64 mOmega;
    F64 mWeight1;
    F64 mWeight2;
};


/// singleton deletion class
template <class T>
class Cleanup {
public:
    /// @param ptr an object to be deleted on destruction of cleanup
    Cleanup(T* ptr = NULL) : mPtr(ptr) {}
    ~Cleanup() { if (mPtr != NULL) { printf("cleanup on isle %x", mPtr); delete mPtr; } }
    T* getObject() const { return mPtr; }
    void setObject(T* ptr) { mPtr = ptr; }

private:
    T* mPtr;
};


struct BooleanTerm : public Term {
    /// @todo make singleton
    /// for now we assume that client deletes
    static BooleanTerm* getTrue() {
        //static Cleanup<BooleanTerm> c;
        //if (c.getObject() == NULL) {
        //    c.setObject(new BooleanTerm(true));
        //}
        //return c.getObject();
        return new BooleanTerm(true);
    }
    /// @todo make singleton
    /// for now we assume that client deletes
    static BooleanTerm* getFalse() {
        //static Cleanup<BooleanTerm> c;
        //if (c.getObject() == NULL) {
        //    c.setObject(new BooleanTerm(false));
        //}
        //return c.getObject();
        return new BooleanTerm(false);
    }

    void print() const { printf(mValue ? "true " : "false "); }
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    bool evaluate(std::vector<F64>& variables) const { return mValue; }

private:
    // true/false is represented by a node that has high/low activation, and is
    // obtained by connecting from the bias node with a large positive/negative
    // weight.
    static const F64 mOmega;
    F64 mWeight;

    bool mValue;

    explicit BooleanTerm(bool val) : mValue(val), mWeight(val ? mOmega : -mOmega) {}
};


struct BinaryTerm : public Term {
    BinaryTerm(U32 var, F64 rhs, F64 w1, F64 w2, F64 w3) :
            mVariable(var), mRHSVal(rhs), mWeight1(w1), mWeight2(w2), mWeight3(w3), mRHSType(eValue) {}
    BinaryTerm(U32 var, U32 rhs, F64 w1, F64 w2, F64 w3) :
            mVariable(var), mRHSVar(rhs), mWeight1(w1), mWeight2(w2), mWeight3(w3), mRHSType(eVariable) {}
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    bool evaluate(std::vector<F64>& variables) const;
    virtual bool compare(F64 lhs, F64 rhs) const = 0;

protected:
    // Suppose we want to construct a network node that produces the activation
    // for GTTerm corresponding to the truth value of mVariable > mRHS.
    // Assume true is represented by high activation (greater than 0.9) and false
    // is represented by low activation (less than 0.1) of this node.  It receives
    // activation a1 and a2 from nodes representing mVariable and mRHS
    // respectively.  Then, the weights of these connections have the same
    // magnitude mOmega given by the following expression:
    //   mOmega(a1 - a2) >= 2.197 (since sigmoid(2.197) is approximately 0.9)
    // Since the range of inputs is usually [-1, 1] or [0, 1], we will use
    // approximately 10% of it as the fuzzy range [-0.1, 0.1] to match (a1 - a2),
    // i.e. we want high activation for a1 - a2 >= 0.1 and low activation for
    // a1 - a2 <= -0.1
    // These assumptions yield the following value for mOmega.
    static const F64 mOmega;
    //static const F64 mOmega = 45.95;  // sigmoid(4.595) ~ 0.99

    // If a2 = mRHSVal, i.e. a constant, it can be represented by the bias node.
    // However, since the activation of the bias node is 1, the connection weight
    // from it will have to be adjusted to compensate.
    //   -mOmega*a2 = -mOmega*mRHSVal = mWeight3*1
    // Therefore, mWeight1 = mOmega and mWeight3 = -mOmega*mRHSVal

    // Similar calculations for the other binary terms yield the same value for
    // mOmega, but different values for mWeight1 and mWeight3, and they will be
    // calculated in their respective constructors.
    F64 mWeight1;
    F64 mWeight2;
    F64 mWeight3;

    // Enum to denote whether the RHS is a value or a variable.
    enum RHSType {eValue, eVariable};

    U32 mVariable;
    F64 mRHSVal;
    U32 mRHSVar;
    RHSType mRHSType;
};


/// less-than-or-equal
struct LEQTerm : public BinaryTerm {
    // Similar to the calculation of mOmega for GEQTerm, but use (a2 - a1) instead
    // of (a1 - a2) and add 0.1 to a1 instead of to a2.
    LEQTerm(U32 var, F64 rhs) : BinaryTerm(var, rhs, -mOmega, 0.0, mOmega*(rhs - 0.1)) {}
    LEQTerm(U32 var, U32 rhs) : BinaryTerm(var, rhs, -mOmega, mOmega, -mOmega*0.1) {}
    void print() const;
    bool compare(F64 lhs, F64 rhs) const { return lhs <= rhs; }
};


/// less-than
struct LTTerm : public BinaryTerm {
    // Similar to the calculation of mOmega for GTTerm, but use (a2 - a1) instead
    // of (a1 - a2), which does not change the value of mOmega, but flips the
    // signs of the weights.
    LTTerm(U32 var, F64 rhs) : BinaryTerm(var, rhs, -mOmega, 0.0, mOmega*rhs) {}
    LTTerm(U32 var, U32 rhs) : BinaryTerm(var, rhs, -mOmega, mOmega, 0.0) {}
    void print() const;
    bool compare(F64 lhs, F64 rhs) const { return lhs < rhs; }
};


/// greater-than-or-equal
struct GEQTerm : public BinaryTerm {
    // Similar to the calculation of mOmega for GTTerm, but 0.1 is added to a2 to
    // shift the fuzzy interval to the right, increasing the magnitude of mWeight3
    // by 0.1*omega.
    GEQTerm(U32 var, F64 rhs) : BinaryTerm(var, rhs, mOmega, 0.0, -mOmega*(rhs + 0.1)) {}
    GEQTerm(U32 var, U32 rhs) : BinaryTerm(var, rhs, mOmega, -mOmega, -mOmega*0.1) {}
    void print() const;
    bool compare(F64 lhs, F64 rhs) const { return lhs >= rhs; }
};


/// greater-than term
struct GTTerm : public BinaryTerm {
    // See comments on how weights are calculated in BinaryTerm.
    GTTerm(U32 var, F64 rhs) : BinaryTerm(var, rhs, mOmega, 0.0, -mOmega*rhs) {}
    GTTerm(U32 var, U32 rhs) : BinaryTerm(var, rhs, mOmega, -mOmega, 0.0) {}
    void print() const;
    bool compare(F64 lhs, F64 rhs) const { return lhs > rhs; }
};


struct Conds : public ANode {
    Conds() : mTerms(), mWeight1(mOmega), mWeight2(mOmega/2.0) {}
    Conds(const std::vector<Term*>& terms) : mTerms(terms), mWeight1(mOmega), mWeight2(mOmega*(-2.0*terms.size()+1.0)/2.0) {}
    ~Conds() {
        for (std::vector<Term*>::iterator i = mTerms.begin(); i != mTerms.end(); ++i) {
            delete (*i);
        }
    }
    void append(Term* t) {
        mTerms.push_back(t);
        mWeight2 = mOmega*(-2.0*mTerms.size()+1.0)/2.0;
    }
    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    bool evaluate(std::vector<F64>& variables) const;

private:
    // Conds is a conjunction of terms; its output node is created using connections
    // from its constituent terms and the bias node.  See towell:phd91 (page 23-24)
    // for weight calculation.
    static const F64 mOmega;
    F64 mWeight1;
    F64 mWeight2;

    std::vector<Term*> mTerms;
};


struct Rule : public ANode {
    virtual ~Rule() {}
    virtual void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const = 0;
    virtual void evaluate(std::vector<F64>& variables) const = 0;
};


struct IfRule : public Rule {
    IfRule(Conds* conds, Rule* then, Rule* els = NULL) : mConds(conds), mThen(then), mElse(els),
                                                         mWeight1(mOmega), mWeight2(mOmega/2) {}
    ~IfRule() { 
        delete mConds;
        delete mThen;
        if (mElse != NULL) delete mElse;
    }
    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    void evaluate(std::vector<F64>& variables) const;

private:
    // The then rule is attached to the condition, and the else rule is attached to
    // an else node created by negating the condition.  The negation is obtained by
    // a large negative weighted connection from the condition node and a smaller
    // positive weighted connection from the bias node.
    static const F64 mOmega;
    F64 mWeight1;
    F64 mWeight2;

    Conds* mConds;
    Rule* mThen;
    Rule* mElse;
};


struct SetRules : public Rule {
    SetRules() : mSetVars() {}
    SetRules(const std::vector<SetVar*>& setvars) : mSetVars(setvars) {}
    ~SetRules() {
        for (std::vector<SetVar*>::iterator i = mSetVars.begin(); i != mSetVars.end(); ++i) {
            delete (*i);
        }
    }
    void append(SetVar* sv) {
        mSetVars.push_back(sv);
    }
    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    void evaluate(std::vector<F64>& variables) const;

private:
    std::vector<SetVar*> mSetVars;
};


struct Rules : public ANode {
    Rules() : mRules(), mWeight1(mOmega), mWeight2(mOmega/2.0) {}
    Rules(const std::vector<Rule*>& rules) : mRules(rules), mWeight1(mOmega), mWeight2(mOmega*(-2.0*rules.size()+1.0)/2.0)  {}
    ~Rules() {
        for (std::vector<Rule*>::iterator i = mRules.begin(); i != mRules.end(); ++i) {
            delete (*i);
        }
    }
    U32 size() const { return mRules.size(); }
    void append(Rule* r) {
        mRules.push_back(r);
        mWeight2 = mOmega*(-2.0*mRules.size()+1.0)/2.0;
    }
    void print() const;
    void buildRepresentation(PopulationPtr population, NNodePtr biasnode, GenomePtr genome, std::vector<NNodePtr>& variables, NNodePtr& ionode) const;
    void evaluate(std::vector<F64>& variables) const;

private:
    // We need to construct the conjunction of the last else nodes of all the rules.
    // The weights below are therefore the same as for Conds, which constructs the
    // conjunction of terms.
    static const F64 mOmega;
    F64 mWeight1;
    F64 mWeight2;

    std::vector<Rule*> mRules;
};


// Namespace for handling variables.
namespace Variable {
    // Enum to denote the type of variable.
    enum Type {eSensor, eEvolvedAction, eAction, eGeneral};

    // Number of sensors and actions in the network.
    extern U32 mNumSensors;
    extern U32 mNumActions;

    // Translate variable of a given type and index into an unsigned integer.
    U32 translate(Type type, U32 index);

    // Function to handle error while translating variables.
    void translateError(Type type, U32 index, std::string message);

    // Given a translated variable, return its type.
    Type getType(U32 var);

    // Given a translated variable, return its index.
    U32 getIndex(U32 var);

    // Convert a translated variable to string suitable for printing.
    std::string toString(U32 var);
    
    // Convert the given type and index to string suitable for printing.
    std::string toString(Type type, U32 index);
    
    // Get the neural network node corresponding to the given translated variable.
    NNodePtr getNode(PopulationPtr population, GenomePtr genome, std::vector<NNodePtr>& variables, U32 var);

    // Get the value of the given translated variable.
    F64 getValue(std::vector<F64>& variables, U32 var);

    // Set the value of the given translated variable.
    void setValue(std::vector<F64>& variables, U32 var, F64 val);

    // Construct rules for setting all action variables to evolved action variables.
    SetRules* setActionsToEvolvedActions();
}


// Namespace for handling numerical values.
namespace Number {
    // Bounds on sensors in the network and advice language.  These bounds are used to
    // convert sensor values between network and advice.
    extern FeatureVectorInfo mSensorBoundsNetwork;
    extern FeatureVectorInfo mSensorBoundsAdvice;

    // Since non-sensor variables have sigmoid output, we use the approximately linear
    // region of the sigmoid, i.e. [0.2, 0.8], as the range for values in the network
    // and [-1, 1] as the range for values in the advice.

    // The values converted in this manner are the numerical constants in comparison and
    // assignment expressions, and the variable on the left-hand side of the expression
    // determines the type of conversion.
    F64 toNetwork(F64 val);
    F64 toNetwork(F64 val, U32 var);

    // This is the inverse of the above conversion.
    F64 toAdvice(F64 val);
    F64 toAdvice(F64 val, U32 var);

    // Numerical values are constrained to the range [-1, 1]; make sure the given value
    // is in this range.
    F64 checkRange(F64 val);
}


// Counters maintained while parsing advice.
namespace Counters {
    extern U32 mLine;
}


// Agent characteristics.
namespace Agent {
    // Whether advice is used for evolved or scripted agent.
    enum Type {eEvolved, eScripted};
    extern Type mType;
}

#endif /* _OPENNERO_ADVICE_REP_H_ */
