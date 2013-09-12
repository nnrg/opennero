#include "core/Common.h"
#include "ai/PythonAI.h"
#include "ai/AI.h"
#include "ai/AIObject.h"
#include "ai/AIManager.h"
#include "ai/Environment.h"
#include "ai/AgentBrain.h"
#include "core/IrrUtil.h"
#include "game/SimEntityData.h"
#include "math/Random.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{
    using namespace boost;

    /// create a new AI object to live in the specified world
    PythonAIObject::PythonAIObject(EnvironmentPtr world, SimEntityPtr parent) 
        : AIObject(world, parent)
    {
    }

    PythonAIObject::~PythonAIObject()
    {
        // nada
    }

    /// load this AI object from template
    /// assumes that the world is already initialized when this is called
    bool PythonAIObject::LoadFromTemplate(ObjectTemplatePtr objTemplate, const SimEntityData& data)
    {
        ScriptingEngine& script = ScriptingEngine::instance();
        PythonAITemplatePtr pythonTemplate = 
            static_pointer_cast<PythonAITemplate, ObjectTemplate>(objTemplate);
        Assert(pythonTemplate);
        std::string py_agent_expression = pythonTemplate->getBrainExpr();
        // TODO: this evaluation line does not work within the context of the module path
        AgentBrainPtr brain;
        script.Eval<AgentBrainPtr>(py_agent_expression, brain);
        AssertMsg(brain, "Could not load Python agent " << py_agent_expression);
        brain->name = pythonTemplate->getBrainExpr();
        setBrain(brain);
        if (!brain) {
            LOG_F_WARNING("ai", "Could not load an agent brain");
            return false;
        } else {
            brain->SetBody(shared_from_this());
            setInitInfo(getWorld()->get_agent_info(brain));
            brain->initialize(getInitInfo());
            return true;
        }
    }
    
    PythonAITemplate::PythonAITemplate(SimFactoryPtr factory, const PropertyMap& propMap, const std::string& prefix)
        : AIObjectTemplate(factory, propMap), mBrainExpr(), mEnvironmentExpr()
    {
        PropertyMap::AttributeMap attribs = propMap.getAttributes(prefix);
        mBrainExpr = attribs["agent"];
        LOG_F_DEBUG("ai", "Loading python agent: " << mBrainExpr);
    }

    /// @param env Environment to connect
    /// @param ent SimEntity to connect
    /// @return an instance of the appropriate AIObject
    AIObjectPtr PythonAITemplate::CreateObject(EnvironmentPtr env, SimEntityPtr ent)
    {
        AIObjectPtr p(new PythonAIObject(env, ent));
        return p;
    }

    inline std::ostream& operator<<(std::ostream& out, AIObject& obj)
    {
        return obj.stream(out);
    }

    /// display this AI object as a string
    std::ostream& PythonAIObject::stream(std::ostream& out) const
    {
        out << "<PythonAIObject />";
        return out;
    }
}
