//---------------------------------------------------
// Name: OpenNero : Log
// Desc:  logs information
//---------------------------------------------------

#include "core/Common.h"
#include "Log.h"
#include "LogConnections.h"
#include "scripting/scriptIncludes.h"
#include "game/Kernel.h"
#include <vector>
#include <iostream>

namespace OpenNero
{
	namespace Log
	{
		static ILogConnectionVector sLogConnections;
        static FilterList           sFilterList;

        /// helper utility functions
        namespace LogUtil
        {
            /**
             * Find a given log connection by its name
             * @param connectionName the connection to search for
             * @return a shared ptr to the connection, NULL if none found
            */
            ILogConnectionPtr find( const std::string& connectionName )
            {
                ILogConnectionVector::iterator itr = sLogConnections.begin();
                for( ; itr != sLogConnections.end(); ++itr )
                    if( (*itr)->getConnectionName() == connectionName )
                        return *itr;

                return ILogConnectionPtr();
            }

            /// define a pointer to member method of ILogConnection
            typedef void (ILogConnection::*MemberLogFunc)( const char* msg );

            /// output a message to a member method of ILogConnection on an instance of ILogConnection
            /// @param func the member function to call
            /// @param msgType the type of message
            /// @param connectionName the name of the connection
            /// @param msg the message to output
            void OutputToLogFunc( MemberLogFunc func, const char* msgType, const char* connectionName, const char* msg )
            {
                if( msgType )
                {
                    // if the message type is in the filter list, then ignore it
                    // if we do not contain the 'all' receiver or the msgType receiver, exit
                    if( std::find( sFilterList.begin(), sFilterList.end(), msgType )   != sFilterList.end() )
                    {
                        return;
                    }
                }

                // if the connectionName is null, broadcast to all
                if( !connectionName )
                {
                    ILogConnectionVector::iterator itr = sLogConnections.begin();
			        ILogConnectionVector::iterator end = sLogConnections.end();

			        for( ; itr != end; ++itr )
			        {
                        Assert( *itr );

                        // Ok, this line looks complicated.
                        // 1) Get a reference to the ILogConnection instance
                        // 2) Call a member method on it with the provided member method pointer
                        // 3) Pass msg to that method
                        ((*(*itr)).*func)(msg);
			        }
                }

                // if the connectionName is valid, output to that connection
                else
                {
                    if( ILogConnectionPtr c = LogUtil::find(connectionName) )
                    {
                        ((*c).*func)(msg);
                    }
                }
            }
        }

		/**
		 * Add a connection to our manager. Increment the ref count on conn
		 * @param conn a log connection to add
	    */
		void AddLogConnection( ILogConnectionPtr conn )
		{
			if(conn)
			{
				sLogConnections.push_back(conn);
			}
		}

		/**
		 * Remove a connection from our manager. Decrement the ref count on conn
		 * @param conn a log connection to remove
	    */
		void RemoveLogConnection( ILogConnectionPtr conn )
		{
            std::remove( sLogConnections.begin(), sLogConnections.end(), conn );
		}

        /// Log a debug message
        /// @param type the type of message to communicate
        /// @param connectionName the target connection name, if NULL broadcast to all
        /// @param msg the message to log
        void LogDebug( const char* type, const char* connectionName, const char* msg )
        {
            LogUtil::OutputToLogFunc( &ILogConnection::LogDebug, type, connectionName, msg );
        }

        /// Log a normal message
        /// @param type the type of message to communicate
        /// @param connectionName the target connection name, if NULL broadcast to all
        /// @param msg the message to log
        void LogMsg( const char* type, const char* connectionName, const char* msg )
        {
            LogUtil::OutputToLogFunc( &ILogConnection::LogMsg, type, connectionName, msg );
        }

        /// Log a warning message
        /// @param type the type of message to communicate
        /// @param connectionName the target connection name, if NULL broadcast to all
        /// @param msg the message to log
        void LogWarning( const char* type, const char* connectionName, const char* msg )
        {
            LogUtil::OutputToLogFunc( &ILogConnection::LogWarning, type, connectionName, msg );
        }

        /// Log an error message
        /// @param type the type of message to communicate
        /// @param connectionName the target connection name, if NULL broadcast to all
        /// @param msg the message to log
        void LogError( const char* type, const char* connectionName, const char* msg )
        {
            LogUtil::OutputToLogFunc( &ILogConnection::LogError, type, connectionName, msg );
        }

        /// Setup the log system by adding a file log and a console log
		void LogSystemInit( const std::string& logFileName )
		{
            // initialize the connections
			string log_file = Kernel::instance().findResource(logFileName, false);
			cout << "LOG CREATED in " << log_file << endl;
            ILogConnectionPtr fileLog( new FileStreamConnection( "nero_file_log", log_file.c_str()) );
            ILogConnectionPtr stdioLog( new StreamLogConnection<std::ostream>( "console_log", &std::cout ) );

			AddLogConnection(fileLog);
            AddLogConnection(stdioLog);
		}

        /// Set the list of filters we want to ignore
        void LogSystemSpecifyFilters( const FilterList& flist )
        {
            sFilterList = flist;
        }

        /// clear all the log connections
		void LogSystemShutdown()
		{
			sLogConnections.clear();
		}

	} // end Log

} //end OpenNero
