//--------------------------------------------------------
// OpenNero : Performance
//  various performance metrics
//--------------------------------------------------------

#include "core/Common.h"
#include "utils/Performance.h"
#include "core/Error.h"
#include "core/ONTime.h"
#include "boost/pool/detail/singleton.hpp"
#include <fstream>
#include "scripting/scriptIncludes.h"

#if NERO_ENABLE_UNIT_TESTS
    #include <boost/test/unit_test.hpp>
    #include <boost/test/floating_point_comparison.hpp>
#endif

namespace OpenNero
{
    /// a magic number stored in a minimun frame value
    const uint32_t kMagicNumber = 9999999;

    // set this to false to disable and event tracking
    bool PerformanceMetricManager::sbEnableEvents = true;

    /// singleton accessor
    PerformanceMetricManager& PerformanceMetricManager::instance()
    {
        return boost::details::pool::singleton_default<PerformanceMetricManager>::instance();
    }

    /// constructor
    PerformanceMetricManager::PerformanceMetricManager()
        : mFrameNum(0)
    {}

    /// Start timing for a given event, this can be recursive
    /// @param id the identifier for the event
    void PerformanceMetricManager::EnterEvent( const EventId& id )
    {
        if( sbEnableEvents )
        {
            PerformanceEvent& event = GetEvent(id);

            uint32_t curTime = GetCurrentTime();

            // update the event stats
            if( !event.mCalledThisFrame )
            {
                event.mFrameAmt++;
                event.mCalledThisFrame = 1;
            }

            event.mCallAmt++;
            event.mCallTimes.push_back(curTime);
        }
    }

    /// End timing for a given event, this can be recursive
    /// @param id the identifier for this event
    void PerformanceMetricManager::LeaveEvent( const EventId& id )
    {
        if( sbEnableEvents )
        {
            // get the current time as soon as possible
            uint32_t curTime  = GetCurrentTime();

            // update event stats
            Assert( ContainEvent(id) );
            PerformanceEvent& event = GetEvent(id);
            Assert( event.mCallTimes.size() > 0 );
            Assert( event.mCallTimes.back() <= curTime );

            uint32_t callTime = event.mCallTimes.back();
            event.mCallTimes.pop_back();

            event.mTotalTime += curTime - callTime;
            event.mFrameTime += curTime - callTime;
        }
    }

    /// Notify the manager that a new simulation frame has started
    /// clearing all per frame flags
    void PerformanceMetricManager::StartSimulationFrame()
    {
        if( sbEnableEvents )
        {
            // go through and clear all of the CalledThisFrame flags
            PerformanceEventMap::iterator itr = mEventMap.begin();
            for( ; itr != mEventMap.end(); ++itr )
            {
#if NERO_DEBUG
                PerformanceEvent& event = itr->second;
                Assert( event.mCallTimes.size() == 0 );
#endif
                itr->second.mCalledThisFrame = 0;
                itr->second.mFrameTime = 0;
            }
        }
    }

    /// Notify the event manager that a simulation frame has ended
    /// and gather frame stats on all active events
    void PerformanceMetricManager::EndSimulationFrame()
    {
        if( sbEnableEvents )
        {
            // Update all the perframe events
            PerformanceEventMap::iterator itr = mEventMap.begin();
            for( ; itr != mEventMap.end(); ++itr )
            {
                PerformanceEvent& event = itr->second;
                Assert( event.mCallTimes.size() == 0 );

                if( event.mCalledThisFrame )
                {
                    event.mFrames.push_back( PerformanceEvent::FrameTimePair(mFrameNum,event.mFrameTime) );

                    if( event.mFrameTime >  event.mMaxFrameTime )
                        event.mMaxFrameTime = event.mFrameTime;

                    else if( event.mFrameTime < event.mMinFrameTime )
                        event.mMinFrameTime = event.mFrameTime;
                }
            }

            // increase the frame number
            ++mFrameNum;
        }
    }

    /// Get the average number of times that this even this event is called per simulation frame
    /// @param id the event id
    /// @return the average number of times event is called per sim frame
    float32_t PerformanceMetricManager::GetCallsPerFrame( const EventId& id ) const
    {
        Assert( ContainEvent(id) );
        const PerformanceEvent& event = GetEvent(id);

        // if this has never been called within the construct of a sim frame, just give call amt
        if( event.mFrameAmt == 0 )
        {
            return (float32_t)event.mCallAmt;
        }

        // otherwise do the ratio of calls to frames
        else
        {
            return (float32_t)event.mCallAmt/event.mFrameAmt;
        }
    }

    /// Get the time spent inside of this event so far this frame
    /// @param id the id of the event
    /// @return the collective amount of time spent in this event this frame
    float32_t PerformanceMetricManager::GetTimeThisFrame( const EventId& id ) const
    {
        Assert( ContainEvent(id) );
        const PerformanceEvent& event = GetEvent(id);
        return (float32_t)event.mFrameTime;
    }

    /// Get the average time spent per frame in an event
    /// @param id the event identifier
    /// @param timeScale the unit we want the time returned in
    /// @return the time requested
    float32_t PerformanceMetricManager::GetTimePerFrame( const EventId& id, TimeScale timeScale ) const
    {
        Assert( ContainEvent(id) );
        PerformanceEvent& event = GetEvent(id);
        if( event.mFrameAmt == 0 )
            return 0;

        return GetScaledUnit( (float32_t)event.mTotalTime/event.mFrameAmt, timeScale );
    }

    /// Get the average time spent per frame in an event
    /// @param id the event identifier
    /// @param timeScale the unit we want the time returned in
    /// @return the time requested
    float32_t PerformanceMetricManager::GetTimePerCall( const EventId& id, TimeScale timeScale ) const
    {
        Assert( ContainEvent(id) );
        PerformanceEvent& event = GetEvent(id);
        if( event.mCallAmt == 0 )
            return 0;

        return GetScaledUnit( (float32_t)event.mTotalTime/event.mCallAmt, timeScale );
    }

    /// Get the total time spent in this event
    /// @param id the event identifier
    /// @param timeScale the unit we want the time returned in
    /// @return the time requested
    float32_t PerformanceMetricManager::GetTotalTime( const EventId& id, TimeScale timeScale ) const
    {
        Assert( ContainEvent(id) );
        PerformanceEvent& event = GetEvent(id);
        if( event.mCallAmt == 0 )
            return 0;

        return GetScaledUnit( (float32_t)event.mTotalTime, timeScale );
    }

    /// Get the maximum time spent in the event in any frame
    /// @param id the event identifier
    /// @param timeScale the unit we want the time returned in
    /// @return the time requested
    float32_t PerformanceMetricManager::GetMaxTime( const EventId& id, TimeScale timeScale ) const
    {
        Assert( ContainEvent(id) );
        PerformanceEvent& event = GetEvent(id);

        return GetScaledUnit( (float32_t)event.mMaxFrameTime, timeScale );
    }

    /// Get the minimum time spent in the event in any frame
    /// @param id the event identifier
    /// @param timeScale the unit we want the time returned in
    /// @return the time requested
    float32_t PerformanceMetricManager::GetMinTime( const EventId& id, TimeScale timeScale ) const
    {
        Assert( ContainEvent(id) );
        PerformanceEvent& event = GetEvent(id);

        return GetScaledUnit( (float32_t)event.mMinFrameTime, timeScale );
    }

    /// write a report to file about the events encountered
    /// @param reportFile the file to write to
    /// @param format the format to write in
    void PerformanceMetricManager::WriteReport( const std::string reportFile, OutputFormat format ) const
    {
        std::ofstream file(reportFile.c_str());
        if( file.is_open() )
        {
            WriteReport( file, format );
            file.close();
        }
    }

    /// Write a report to a stream about the events encountered
    /// @param stream the stream to write the report to
    /// @param format the format to write the stream in
    void PerformanceMetricManager::WriteReport( std::ostream& stream, OutputFormat format ) const
    {
        switch( format )
        {
        case kFormat_Report: WriteTextReport(stream); break;
        case kFormat_CSV:    WriteCSVReport(stream); break;
        default: Assert( false );
        }
    }


#if NERO_ENABLE_UNIT_TESTS

    /// A unit test for the PerformanceMetricManager
    void PerformanceMetricManager::UnitTest()
    {
#if NERO_ENABLE_PERFORMANCE_METRICS
        using namespace boost::unit_test;

        PerformanceMetricManager& pMM = PerformanceMetricManager::instance();

        NERO_PERF_EVENT_ENTER( unitTest );
        BOOST_CHECK_EQUAL( pMM.ContainEvent("unitTest"), true );

        // scoping tests
        {
            {
                NERO_PERF_EVENT_SCOPED( scopedTest );

                BOOST_CHECK_EQUAL( pMM.ContainEvent("scopedTest"), true );

                PerformanceEvent& event = pMM.GetEvent("scopedTest");
                BOOST_CHECK_EQUAL( event.mCallAmt, 1 );
                BOOST_CHECK_EQUAL( event.mCalledThisFrame, 1 );
                BOOST_CHECK_EQUAL( event.mEventId, EventId("scopedTest") );
                BOOST_CHECK_EQUAL( event.mFrameAmt, 1 );
                BOOST_CHECK_EQUAL( event.mFrameTime, 0 );
                BOOST_CHECK_EQUAL( event.mTotalTime, 0 );
                BOOST_CHECK_EQUAL( (uint32_t)event.mCallTimes.size(), 1 );

                {
                    NERO_PERF_EVENT_SCOPED( scopedTest );
                    BOOST_CHECK_EQUAL( event.mCallAmt, 2 );
                    BOOST_CHECK_EQUAL( event.mCalledThisFrame, 1 );
                    BOOST_CHECK_EQUAL( event.mEventId, EventId("scopedTest") );
                    BOOST_CHECK_EQUAL( event.mFrameAmt, 1 );
                    BOOST_CHECK_EQUAL( event.mFrameTime, 0 );
                    BOOST_CHECK_EQUAL( event.mTotalTime, 0 );
                    BOOST_CHECK_EQUAL( (uint32_t)event.mCallTimes.size(), 2 );
                }

                BOOST_CHECK_EQUAL( event.mCallAmt, 2 );
                BOOST_CHECK_EQUAL( event.mCalledThisFrame, 1 );
                BOOST_CHECK_EQUAL( event.mEventId, EventId("scopedTest") );
                BOOST_CHECK_EQUAL( event.mFrameAmt, 1 );
                Assert( event.mFrameTime >= 0 );
                Assert( event.mTotalTime >= 0 );
                BOOST_CHECK_EQUAL( (uint32_t)event.mCallTimes.size(), 1 );

                for( uint32_t i = 0; i < 10000; ++i )
                    i += 3;
            }

            BOOST_CHECK_EQUAL( pMM.ContainEvent("scopedTest"), true );

            PerformanceEvent& event = pMM.GetEvent("scopedTest");
            BOOST_CHECK_EQUAL( event.mCallAmt, 2 );
            BOOST_CHECK_EQUAL( event.mCalledThisFrame, 1 );
            BOOST_CHECK_EQUAL( event.mEventId, EventId("scopedTest") );
            BOOST_CHECK_EQUAL( event.mFrameAmt, 1 );
            Assert( event.mFrameTime >= 0 );
            Assert( event.mTotalTime >= 0 );
            BOOST_CHECK_EQUAL( (uint32_t)event.mCallTimes.size(), 0 );
        }

        // frame based tests
        {
            BOOST_CHECK_EQUAL( pMM.ContainEvent("frameA"), false );
            BOOST_CHECK_EQUAL( pMM.ContainEvent("frameB"), false );

            for( uint32_t i = 0; i < 100; ++i )
            {
                NERO_PERF_BEGIN_SIMFRAME();

                if( i % 2 == 0 )
                {
                    NERO_PERF_EVENT_SCOPED( frameA );
                    BOOST_CHECK_EQUAL( pMM.ContainEvent("frameA"), true );
                    for( uint32_t j = 0; j < 10000; j++ );
                }

                else
                {
                    NERO_PERF_EVENT_SCOPED( frameB );
                    BOOST_CHECK_EQUAL( pMM.ContainEvent("frameB"), true );
                    for( uint32_t j = 0; j < 10000; j++ );
                }

                NERO_PERF_END_SIMFRAME();

                if( i % 2 == 0 )
                {
                    BOOST_CHECK_EQUAL( pMM.ContainEvent("frameA"), true );
                    BOOST_CHECK_EQUAL( pMM.GetEvent("frameA").mFrameAmt, i/2+1 );
                    BOOST_CHECK_EQUAL( (uint32_t)pMM.GetEvent("frameA").mFrames.size(), i/2+1 );
                }

                else
                {
                    BOOST_CHECK_EQUAL( pMM.ContainEvent("frameB"), true );
                    BOOST_CHECK_EQUAL( pMM.GetEvent("frameB").mFrameAmt, i/2+1 );
                    BOOST_CHECK_EQUAL( (uint32_t)pMM.GetEvent("frameB").mFrames.size(), i/2+1 );
                }
            }
        }

        // time scale conversion tests
        {
            float millis = 60000.0f;
            BOOST_CHECK_CLOSE( pMM.GetScaledUnit( millis, kTimeScale_Milliseconds ), 60000.0f, 0.1f );
            BOOST_CHECK_CLOSE( pMM.GetScaledUnit( millis, kTimeScale_Seconds ), 60.0f, 0.1f );
            BOOST_CHECK_CLOSE( pMM.GetScaledUnit( millis, kTimeScale_Minutes ), 1.0f, 0.1f );
            BOOST_CHECK_CLOSE( pMM.GetScaledUnit( millis, kTimeScale_Hours ), 0.016666666666666666666666666666667f, 0.1f );
            BOOST_CHECK_CLOSE( pMM.GetScaledUnit( millis, kTimeScale_Days ), 6.9444444444444444444444444444444e-4f, 0.1f );
        }

        NERO_PERF_EVENT_LEAVE( unitTest );
        BOOST_CHECK_EQUAL( pMM.ContainEvent("unitTest"), true );
        BOOST_CHECK_EQUAL( (uint32_t)pMM.GetEvent("unitTest").mCallTimes.size(), 0 );
#endif
    }
#endif

    // ctor for a performance event
    PerformanceMetricManager::PerformanceEvent::PerformanceEvent()
        : mCallAmt(0)
        , mFrameAmt(0)
        , mTotalTime(0)
        , mFrameTime(0)
        , mMinFrameTime(kMagicNumber)
        , mMaxFrameTime(0)
        , mCalledThisFrame(0)
    {}

    // Get the current time according to our clock
    uint32_t PerformanceMetricManager::GetCurrentTime() const
    {
        return GetStaticTimer().getMilliseconds();
    }

    /// Check if we have ever encountered this event before
    /// @param id the identifier for the event
    /// @return true if we have encountered the event before
    bool PerformanceMetricManager::ContainEvent( const EventId& id ) const
    {
        PerformanceEventMap::const_iterator itr = mEventMap.find(id);
        return itr != mEventMap.end();
    }

    /// Get the event associated with the id, create one if it doesnt exist
    /// @param the identifier to request
    /// @param a reference to the requested event
    PerformanceMetricManager::PerformanceEvent& PerformanceMetricManager::GetEvent( const EventId& id ) const
    {
        PerformanceMetricManager* pThis = const_cast<PerformanceMetricManager*>(this);

        PerformanceEventMap::iterator itr = pThis->mEventMap.find(id);
        if( itr == pThis->mEventMap.end() )
        {
            PerformanceEvent e;
            e.mEventId = id;

            pThis->mEventMap[id] = e;

            return pThis->mEventMap[id];
        }

        return itr->second;
    }

    /// Scale an inputed unit by the given scale and return it
    /// @param timeInMilliseconds the time...in millseconds
    /// @paramm timeScale the desired time scale
    /// @return  the input time put in the request time scale
    float32_t PerformanceMetricManager::GetScaledUnit( float32_t timeInMilliseconds, TimeScale timeScale ) const
    {
        // conversion constants
        const float32_t toMillisConversion          = 1.0f;
        const float32_t toSecondsConversion         = toMillisConversion/1000.0f;
        const float32_t toMinutesConversion         = toSecondsConversion/60.0f;
        const float32_t toHoursConversion           = toMinutesConversion/60.0f;
        const float32_t toDaysConversion            = toHoursConversion/24.0f;

        switch( timeScale )
        {
        case kTimeScale_Milliseconds:   return timeInMilliseconds * toMillisConversion;
        case kTimeScale_Seconds:        return timeInMilliseconds * toSecondsConversion;
        case kTimeScale_Minutes:        return timeInMilliseconds * toMinutesConversion;
        case kTimeScale_Hours:          return timeInMilliseconds * toHoursConversion;
        case kTimeScale_Days:           return timeInMilliseconds * toDaysConversion;
        default: Assert(false);
        }

        return 0;
    }

    /// a magic separator this splits events into one-shot and per-frame events
    const uint32_t kMagicSeparator = 10;

    /// Write out a report in text format
    /// @param stream the stream to write to
    void PerformanceMetricManager::WriteTextReport( std::ostream& stream ) const
    {
        stream << "----Performance Text Report----\n";
        stream << "Number of frames tracked: " << mFrameNum << "\n\n";

        // write all the event tags we have
        stream << "----Performance Events:----\n";

        PerformanceEventMap::const_iterator citr = mEventMap.begin();
        PerformanceEventMap::const_iterator cend = mEventMap.end();

        for( ; citr != cend; ++citr )
        {
            stream << citr->second.mEventId << "\n";
        }

        stream << "\n";

        // write stats per event
        stream << "----Event Statistics:----\n\n";

        citr = mEventMap.begin();
        for( ; citr != cend; ++citr )
        {
            const PerformanceEvent& event = citr->second;
            stream << "Event Id: " << event.mEventId << "\n";

            if( event.mCallAmt > 0 )
            {
                stream << "Called " << event.mCallAmt << " time(s).\n";
                stream << "Called from " << event.mFrameAmt << " unique frame(s).\n";

                // kMagicSeparator is an abitrary number that determines whether
                // this is a one-shot thing or a per frame thing.
                if( event.mFrameAmt > kMagicSeparator )
                {
                    stream << "Average calls per frame: " << (float32_t)event.mCallAmt/event.mFrameAmt << "\n";
                }

                stream << "Average execution time per call: " << (float32_t)event.mTotalTime/event.mCallAmt << " ms\n";

                if( event.mFrameAmt > 0 && event.mMinFrameTime != kMagicNumber )
                {
                    stream << "Its per frame contribution had a minimum of " << event.mMinFrameTime << " ms and a max of " << event.mMaxFrameTime << " ms.\n";
                }
            }
            else
            {
                stream << "Never called during the performance measuring period.\n";
            }

            stream << "\n\n";
        }
    }

    /// Write the report in a spreadsheet friendly format
    ///@param stream the stream to write the report to
    void PerformanceMetricManager::WriteCSVReport( std::ostream& stream ) const
    {
        PerformanceEventMap::const_iterator citr = mEventMap.begin();
        PerformanceEventMap::const_iterator cend = mEventMap.end();

        // --- write all the one shot events ---
        stream << "One-Shot Events:\n";

        stream << "event id, call amt, avg ms per call, \n";
        for( ; citr != cend; ++citr )
        {
            if( citr->second.mFrameAmt <= kMagicSeparator )
            {
                const PerformanceEvent& event = citr->second;

                float32_t avgMsPerCall  = (event.mCallAmt > 0)?  (float32_t)event.mTotalTime/event.mCallAmt  : 0;
                stream << event.mEventId << "," << event.mCallAmt << "," << avgMsPerCall << "\n";
            }
        }

        // --- write all the per-frame events ---
        stream << "\n\n";
        stream << "Per-Frame Events:\n";
        stream << "event id, call amt, frame amt, calls per frame, avg ms per call, avg ms per frame, min frame ms, max frame ms\n";

        citr = mEventMap.begin();

        for( ; citr != cend; ++citr )
        {
            const PerformanceEvent& event = citr->second;

            if( citr->second.mFrameAmt > kMagicSeparator )
            {
                float32_t avgMsPerCall  = (event.mCallAmt > 0)?  (float32_t)event.mTotalTime/event.mCallAmt  : 0;
                float32_t avgMsPerFrame = (event.mFrameAmt > 0)? (float32_t)event.mTotalTime/event.mFrameAmt : 0;

                stream << event.mEventId << "," << event.mCallAmt << "," << event.mFrameAmt << "," << (float32_t)event.mCallAmt/event.mFrameAmt << "," << avgMsPerCall << "," << avgMsPerFrame << "," << event.mMinFrameTime << "," << event.mMaxFrameTime << "\n";
            }
        }

        stream << "\n\n";

        // --- write out frame history ---
        citr = mEventMap.begin();

        for( ; citr != cend; ++citr )
        {
            const PerformanceEvent& event = citr->second;

            if( event.mFrameAmt > kMagicSeparator )
            {
                PerformanceEvent::FrameList::const_iterator cfItr = event.mFrames.begin();
                PerformanceEvent::FrameList::const_iterator cfEnd = event.mFrames.end();

                stream << event.mEventId << "\n";
                stream << "Frame #,";

                for( ; cfItr != cfEnd; ++cfItr )
                {
                    stream << cfItr->first << ",";
                }

                stream << "\n";
                stream << "Time (ms),";

                cfItr = event.mFrames.begin();

                for( ; cfItr != cfEnd; ++cfItr )
                {
                    stream << cfItr->second << ",";
                }

                stream << "\n\n";
            }
        }
    }



    /// ctor for the scoped event - start the event
    ScopedPerformanceMetricEvent::ScopedPerformanceMetricEvent( const PerformanceMetricManager::EventId& id )
        : mEventId(id)
    {
        if( PerformanceMetricManager::sbEnableEvents )
        {
            PerformanceMetricManager& pMM = PerformanceMetricManager::instance();
            pMM.EnterEvent(mEventId);
        }
    }

    /// dtor for the scoped event - end the event
    ScopedPerformanceMetricEvent::~ScopedPerformanceMetricEvent()
    {
        if( PerformanceMetricManager::sbEnableEvents )
        {
            PerformanceMetricManager& pMM = PerformanceMetricManager::instance();
            pMM.LeaveEvent(mEventId);
        }
    }

    /// python interface methods
    /// @{

    /// initiate performance scope
    void PyEnterPerfEvent( const std::string& eventId )
    {
    #if NERO_ENABLE_PERFORMANCE_METRICS
        if( PerformanceMetricManager::sbEnableEvents )
        {
            PerformanceMetricManager::instance().EnterEvent(eventId);
        }
    #endif
    }

    /// leave performance scope
    void PyLeavePerfEvent( const std::string& eventId )
    {
    #if NERO_ENABLE_PERFORMANCE_METRICS
        if( PerformanceMetricManager::sbEnableEvents )
        {
            PerformanceMetricManager::instance().LeaveEvent(eventId);
        }
    #endif
    }

    /// export performance methods to Python
    PYTHON_BINDER(Performance)
    {
        // export manual timing methods
        def("PyEnterPerfEvent", PyEnterPerfEvent);
        def("PyLeavePerfEvent", PyLeavePerfEvent);

        // to use the scoped event you must first of all be in a scope (method)
        // and you must catch a reference to the scoped event
        // ex: s = PyPerfEventScoped('PyTest') will work because s holds it until the method is complete
        // ex: PyPerfEventScoped('PyTest') will _not_ work because nothing holds the event and it is automatically destroyed

        // export a scope event object
        class_<ScopedPerformanceMetricEvent>("PyPerfEventScoped", "A performance event timed by scope rules", init<std::string>() );
    }
    ///@}
}
