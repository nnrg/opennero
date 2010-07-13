//--------------------------------------------------------
// OpenNero : Ale
//  audio library extensions
//--------------------------------------------------------

#ifndef _AUDIO_ALE_H_
#define _AUDIO_ALE_H_

#include "core/Common.h"

#if NERO_BUILD_AUDIO

#if NERO_PLATFORM_WINDOWS
#include <alc.h>
#include <al.h>
#include <alut.h>
#elif NERO_PLATFORM_MAC
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#include <FreeALUT/alut.h>
#else
#include <openal/al.h>
#include <openal/alc.h>
#include <openal/alut.h>
#endif

/**

Ale is a small extension of the OpenAL and the Alut libraries.
Ale stands for "Audio Library Extension" and is designed to provide utility functions
and resource management features to the already present libraries.

*/


namespace OpenNero 
{
    namespace AleUtil
    {        
        std::string makeAlErrorString( ALenum err );        
        std::string makeAlutErrorString( ALenum err );        
        
    }; // end AleUtil namespace


    typedef std::vector<std::string> DeviceList; /// A Vector of supported devices on the machine
    typedef std::string				 AleVersion; /// The version of the library

    // Initialize the audio system
    bool		aleInit();

    // Shut down the audio system
    bool		aleShutdown();

    // Query the system for the library version we are using
    AleVersion	aleVersion();

    // Query the system for the supported devices on this machine
    DeviceList	aleQueryDevices();	

    // Load an audio file and return a buffer id (currently support .wav and .ogg)
    ALuint		aleLoadFile( const std::string& filePath );

    // Get the current device
    ALCdevice*&		Device();

    // Get the current context
    ALCcontext*&	Context();

} //end OpenNero

#endif // NERO_BUILD_AUDIO

#endif // _AUDIO_ALE_H_
