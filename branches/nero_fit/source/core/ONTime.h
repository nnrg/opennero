//--------------------------------------------------------
// OpenNero : Time
//  time operations
//--------------------------------------------------------

#ifndef _CORE_TIME_H_
#define _CORE_TIME_H_

#include "core/BoostCommon.h"
#include "core/ONTypes.h"
#include <iostream>

namespace OpenNero 
{   
    typedef float64_t TimeType; ///< Time type
    
    /// @cond
    BOOST_SHARED_STRUCT(Timer);
    /// @endcond
    
    /// microsecond-resolution wall clock timer
    struct Timer
    {
        virtual ~Timer() = 0;

        /// reset timer to 0
        /// @return time since last reset/constructor
        virtual uint64_t resetMicroseconds() = 0;

        /// @return time since last reset/constructor in microseconds
        virtual uint64_t getMicroseconds() = 0;

        /// @return time since last reset/constructor in milliseconds
        virtual uint64_t getMilliseconds() = 0;
        
        /// print timestamp to output stream
        virtual void stamp(std::ostream& out) = 0;
    };
        
    /// @return a new timer instance
    TimerPtr GetTimer();

    /// @return a static timer created only one
    Timer& GetStaticTimer();    

}//end OpenNero

#endif // _CORE_TIME_H_
