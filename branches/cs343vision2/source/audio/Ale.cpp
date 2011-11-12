//--------------------------------------------------------
// OpenNero : Ale
//  audio library extensions
//--------------------------------------------------------

#include "core/Common.h"

#if NERO_BUILD_AUDIO

#include "Ale.h"
#include "boost/lexical_cast.hpp"
#include "vorbis/vorbisfile.h"

namespace OpenNero 
{
    namespace AleUtil
    {
        /// Make a string from an al error
        /// @param err error to string-ize
        /// @return string version of the error
        std::string makeAlErrorString( ALenum err )
        {
            switch( err )
            {
            case AL_NO_ERROR:			return "AL_NO_ERROR";
            case AL_INVALID_NAME:		return "AL_INVALID_NAME";
            case AL_INVALID_ENUM:		return "AL_INVALID_ENUM";
            case AL_INVALID_VALUE:		return "AL_INVALID_VALUE";
            case AL_INVALID_OPERATION:	return "AL_INVALID_OPERATION";
            case AL_OUT_OF_MEMORY:		return "AL_OUT_OF_MEMORY";			
            }

            return "-Unrecognized error code: " + err;
        }	

        /// Make a string from an alut error
        /// @param err error to string-ize
        /// @return string version of the error
        std::string makeAlutErrorString( ALenum err )
        {
            return alutGetErrorString(err);
        }

        /// Make a string from an alut error
        /// @param err error to string-ize
        /// @return string version of the error
        std::string makeAlutErrorString()
        {
            return makeAlutErrorString( alutGetError() );
        }

        /// Taken from http://www.gamedev.net/reference/articles/article2031.asp
        ALuint LoadOGG( const std::string& fileName )
        {
            // yes, MSVS I know fopen is old, but the ogg sdk uses it.
            // Can i bribe you with chocolates? No? Fine.
#pragma warning( push ) 
#pragma warning( disable : 4996 ) 

            // try to open the file
            FILE* file = fopen( fileName.c_str(), "rb" );
            if( !file )
                return AL_NONE;	

            OggVorbis_File oggFile;

            // open the ogg file
            ov_open(file, &oggFile, NULL, 0);

            // Get some information about the OGG file
            vorbis_info *pInfo = ov_info(&oggFile, -1);

            // Check the number of channels... always use 16-bit samples
            ALenum format = AL_FORMAT_STEREO16;

            if (pInfo->channels == 1)
                format = AL_FORMAT_MONO16;  

            // The frequency of the sampling rate
            ALsizei freq = pInfo->rate;

            // read in the data
            const uint32_t kBufferSize = 131072;
            char array[kBufferSize];

            long bytes = 0;
            std::vector<uint8_t> buffer;

            // TODO : Mark for Endianess!
            uint32_t endian = 0; // 0 for Little-Endian, 1 for Big-Endian
            int bitStream;

            do 
            {
                // Read up to a buffer's worth of decoded sound data
                bytes = ov_read(&oggFile, array, kBufferSize, endian, 2, 1, &bitStream);

                // Append to end of buffer
                buffer.insert(buffer.end(), array, array + bytes);

            } while (bytes > 0);

            // clear the ogg file, calls fclose for us
            ov_clear(&oggFile);

#pragma warning( pop ) 

            // clear the error state
            alGetError();
            ALenum error = AL_NONE;

            // try to generate a buffer
            ALuint bufferID = 0;
            alGenBuffers(1, &bufferID);

            if( ( error = alGetError() ) != AL_NO_ERROR )
            {
                LOG_F_ERROR( "audio", "Received error " << makeAlErrorString(error) << " when generating buffer for ogg file " << fileName );
                return AL_NONE;
            }

            // update the sound data
            alBufferData( bufferID, format, &buffer[0], static_cast<ALsizei>(buffer.size()), freq );		  

            if( ( error = alGetError() ) != AL_NO_ERROR )
            {
                // report an error
                LOG_F_ERROR( "audio", "Received error " << makeAlErrorString(error) << " when uploading sound data for ogg file " << fileName );

                //delete the buffer
                alDeleteBuffers( 1, &bufferID );

                // done
                return AL_NONE;
            }

            // we're good, return the id
            return bufferID;
        }
    } // end AleUtil namespace


    /// ----------------------------------------------------

    ALCdevice*	gDevice  = NULL;	/// The main device to use for loading/playing soundf iles
    ALCcontext* gContext = NULL;	/// The main context to use for sound rendering

    /**
    * Initialize the entire audio library state. Checks to make sure that 
    * the linked libraries are the correct version, initializes alut, 
    * initializes al, creates a device, and creates a context. 
    * @return false if anything goes awry
    */
    bool aleInit()
    {
        // initialize alut
        if( alutInitWithoutContext(NULL,NULL) != AL_TRUE )
        {
            LOG_F_ERROR( "audio", "Failed to initialize alut with following error: " + AleUtil::makeAlutErrorString() );
            return false;
        }

        // create a device (default)
        // TODO : Let user specify the device to use through some gui menu
        if( !(gDevice = alcOpenDevice(NULL)) )
        {
            LOG_F_ERROR( "audio", "Failed to open al device with following error: " + AleUtil::makeAlErrorString( alGetError() ) );
            return false;
        }

        // create the context to use
        if( !( gContext = alcCreateContext( gDevice, NULL ) ) )
        {
            LOG_F_ERROR( "audio", "Failed to create al context with following error: " + AleUtil::makeAlErrorString( alGetError() ) );
            return false;
        }

        // set the context as the used one
        if( alcMakeContextCurrent(gContext) != AL_TRUE )
        {
            LOG_F_ERROR( "audio", "Failed to make current al context with following error: " + AleUtil::makeAlErrorString( alGetError() ) );
            return false;
        }

        LOG_F_MSG( "audio", "Successfully started ALE" );
        return true;
    }

    /**
    * Shut down the entire audio system. Destroy our context and device,
    * and exit out of alut.
    * @return true if shutdown went well
    */
    // TODO : Should WE or the USER be responsible for destroying sources and buffers?
    bool aleShutdown()
    {
        LOG_F_MSG( "audio", "Shutting down ALE" );

        if( gContext )
        {
            alcDestroyContext( gContext );
            gContext = NULL;
        }

        if( gDevice )
        {
            alcCloseDevice( gDevice );
            gDevice = NULL;
        }

        alutExit();

        return true;
    }

    /// Get the version of the alut library we are using
    /// @return a string "Alut - Version: Major.Minor" => "Alut - Version: 1.23"
    AleVersion aleVersion()
    {
        return "Alut - Version: " +
            boost::lexical_cast<std::string>( alutGetMajorVersion() ) + "." +
            boost::lexical_cast<std::string>( alutGetMinorVersion() );
    }

    /**
    * Query open al for all of the audio devices available to us and return them
    * as a vector. We could do this before, but we're abstracted the char* manipulation
    * to make it more friendly.
    * @return a vector of strings representing the available devices
    */
    DeviceList	aleQueryDevices()
    {
        DeviceList dList;

        const char* devices = alcGetString( NULL, ALC_DEVICE_SPECIFIER );
        while( strlen(devices) != 0 )
        {
            size_t len = strlen(devices);
            dList.push_back(devices);			
            devices += len+1;
        }

        return dList;
    }

    /// Get the current device
    ALCdevice*&		Device()
    {
        return gDevice;
    }

    /// Get the current context
    ALCcontext*&	Context()
    {
        return gContext;
    }

    /**
    * Load a file into an AL buffer. Currently support at least
    * .wav and .ogg files
    * @param filePath the non-mod path to the audio file
    * @return the buffer id if success, AL_NONE if failure
    */
    ALuint aleLoadFile( const std::string& filePath )
    {	  
        ALuint id;

        // do special loading code for ogg files
        if( filePath.find( ".ogg" ) != std::string::npos )
        {
            id = AleUtil::LoadOGG( filePath );
            if( id == AL_NONE )
            {
                LOG_F_ERROR( "audio", "Failed to load ogg file: " << filePath );
                AleUtil::makeAlutErrorString();
            }
        }

        // otherwise, pass to alut
        else
        {
            // yum, passing the buck.
            id = alutCreateBufferFromFile( filePath.c_str() );
            if( id == AL_NONE )
            {                
                LOG_F_ERROR( "audio", "Failed to load file : " << filePath );
                LOG_F_ERROR( "audio", AleUtil::makeAlutErrorString() );
            }
        }

        if( id != AL_NONE )
        {
            LOG_F_MSG( "audio", "Successfully loaded audio file " << filePath );
        }		

        return id;
    }

} //end OpenNero

#endif // NERO_BUILD_AUDIO
