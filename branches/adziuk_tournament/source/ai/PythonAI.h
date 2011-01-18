#ifndef _OPENNERO_AI_PYTHONAI_H_
#define _OPENNERO_AI_PYTHONAI_H_

#include <string>
#include "ai/AIObject.h"
#include "ai/AI.h"
#include "ai/Environment.h"
#include "ai/AIObject.h"
#include "core/ONTime.h"

namespace OpenNero
{
	/// @cond
    BOOST_SHARED_DECL(AgentBrain);
    BOOST_SHARED_DECL(PythonAITemplate);
    BOOST_SHARED_DECL(PythonAIObject);
    /// @endcond

    /**
    * PythonAIObject is where an AgentBrain meets the Environment
    */
    class PythonAIObject : public AIObject
    {
    public:
        /// create a PythonAIObject for the specified world
        /// @param world the Environment to use for this Python object
        /// @param parent SimEntity that owns this AIObject
        PythonAIObject(EnvironmentPtr world, SimEntityPtr parent);
        virtual ~PythonAIObject();

        /// load this AI object from template
        virtual bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data );

        /// display this AI object as a string
        virtual std::ostream& stream(std::ostream& out) const;
    };

    /// template for creating a Python AI object from an XML file
    class PythonAITemplate : public AIObjectTemplate
    {
    public:
        /// constructor when given a factory, propMap, and a prefix
        PythonAITemplate(SimFactoryPtr factory, const PropertyMap& propMap, const std::string& prefix);
        virtual ~PythonAITemplate() {}

        static const std::string TemplateType()
        {
            return "Template.AI.Python";
        }

        /// get the name of the Python class to load for the brain
        std::string getBrainExpr() const { return mBrainExpr; }

        /// get the name of the Python class to load for the env.
        std::string getEnvironmentExpr() const { return mEnvironmentExpr; }

        /// create an instance of the appropriate AIObject
        virtual AIObjectPtr CreateObject(EnvironmentPtr env, SimEntityPtr ent);
    private:
        std::string mBrainExpr; ///< name of the Python class to load for the brain
        std::string mEnvironmentExpr; ///< name of the Python class to load for the environment (if any)
    };

}

#endif /* PYTHONAI_H_ */
