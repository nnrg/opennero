//---------------------------------------------------
// Name: OpenNero : LogConnections
// Desc:  implementations of the LogConnection interface
//---------------------------------------------------

#ifndef _OPEN_NERO_LOG_CONNECTIONS_H_
#define _OPEN_NERO_LOG_CONNECTIONS_H_

#include <assert.h>
#include <fstream>
#include "core/Preprocessor.h"
#include "Log.h"
#include "scripting/scriptIncludes.h"

namespace OpenNero
{	
	namespace Log
    {
        /**
         * An ILogConnection concrete class that takes in any "stream" type 
         * object for logging use. This "stream" type object must implement
         * operators << and >> for character streams. Specific overrides may
         * need to be created for the flush() method to provide proper flushing.
        */
        template <typename T>
        class StreamLogConnection : public ILogConnection
        {
        private:

            /// a pointer to the stream we want to use. Note that we do not store the memory, only point to it
            T* mStreamPtr;

            /// the name of the connection
            std::string mConnectionName;

        protected:

            /**
             * Dump a message into our stream
             * @param msg the character buffer to dump
            */
            void dump( const char* msg )
            {
                assert( mStreamPtr );
                (*mStreamPtr) << msg << "\n";
            }

        public:

            /// a flushing method that children can override or inherit
            virtual void flush() {}

            /// constructor
            StreamLogConnection( const std::string connectionName, T* streamPtr = 0 ) 
                : mStreamPtr(streamPtr),
                  mConnectionName(connectionName) {}          

            /// destructor
            ~StreamLogConnection()
            {
                flush();
            }

            /// set the pointer to the stream
            /// @param streamPtr the pointer
            void setStreamPtr( T* streamPtr )
            {
                mStreamPtr = streamPtr;
            }

			/// log a debug message
			void LogDebug( const char* msg )
			{
#if NERO_DEBUG
				dump(msg);
                flush();
#endif
			}

            /// log a message
            void LogMsg( const char* msg )
            {
                dump(msg);
                flush();
            }

            /// log a warning
            void LogWarning( const char* msg )
            {
                dump(msg);
                flush();
            }

            /// log an error
            void LogError( const char* msg )
            {
                dump(msg);
                flush();
            }

            /// get the name of the connection
            const std::string getConnectionName() const
            {
                return mConnectionName;
            }
        };
        
        /**
         * An abstract utility class whose only purpose is to store the
         * allocation of a given stream object S. The rules for S are as
         * defined in StreamLogConnection.
        */
        template <typename S>
        class ContainerStreamConnection : public StreamLogConnection<S>
        {
        protected:

            /// the stream we are storing
            S mStream;

        public:

            /// constructor, just setup the stream pointer
            ContainerStreamConnection( const std::string connectionName )
                : StreamLogConnection<S>(connectionName)
            {
                this->setStreamPtr( &mStream );
            }

            /// pure virtual destructor to make this class abstract
            virtual ~ContainerStreamConnection() = 0;
        };

        // a definition for the abstractor b.
        template< typename S >
        ContainerStreamConnection<S>::~ContainerStreamConnection() {}

        /**
         * A stream which will send all of its output to a file. Takes in the filename
         * as a character array and manages the rest
        */
        class FileStreamConnection : public ContainerStreamConnection< std::ofstream >
        {

        public:            

            // ctor / dtor
            FileStreamConnection( const char* connectionName, const char* file );
            ~FileStreamConnection();

            // do a special flush for the file object
            void flush();
        };

        /// PyLogConnection allows us to send logs to a logging system implemented in Python
        /// This can be something in Python's logging module, or something that sends messages
        /// over the network.
        class PyLogConnection : public ILogConnection
        {
        	/// a Python object that implements logging methods
        	boost::python::object logging_object;

        	/// write to the log
        	void Write( const char* msg );
        public:
            /// constructor
        	PyLogConnection();

            /// destructor
            ~PyLogConnection();

			/// log a debug message
			void LogDebug( const char* msg );

            /// log a message
            void LogMsg( const char* msg );

            /// log a warning
            void LogWarning( const char* msg );

            /// log an error
            void LogError( const char* msg );

            /// get the name of the connection
            const std::string getConnectionName() const;
        };
    } // end Log
     
	
} //end OpenNero

#endif // end _OPEN_NERO_LOG_CONNECTIONS_H_
    
