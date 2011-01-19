//---------------------------------------------------
// Name: OpenNero : Log
// Desc:  logs information
//---------------------------------------------------

#ifndef _OPEN_NERO_LOG_H_
#define _OPEN_NERO_LOG_H_

#include "core/Common.h"
#include "core/Preprocessor.h"
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <string>

// only possibly turn off logs in the final release
#define NERO_ENABLE_LOGS (!NERO_FINAL_RELEASE)

namespace OpenNero
{
    namespace Log
    {
        /**
         * An abstraction log connection. The ILogConnection serve as the "destinations"
         * for the log routing system. Message get sent to the central hub and the log system
         * then dispatches the messages to the ILogConnections which store the message in some fasion
        */
        class ILogConnection
        {
        public:

            virtual ~ILogConnection()
            {}

	        /// send out a low priority message
	        virtual void LogDebug( const char* msg )    = 0;

            /// send out a normal priority message
            virtual void LogMsg( const char* msg )		= 0;

            /// send out a medium priority message - something has gone wrong, but we can continue
            virtual void LogWarning( const char* msg )	= 0;

            /// send out a HIGH priority message - something has gone wrong and we should stop
            virtual void LogError( const char* msg )	= 0;

            /// get an identifying name for this connection
            virtual const std::string getConnectionName() const = 0;
        };
        
        typedef boost::shared_ptr<ILogConnection> ILogConnectionPtr;
        typedef std::vector<ILogConnectionPtr> ILogConnectionVector;
        typedef std::vector< std::string > FilterList;        

        /// add a log connection to our system
        extern void AddLogConnection( ILogConnectionPtr conn );

        /// remove a log connection from our system
        extern void RemoveLogConnection( ILogConnectionPtr conn );

        /// send out appropriate level messages
        extern void LogDebug( const char* type, const char* connectionName, const char* msg );
        extern void LogMsg( const char* type,  const char* connectionName, const char* msg );
        extern void LogWarning( const char* type,  const char* connectionName, const char* msg );
        extern void LogError( const char* type,  const char* connectionName, const char* msg );

        /// start up and shut down the logging system
        extern void LogSystemInit( const std::string& logFileName );
        extern void LogSystemSpecifyFilters( const FilterList& flist );
        extern void LogSystemShutdown();

        /// Allow ALL messages of any type to come through the logger
        extern const std::string kLogFilterAcceptAll;

    } // end Log

// some semi-ugly defines

#if NERO_ENABLE_LOGS

    // default broadcasting logger macros - sends the message to every log connection regardless of target or type
    #define LOG_DEBUG_EVERY(t, msg) do { static int counter = 0; if (++counter % t == 0) { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (D) " << msg; OpenNero::Log::LogDebug( NULL, NULL, cmsg.str().c_str() ); } } while (0)
    #define LOG_DEBUG( msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (D) " << msg; OpenNero::Log::LogDebug  ( NULL, NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_ERROR( msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << msg; OpenNero::Log::LogError  ( NULL, NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_MSG( msg )     do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (M) " << msg; OpenNero::Log::LogMsg    ( NULL, NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_WARNING( msg ) do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (*) " << msg; OpenNero::Log::LogWarning( NULL, NULL, cmsg.str().c_str() ); } while(0)

    // destination based logs - send the message to a certain connection
    // Example: LOG_D_DEBUG( "texture_loader_connection", "A texture load as failed" )
    #define LOG_D_DEBUG( name, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << msg; OpenNero::Log::LogDebug  ( NULL, (name), cmsg.str().c_str() ); } while(0)
    #define LOG_D_ERROR( name, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << msg; OpenNero::Log::LogError  ( NULL, (name), cmsg.str().c_str() ); } while(0)
    #define LOG_D_MSG( name, msg )     do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (M) " << msg; OpenNero::Log::LogMsg    ( NULL, (name), cmsg.str().c_str() ); } while(0)
    #define LOG_D_WARNING( name, msg ) do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (*) " << msg; OpenNero::Log::LogWarning( NULL, (name), cmsg.str().c_str() ); } while(0)

    // filter based logs - send the message only to receivers of certain types
    
    #define LOG_FILTER_TOKEN(type) "[" << (type) << "] "
    
    // The basis behind this is that a user or developer should be able to filter out the log messages that are relevant.
    // If I am only concerned with messages from a given system, don't show me messages from these other ones.
    // Example: LOG_F_MSG( "render", "Rendering a complex object" )
    //   This message only gets sent to developers that have the "render" type enabled in their filter list
    //   as setup by the LogSystemSpecifyFilters method

    #define LOG_F_DEBUG( type, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogDebug(   (type), NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_F_ERROR( type, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogError(   (type), NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_F_MSG( type, msg )     do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (M) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogMsg(     (type), NULL, cmsg.str().c_str() ); } while(0)
    #define LOG_F_WARNING( type, msg ) do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (*) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogWarning( (type), NULL, cmsg.str().c_str() ); } while(0)

    // filter and destination based logs - send the message only to the given connection only to receivers of certain types
    #define LOG_FD_DEBUG( type, target, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogDebug(   (type), (target), cmsg.str().c_str() ); } while(0)
    #define LOG_FD_ERROR( type, target, msg )   do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (!) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogError(   (type), (target), cmsg.str().c_str() ); } while(0)
    #define LOG_FD_MSG( type, target, msg )     do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (M) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogMsg(     (type), (target), cmsg.str().c_str() ); } while(0)
    #define LOG_FD_WARNING( type, target, msg ) do { std::stringstream cmsg; GetStaticTimer().stamp(cmsg); cmsg << " (*) " << LOG_FILTER_TOKEN(type) << msg; OpenNero::Log::LogWarning( (type), (target), cmsg.str().c_str() ); } while(0)

#else

    // remove code from modules
    #define LOG_DEBUG_EVERY(t, msg)
    #define LOG_DEBUG( msg )
    #define LOG_ERROR( msg )
    #define LOG_MSG( msg )
    #define LOG_WARNING( msg )
    #define LOG_D_DEBUG( name, msg )
    #define LOG_D_ERROR( name, msg )
    #define LOG_D_MSG( name, msg )
    #define LOG_D_WARNING( name, msg )
    #define LOG_F_DEBUG( type, msg )
    #define LOG_F_ERROR( type, msg )
    #define LOG_F_MSG( type, msg )
    #define LOG_F_WARNING( type, msg )
    #define LOG_FD_DEBUG( type, target, msg )
    #define LOG_FD_ERROR( type, target, msg )
    #define LOG_FD_MSG( type, target, msg )
    #define LOG_FD_WARNING( type, target, msg )

#endif // NERO_ENABLE_LOGS

// utility macros...
#define LOG_DF_DEBUG( type, target, msg )   LOG_FD_DEBUG( type, target, msg )
#define LOG_DF_ERROR( type, target, msg )   LOG_FD_ERROR( type, target, msg )
#define LOG_DF_MSG( type, target, msg )     LOG_FD_MSG( type, target, msg )
#define LOG_DF_WARNING( type, target, msg ) LOG_FD_WARNING( type, target, msg )

} //end OpenNero

#endif // end _OPEN_NERO_LOG_H_

