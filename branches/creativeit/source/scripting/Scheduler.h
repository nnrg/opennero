//--------------------------------------------------------
// OpenNero : Scheduler
//  An event driven scripting scheduler
//  October 9, 2007
//--------------------------------------------------------

#ifndef _SCRIPTING_SCHEDULER_H_
#define _SCRIPTING_SCHEDULER_H_

#include <string>
#include <list>
#include "core/ONTypes.h"

namespace OpenNero
{
    /// The Scheduler is responsible for scheduling events to be executed
    /// at a later time.
    class Scheduler
    {
    public:

        typedef std::string ScriptCommand;  ///< A Script Command event
        typedef uint32_t EventId;           ///< An identifier for an event

    public:

        /// An invalid event handle
        static const EventId kInvalidEventId = 0xffffffff;

    public:

        /// Schedule an event at some time in the future
        /// @param timeOffsetMs the offset in time from now to execute the command in milliseconds
        /// @param command the script command to execute at the desired time
        /// @return an event id handle to track this execution
        EventId ScheduleEvent( uint32_t timeOffsetMs, const ScriptCommand& command );

        /// Run all Events up to a given time
        /// @param endTime the latest event to execute
        uint32_t RushEvents( uint32_t endTime );

        /// Run all events up to now
        uint32_t RushEvents();

        /// Clear all of the scheduled events
        void ClearEvents();

        /// Cancel a given event
        /// @param eventId the event id handle of the event to cancel
        /// @return true if the system found the event and canceled it
        bool CancelEvent( const EventId& eventId );

        /// Process all events up to the current time
        uint32_t ProcessEvents();

    private:

        friend struct CompareEventExecTime;
        friend struct FindEventByEventId;

        /// The information relevant to a single event
        struct EventInfo
        {
            EventInfo( const EventId& id, uint32_t execTime, const ScriptCommand& command );

            EventId             mEventId;       ///< The identifier for the event
            uint32_t            mExecTime;      ///< The time in real time when the event should execute
            ScriptCommand       mCommand;       ///< The command to execute when the time comes
        };

    private:

        typedef std::list<EventInfo> EventInfoList;

    private:

        /// Execute a given event
        /// @param event information needed to execute the event
        /// @return true if the script command did not fail
        bool ExecEvent( const EventInfo& event );

    private:

        /// An identifier generator
        static EventId sEventId;

    private:

        /// our pending events sorted by proximity in time, soonest first.
        EventInfoList           mEvents;
    };
}

#endif // _SCRIPTING_SCHEDULER_H_
