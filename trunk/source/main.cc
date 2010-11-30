//--------------------------------------------------------
// Application entrance

#include "core/Common.h"

#include <string>
#include <vector>
#include <iostream>
#include "core/File.h"

#if NERO_PLATFORM_WINDOWS
    #include <windows.h>
#endif

#if NERO_PLATFORM_MAC
    #include <CoreFoundation/CoreFoundation.h>
#endif

namespace OpenNero
{
    typedef std::map<std::string, std::string> CommandLineParams;
    extern int32_t OpenNeroMain( int argc, char** argv );
}

const char* const kErrMsg = "OpenNero cannot detect the resources it needs to run.\nPlease start OpenNero in the directory which contains the executable.";

int main( int argc, char* argv[] )
{
    using namespace OpenNero;

#if NERO_PLATFORM_MAC
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        std::cerr << "Error changing the current directory " << std::endl;
        return 1;
    }
    CFRelease(resourcesURL);
    chdir(path);
#endif

    // assure that we are starting in the proper folder
    if( !FileExists( "TokenFile" ) )
    {
#if NERO_PLATFORM_WINDOWS
        MessageBox( NULL, LPCSTR(kErrMsg), NULL, MB_ICONERROR );
#else // NERO_PLATFORM_LINUX
        std::cout << kErrMsg << std::endl;
#endif
        return 1;
    }

    // start OpenNero
    return OpenNeroMain(argc, argv);
}
