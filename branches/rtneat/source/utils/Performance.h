//--------------------------------------------------------
// OpenNero : Performance
//  various performance metrics
//--------------------------------------------------------

#ifndef _OPENNERO_UTIL_PERFORMANCE_SCRIPTING_H_
#define _OPENNERO_UTIL_PERFORMANCE_SCRIPTING_H_

#include "core/ONTypes.h"
#include "core/Preprocessor.h"
#include "core/HashMap.h"
#include <ostream>
#include <utility>
#include <vector>
#include <list>

// the targets we want metrics to be in
//#define NERO_ENABLE_PERFORMANCE_METRICS (NERO_DEBUG || NERO_RELEASE)
#ifndef NERO_ENABLE_PERFORMANCE_METRICS
#define NERO_ENABLE_PERFORMANCE_METRICS 0
#endif

namespace OpenNero
{
    /// The PerformanceMetricManager is responsible for storing and containing
    /// all performance related timing events. The user should go through this
    /// singleton to do all performancde related timings.
    class PerformanceMetricManager
    {
    public:

        /// set this variable to false if you do not want to track any events
        static bool sbEnableEvents;

    public:

        /// an identifier for a PerformanceEvent
        typedef std::string EventId;

        /// A unit scale of time
        enum TimeScale
        {
            // ways to report the time distance
            kTimeScale_Milliseconds,
            kTimeScale_Seconds,
            kTimeScale_Minutes,
            kTimeScale_Hours,
            kTimeScale_Days
        };

        /// The output format for a log
        enum OutputFormat
        {
            kFormat_Report,     ///< A Textual Report
            kFormat_CSV,        ///< Comma Seperated Values
        };

    public:

        // Singleton access
        static PerformanceMetricManager& instance();

    public:

        PerformanceMetricManager();

        // controls when the event timings occur
        void EnterEvent( const EventId& id );
        void LeaveEvent( const EventId& id );

        // used to control when the metric clears per frame flags.
        // should be called at the beginning and end of a simulation loop
        void StartSimulationFrame();
        void EndSimulationFrame();

        // get various stats on a given event
        float32_t GetCallsPerFrame( const EventId& id ) const;
        float32_t GetTimeThisFrame( const EventId& id ) const;
        float32_t GetTimePerFrame( const EventId& id, TimeScale timeScale ) const;
        float32_t GetTimePerCall( const EventId& id, TimeScale timeScale ) const;
        float32_t GetTotalTime( const EventId& id, TimeScale timeScale ) const;
        float32_t GetMaxTime( const EventId& id, TimeScale timeScale ) const;
        float32_t GetMinTime( const EventId& id, TimeScale timeScale ) const;

        // output a report in a given format
        void WriteReport( const std::string reportFile, OutputFormat format ) const;
        void WriteReport( std::ostream& stream, OutputFormat format ) const;

    public:

    #if NERO_ENABLE_UNIT_TESTS
        static void UnitTest();
    #endif

    private:

        /// A structure for storing information about a given performance event.
        /// This structure stores all of the stats relating to time spent inside
        /// a events laid out by users
        struct PerformanceEvent
        {
            typedef std::pair<uint32_t,uint32_t> FrameTimePair; ///< pair holding the frame number and time for that frame
            typedef std::vector<FrameTimePair> FrameList;       ///< vector holding out frame pairs
            typedef std::list<uint32_t> CallTimes;              ///< Times we have been called (recursively if possible)

            EventId                 mEventId;           ///< identifier for the event

            uint32_t                mCallAmt;           ///< The number of times this event has been called
            uint32_t                mFrameAmt;          ///< The number of frames this event has been called in
            uint32_t                mTotalTime;         ///< The total time allocated to this event
            uint32_t                mFrameTime;         ///< Time spent in this event during this frame
            uint32_t                mMinFrameTime;      ///< The minimum event time of all frames
            uint32_t                mMaxFrameTime;      ///< The maximum event time of all frames

            uint8_t                 mCalledThisFrame;   ///< Has this event been called this frame?

            FrameList               mFrames;            ///< A collection of all frame history

            CallTimes               mCallTimes;         ///< A stack of calls to this event

            PerformanceEvent();
        };

        ///< A map of identifiers to events
        typedef hash_map< EventId, PerformanceEvent > PerformanceEventMap;

        PerformanceEventMap         mEventMap;  ///< Maps all used events
        uint32_t                    mFrameNum;  ///< The current metric frame number

    private:

        // get the current time in milliseconds
        uint32_t GetCurrentTime() const;

        // does the manager contain this event?
        bool ContainEvent( const EventId& id ) const;

        // get the event associated with the id, creating a new one if it is not present
        PerformanceEvent& GetEvent( const EventId& id ) const;

        // convert an input amount of time into the appropriate time scale
        float32_t GetScaledUnit( float32_t timeInMilliseconds, TimeScale timeScale ) const;

        // write a performance report in multiple formats
        void WriteTextReport( std::ostream& stream ) const;
        void WriteCSVReport( std::ostream& stream ) const;
    };

    /// This class is a utility for the scoped performance events
    /// that start upon construction and end upon destruction
    class ScopedPerformanceMetricEvent
    {
    public:

        ScopedPerformanceMetricEvent( const PerformanceMetricManager::EventId& id );
        ~ScopedPerformanceMetricEvent();

    private:

        PerformanceMetricManager::EventId mEventId;
    };

// if we have performance metrics enabled define these macros, otherwise make them blank
#if NERO_ENABLE_PERFORMANCE_METRICS

#define NERO_PERF_EVENT_ENTER( eventId ) OpenNero::PerformanceMetricManager::instance().EnterEvent(#eventId)
#define NERO_PERF_EVENT_LEAVE( eventId ) OpenNero::PerformanceMetricManager::instance().LeaveEvent(#eventId)
#define NERO_PERF_BEGIN_SIMFRAME()       OpenNero::PerformanceMetricManager::instance().StartSimulationFrame()
#define NERO_PERF_END_SIMFRAME()         OpenNero::PerformanceMetricManager::instance().EndSimulationFrame()

#define NERO_PERF_EVENT_SCOPED( eventId ) OpenNero::ScopedPerformanceMetricEvent __perfInst_##eventId(#eventId)

#else

#define NERO_PERF_EVENT_ENTER( eventId )
#define NERO_PERF_EVENT_LEAVE( eventId )
#define NERO_PERF_BEGIN_SIMFRAME()
#define NERO_PERF_END_SIMFRAME()

#define NERO_PERF_EVENT_SCOPED( eventId )

#endif

// python performance event connection methods
void PyEnterPerfEvent( const std::string& eventId );
void PyLeavePerfEvent( const std::string& eventId );

};

#endif // _OPENNERO_UTIL_PERFORMANCE_SCRIPTING_H_
