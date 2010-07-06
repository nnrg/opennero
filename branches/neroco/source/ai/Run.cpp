#include "core/Common.h"
#include "Run.h"
#include "Task.h"
#include <vector>

namespace OpenNero
{
    using namespace std;
    
    SimpleRun::SimpleRun(EnvironmentPtr env, AgentSet agents, TaskPtr task)
        : _env(env)
        , _agents(agents)
        , _task(task)
    {
    
    }
    
    bool SimpleRun::step()
    {
        return false;
    }
    
    SequenceRun::SequenceRun()
    {
        // nothing here
    }
    
    void SequenceRun::append(RunPtr run)
    {
        _runs.push_back(run);
    }
    
    bool SequenceRun::step()
    {
        vector<RunPtr>::iterator iter;
        vector< RunPtr > to_remove;
        for (iter = _runs.begin(); iter != _runs.end(); ++iter)
        {
            if ((*iter)->step())
            {
                to_remove.push_back(*iter);
            }
        }
        for (iter = to_remove.begin(); iter != to_remove.end(); ++iter)
        {
            _runs.erase(find(_runs.begin(), _runs.end(), *iter));
        }
        return !_runs.empty();
    }
    
    RepeatRun::RepeatRun(RunPtr run, size_t number)
        : _run(run)
        , _number(number)
        , _current_run(0)
    {}
    
    bool RepeatRun::step()
    {
        if (_current_run >= _number)
        {
            return false;
        }
        else
        {
            ++_current_run;
            return _run->step();
        }
    }
}