//--------------------------------------------------------
// OpenNero : File
//  file utilities
//--------------------------------------------------------

#ifndef _CORE_FILE_H_
#define _CORE_FILE_H_

#include "Common.h"

namespace OpenNero 
{
    /// Read the contents of a file into a string
    std::string ReadFileToString( const std::string& filePath );	

    /// Check for the existence of a file
    bool FileExists( const std::string& filePathName );

    /// Convert internal path to system-specific path
    std::string ConvertNeroToSystemPath( const std::string& pathName );

    /// Join two path elements
    std::string FilePathJoin( const std::string& p1, const std::string& p2 );

	/// Create a directory
	bool CreateDirectory( const std::string& filePathName );
} //end OpenNero

#endif // _CORE_FILE_H_
