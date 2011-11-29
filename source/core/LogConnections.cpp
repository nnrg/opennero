//---------------------------------------------------
// Name: OpenNero : LogConnections
// Desc:  implementations of the LogConnection interface
//---------------------------------------------------

#include "core/Common.h"
#include <iostream>
#include <fstream>
#include "LogConnections.h"
#include "scripting/scripting.h"

namespace OpenNero
{	
	namespace Log
	{	
        /**
         * Constructor. Takes in the name of a file as a parameter.
         * @param file the filename to use
         * @param connectionName name of the connection
        */
        FileStreamConnection::FileStreamConnection( const char* connectionName, const char* file )
            : ContainerStreamConnection< std::ofstream >(connectionName)
        {
            if( !file )
            {
                // we should probably throw an exception here
                assert(false);
            }

            mStream.open(file);

            if( !mStream.is_open() )
            {
                // we should probably throw an exception here
                assert(false);
            }            
        }

        /// dtor, closes the file
        FileStreamConnection::~FileStreamConnection()
        {
            mStream.close();
        }         

        /// flushes the file
        void FileStreamConnection::flush()
        {
            mStream.flush();
        }

        /// constructor
        PyLogConnection::PyLogConnection() : logging_object()
        {
            // set up network logging facilities
        	ScriptingEngine::instance().ImportModule("plot_client");
        	ScriptingEngine::instance().Eval("plot_client.NetworkLogWriter", logging_object);
        }

        /// destructor
        PyLogConnection::~PyLogConnection()
        {
        	// TODO: should flush and close the logging object somehow
        }

        /// write to the log
		void PyLogConnection::Write(const char* msg)
		{
			try
			{
				logging_object.attr("write")(msg);
			}
			catch (const boost::python::error_already_set&)
			{
				ScriptingEngine::instance().LogError();
			}
		}

		/// log a debug message
		void PyLogConnection::LogDebug( const char* msg )
		{
			Write(msg);
		}

        /// log a message
        void PyLogConnection::LogMsg( const char* msg )
        {
			Write(msg);
        }

        /// log a warning
        void PyLogConnection::LogWarning( const char* msg )
        {
			Write(msg);
        }

        /// log an error
        void PyLogConnection::LogError( const char* msg )
        {
			Write(msg);
        }

        /// get the name of the connection
        const std::string PyLogConnection::getConnectionName() const
		{
        	return "PyLogConnection";
		}
	} // end Log	
	
} //end OpenNero
