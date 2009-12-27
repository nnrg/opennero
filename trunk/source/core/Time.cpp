//--------------------------------------------------------
// OpenNero : Time
//  time operations
//--------------------------------------------------------

#include "core/Common.h"
#include "core/ONTime.h"
#include "core/TimeImpl.h"

namespace OpenNero 
{      
    TimerPtr GetTimer()
    {
        TimerPtr result(new BoostTimer());
        return result;
    }

    /// @return a static timer created only one
    Timer& GetStaticTimer()
    {
        static BoostTimer sTimer;
        return sTimer;
    }

    Timer::~Timer() {}        

}//end OpenNero
