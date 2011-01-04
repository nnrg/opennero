#ifndef TIMEIMPL_H_
#define TIMEIMPL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <string>
#include "core/ONTime.h"

namespace OpenNero
{   
    using namespace boost::posix_time;

    /// a Timer implementation that uses the Boost.Time microsecond clock
    class BoostTimer : public Timer
    {
        private:
            ptime mStartingTime; ///< starting time
        public:
            BoostTimer()
                : mStartingTime(microsec_clock::local_time())
            {
                // nothing
            }
            
            ~BoostTimer()
            {
                // nothing                    
            }

            /// reset timer to 0
            /// @return time since last reset/constructor
            uint64_t resetMicroseconds()
            {
                uint64_t answer = getMicroseconds();
                mStartingTime = microsec_clock::local_time();
                return answer;
            }

            /// @return time since last reset/constructor
            uint64_t getMicroseconds()
            {
                return (uint64_t)(microsec_clock::local_time() - mStartingTime).total_microseconds();
            }

            /// @return time since last reset/constructor in milliseconds
            uint64_t getMilliseconds()
            {
                return getMicroseconds()/1000;
            }
            
            /// print time stamp to output stream
            void stamp(std::ostream& out)
            {
                ptime pt(microsec_clock::local_time());
                out << to_simple_string(pt);
            }
    };
}

#endif /*TIMEIMPL_H_*/
