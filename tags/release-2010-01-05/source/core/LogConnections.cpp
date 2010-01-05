//---------------------------------------------------
// Name: OpenNero : LogConnections
// Desc:  implementations of the LogConnection interface
//---------------------------------------------------

#include "core/Common.h"
#include "LogConnections.h"
#include <iostream>
#include <fstream>

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

#if NERO_DEBUG
            mStream.open(file);
#else
            mStream.open(file, std::ios_base::app);
#endif
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

	} // end Log	
	
} //end OpenNero
