//--------------------------------------------------------
// OpenNero : Scheduler
//  An event driven scripting scheduler
//  October 9, 2007
//--------------------------------------------------------

#include "core/Common.h"
#include "core/ONTime.h"
#include "core/Algorithm.h"
#include "scripting/Scheduler.h"
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
}
