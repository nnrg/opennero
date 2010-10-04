//--------------------------------------------------------
// OpenNero : Scheduler
//  An event driven scripting scheduler
//  October 9, 2007
//--------------------------------------------------------

#include "core/Common.h"
#include "core/ONTime.h"
#include "core/Algorithm.h"
#include "scripting/Scheduler.h"
#include "scripting/scriptIncludes.h"
#include "scripting/scripting.h"

namespace OpenNero
{
    /// functor for comparing two events based on time
    struct CompareEventExecTime
    {
    	/// @return true iff x is scheduled for earlier or identical time to that of y
        bool operator() ( const Scheduler::EventInfo& x, const Scheduler::EventInfo& y ) const
        {
            return x.mExecTime <= y.mExecTime;
        }
    };

    /// functor for finding an event by its id
    struct FindEventByEventId
    {
    	/// desired event ID
        const Scheduler::EventId mDesiredId;

        /// @param id event id to find
        FindEventByEventId( const Scheduler::EventId& id )
            : mDesiredId(id)
        {}

        /// perform the search operation
        /// @param x event information to look for
        bool operator() ( const Scheduler::EventInfo& x ) const
        {
            return mDesiredId == x.mEventId;
        }
    };



    Scheduler::EventInfo::EventInfo( const EventId& id, uint32_t execTime, const ScriptCommand& command ) :
    	mEventId(id),
    	mExecTime(execTime),
    	mCommand(command)
    {}



    Scheduler::EventId Scheduler::sEventId = 0;

    Scheduler::EventId Scheduler::ScheduleEvent( uint32_t timeOffsetMs, const ScriptCommand& command )
    {
        // calculate the execution time
        const uint32_t execTime = GetStaticTimer().getMilliseconds() + timeOffsetMs;

        // insert the info into the event list
        EventInfo info( sEventId++, execTime, command );
        mEvents.insert( std::lower_bound( mEvents.begin(), mEvents.end(), info, CompareEventExecTime() ), info );

        // make sure the events are sorted
        AssertMsg( is_sorted( mEvents.begin(), mEvents.end(), CompareEventExecTime() ), "Events are not sorted properly!" );

        return info.mEventId;
    }

    uint32_t Scheduler::RushEvents()
    {
        if( !mEvents.empty() )
            return RushEvents( mEvents.back().mExecTime );

        return 0;
    }

    uint32_t Scheduler::RushEvents( uint32_t endTime )
    {
        EventInfoList::iterator start = mEvents.begin();
        EventInfoList::iterator itr   = start;
        EventInfoList::iterator end   = mEvents.end();

        uint32_t c = 0;

        for( ; ( itr != end ) && ( itr->mExecTime <= endTime ); ++itr, ++c )
            ExecEvent(*itr);

        mEvents.erase( start, itr );

        // make sure the events are sorted
        AssertMsg( is_sorted( mEvents.begin(), mEvents.end(), CompareEventExecTime() ), "Events are not sorted properly!" );

        return c;
    }

    void Scheduler::ClearEvents()
    {
        mEvents.clear();
    }

    bool Scheduler::CancelEvent( const EventId& eventId )
    {
        EventInfoList::iterator itr = std::find_if( mEvents.begin(), mEvents.end(), FindEventByEventId(eventId) );
        if( itr == mEvents.end() )
            return false;

        mEvents.erase(itr);
        return true;
    }

    uint32_t Scheduler::ProcessEvents()
    {
        return RushEvents( GetStaticTimer().getMilliseconds() );
    }

    bool Scheduler::ExecEvent( const EventInfo& event )
    {
        ScriptingEngine& se = ScriptingEngine::instance();
        return se.Exec(event.mCommand);
    }

    namespace ScriptEventScheduling
    {
        Scheduler::EventId schedule( uint32_t timeOffset, const Scheduler::ScriptCommand& command )
        {
            Scheduler& scheduler = ScriptingEngine::instance().GetScheduler();
            return scheduler.ScheduleEvent( timeOffset, command );
        }

        bool cancel( const Scheduler::EventId& id )
        {
            Scheduler& scheduler = ScriptingEngine::instance().GetScheduler();
            return scheduler.CancelEvent(id);
        }
    }

    /// export scheduler methods into Python API
    PYTHON_BINDER( Scheduler )
    {
        def( "schedule", &ScriptEventScheduling::schedule, "Schedule an event to execute in some time offset. schedule(offset,command)");
        def( "cancel",   &ScriptEventScheduling::cancel,   "Cancel an event from executing. cancel( eventId )" );
    }
}
