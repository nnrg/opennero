//--------------------------------------------------------
// OpenNero : File
//  file utilities
//--------------------------------------------------------

#include "core/Common.h"
#include "File.h"
#include <fstream>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
namespace fs = boost::filesystem;

namespace OpenNero 
{   
    /**
     * Read the contents of a file into a string
     * @param filePath the absolute path to the file
     * @return a string representing all of the contents of the file
    */
    std::string ReadFileToString( const std::string& filePath )
    {
        // try to open the file
        std::ifstream inFile( filePath.c_str() );        

        // did it work?
        if( !inFile )
            return "";		 

        // read file into a stream
        std::stringstream file;
		char ch;
        while (inFile.get(ch)) {
			if (ch != '\r') { // fix a mac problem
				file << ch;
			}
        }

        // close the file
        inFile.close();

        return file.str();
    }

    std::string ConvertNeroToSystemPath( const std::string& pathName )
    {
        fs::path p(pathName);
        return p.string();
    }

    /// Check for the existence of a file at a given path
    /// @param filePathName the file path to check for existence
    /// @return true if the file exists
    bool FileExists( const std::string& filePathName )
    {
        return fs::exists(fs::path(filePathName));
    }

    /// Join two path elements
    std::string FilePathJoin( const std::string& p1, const std::string& p2 )
    {
		fs::path path1(p1);
		fs::path path2(p2);
        return (path1 / path2).string();
    }

	/// Create a directory
	bool CreateDirectory( const std::string& filePathName ) {
		return fs::create_directory(filePathName);
	}

} //end OpenNero


