//--------------------------------------------------------
// OpenNero : AudioManager
//  manages all of the audio buffers
//--------------------------------------------------------

#include "core/Common.h"

#if NERO_BUILD_AUDIO

#include "AudioManager.h"
#include "boost/pool/detail/singleton.hpp"

namespace OpenNero
{
    /// Constructor - set the absolute path
    /// @param absPath the absolute path for this sound resource
    SoundBuffer::SoundBuffer( const std::string& absPath) :
        mALid(AL_NONE),
        mAbsolutePath(absPath),
        mLoaded(false)
    {}

    /// Dtor - destroy the sound buffer
    SoundBuffer::~SoundBuffer()
    {
        if( mLoaded )
        {
            ReleaseBuffer();            
        }
    }

    /// Get the absolute file path for this object
    const std::string& SoundBuffer::GetAbsolutePath() const
    {
        return mAbsolutePath;
    }

    /// @return true if the buffer is currently in memory
    bool SoundBuffer::IsLoaded() const
    {
        return mLoaded;
    }

    /// Load a buffer from the path initially passed into us
    /// @return true if we successfully loaded the buffer
    bool SoundBuffer::LoadBuffer()
    {
        // could not find it, try loading
        ALuint id = aleLoadFile( mAbsolutePath );

        // load was successful, add to map
        if( id != AL_NONE )
        {
            // create the sound
            mALid       = id;
            mLoaded     = true;
        }
        
        return id != AL_NONE;
    }

    /// Release the buffer from memory    
    void SoundBuffer::ReleaseBuffer()
    {
        Assert( mALid != AL_NONE );
        alDeleteBuffers( 1, &mALid );
        mLoaded = false;

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error deleting buffer " << mAbsolutePath << " : " << AleUtil::makeAlErrorString(error) );
        }
    }

    AudioSource::AudioSource( SourceId id )
        : mSourceId(id)
    {}

    /// Attach a sound buffer to this audio source
    /// @param buffer the sound buffer to attach
    /// @return true if the attaching went well
    bool AudioSource::AttachSoundBuffer( SoundBufferPtr buffer )
    {
        Assert( buffer );
        Assert( buffer->mALid != AL_NONE );
        Assert( alcGetCurrentContext() );

        // release an old buffer if we have one
        if( mSoundBuffer && !ReleaseSoundBuffer() )
            return false;
        
        // attach the buffer
        alSourcei( mSourceId, AL_BUFFER, buffer->mALid );

        // check for any errors
        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error attaching buffer " << buffer->mAbsolutePath << " to source id " << mSourceId << " : " << AleUtil::makeAlErrorString(error) );
            return false;
        }
        
        // save the buffer
        mSoundBuffer = buffer;

        return true;
    }
    
    /// Release a sound buffer from an audio source
    /// @return true if the detach was successful
    bool AudioSource::ReleaseSoundBuffer()
    {
        Assert( mSoundBuffer );
        Assert( mSoundBuffer->mALid != AL_NONE );

        // stop the source if it is playing
        if( IsPlaying() && !StopSource() )            
            return false;

        // detach the buffer
        alSourcei( mSourceId, AL_BUFFER, 0 );

        // check for any errors
        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error releasing buffer " << mSoundBuffer->mAbsolutePath << " from source id " << mSourceId << " : " << AleUtil::makeAlErrorString(error) );
            return false;
        }

        // release the buffer
        mSoundBuffer.reset();

        return true;
    }    

    /// Check if this source is currently playing
    /// @return true if source is playing
    bool AudioSource::IsPlaying() const
    {
        ALint playing;
        alGetSourcei( mSourceId, AL_SOURCE_STATE, &playing );

        // check for any errors
        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error checking playing status from source id " << mSourceId << " : " << AleUtil::makeAlErrorString(error) );
            return false;
        }

        return playing == AL_PLAYING;
    }
     
    /// Check if this source is stopped or paused
    bool AudioSource::IsStopped() const
    {
        return !IsPlaying();
    }

    /// Play an attached sound buffer from this source
    /// @return true if no errors reported by OpenAL
    bool AudioSource::PlaySource() const
    {
        AssertMsg( mSoundBuffer, "No attached sound buffer" );
        alSourcePlay( mSourceId );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error playing source " << mSourceId << " : " << AleUtil::makeAlErrorString(error) );
            return false;
        }

        return true;
    }

    /// Stop a currently playing source
    /// @return true if stop was succesful
    bool AudioSource::StopSource() const
    {
        AssertMsg( mSoundBuffer, "No attached sound buffeR" );
        alSourceStop( mSourceId );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error stopping source " << mSourceId << " : " << AleUtil::makeAlErrorString(error) );
            return false;
        }

        return true;
    }

    void AudioSource::SetVolume( float32_t volumeLevel )
    {
        Assert( volumeLevel >= 0.0f );
        alSourcef( GetSourceId(), AL_GAIN, volumeLevel );
    }
    
    float32_t AudioSource::GetVolume() const
    {
        float32_t volumeLevel;
        alGetSourcef( GetSourceId(), AL_GAIN, &volumeLevel );
        return volumeLevel;
    }

    /// Get our source id    
    SourceId AudioSource::GetSourceId() const
    {
        return mSourceId;
    }

    SoundSource::SoundSource( SourceId id )
        : AudioSource(id)
    {}
    
    /// Set the position of the sound source
    /// @param pos the position
    void SoundSource::SetPosition( const Vector3f& pos )
    {   
        alSourcefv( GetSourceId(), AL_POSITION, (float32_t*)&pos );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting source position - " << GetSourceId() << " : " << AleUtil::makeAlErrorString(error) );            
        }
    }
     
    /// Set the orientation of the sound source
    /// @param at the sound direction
    /// @param up the up direction (not used)
    void SoundSource::SetOrientation( const Vector3f& at, const Vector3f& up )
    {
        alSourcefv( GetSourceId(), AL_DIRECTION, (float32_t*)&at );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting source orientation - " << GetSourceId() << " : " << AleUtil::makeAlErrorString(error) );            
        }
    }

    /// Set the sound source velocity
    /// @param vel the velocity to use
    void SoundSource::SetVelocity( const Vector3f& vel )
    {
        alSourcefv( GetSourceId(), AL_VELOCITY, (float32_t*)&vel );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting source velocity - " << GetSourceId() << " : " << AleUtil::makeAlErrorString(error) );            
        }
    }

    void SoundSource::SetSoundDistance( float32_t distance )
    {
        alSourcef( GetSourceId(), AL_REFERENCE_DISTANCE, distance );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting source sound distance - " << GetSourceId() << " : " << AleUtil::makeAlErrorString(error) );            
        }
    }

    void SoundSource::SetRolloffFactor( float32_t rolloff )
    {
        alSourcef( GetSourceId(), AL_ROLLOFF_FACTOR, rolloff );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting source rolloff factor - " << GetSourceId() << " : " << AleUtil::makeAlErrorString(error) );            
        }
    }

    /// Set the position of the listener
    /// @param pos the listener position
    void SoundListener::SetPosition( const Vector3f& pos )
    {
        alListenerfv( AL_POSITION, (float32_t*)&pos );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting listener position - " << AleUtil::makeAlErrorString(error) );            
        }
    }

    /// Set the orientation of the listener
    void SoundListener::SetOrientation( const Vector3f& at, const Vector3f& up )
    {
        static Vector3f orientation[2];
        orientation[0] = at;
        orientation[1] = up;

        alListenerfv( AL_ORIENTATION, (float32_t*)&orientation[0] );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting listener orientation - " << AleUtil::makeAlErrorString(error) );            
        }
    }

    /// Set the velocity of the listener
    void SoundListener::SetVelocity( const Vector3f& vel )
    {
        alListenerfv( AL_VELOCITY, (float32_t*)&vel );

        ALenum error = alGetError();
        if( error != AL_NO_ERROR )
        {
            LOG_F_ERROR( "audio",  "Error setting listener velocity - " << AleUtil::makeAlErrorString(error) );            
        }
    }

    // this is the maximum number of sources that we will manage
    // (actually number may be less due to hardware constraints)
    static uint32_t suHardwareSourceLimit = 64;

    // The default sound distance at which sources begin to attenuate
    const float32_t AudioManager::kDefaultSoundDistance = 32.0f;

    /// Get the hardware source max limit on sources
    uint32_t AudioManager::GetHardwareSourceMax()
    {
        return suHardwareSourceLimit;
    }

    /// Get the singleton
    /// @return reference to the singleton instance
    AudioManager& AudioManager::instance()
    {
        return boost::details::pool::singleton_default<AudioManager>::instance();
    }
    
    /// Get the singleton
    /// @return reference to the singleton instance
    const AudioManager& AudioManager::const_instance()
    {
        return boost::details::pool::singleton_default<AudioManager>::instance();
    }

    /// Initialize the audio manager
    /// @return true if success
    bool AudioManager::Init()
    {
        // initialize ale
        if( !aleInit() )
            return false;

        // print out the current version
        LOG_F_MSG( "audio",  OpenNero::aleVersion() );

        // generate our sources
        const uint32_t numSources = GenerateSources();
        if( numSources == 0 )
        {
            LOG_F_ERROR( "audio",  "Unable to create any sound sources..." );
            return false;
        }

        LOG_F_MSG( "audio",  "Generated " << numSources << " sound sources." );

        // create our listener
        mSoundListener.reset( new SoundListener() );

        // setup some OpenAL constants

        // non-clamped linear inverse distance model
        //gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE));
        alDistanceModel(AL_INVERSE_DISTANCE);

        // set the volume at full blast?
        SetMasterVolumeLevel(1.0f);

        // set the background music at a tenth
        SetMusicVolumeLevel(0.1f);

        // success
        return true;
    }

    /// Shut down the audio manager
    /// @return true if success
    bool AudioManager::Shutdown()
    {
        // clear all the sounds
        mSoundMap.clear();

        // shut down ale
        return aleShutdown();
    }

    /// Play a given sound file as the background music.
    /// Note: ProcessTick should update the music source to
    /// follow the listener
    /// @param absPath the path of the music file
    /// @param loopMusic whether or not to loop the music forever
    /// @return true if the buffer could be found and played
    bool AudioManager::PlayMusic( const std::string& absPath, bool loopMusic )
    {
        SoundBufferPtr buffer = GetSoundBuffer(absPath);

        // attach to music buffer and play
        if( buffer && mMusicSource && mMusicSource->AttachSoundBuffer(buffer) )
        {
            mLoopMusic = loopMusic;
            return mMusicSource->PlaySource();
        }

        return false;
    }

    /// Play the audio file with the given orientation data
    /// @param absPath the path to the sound file
    /// @param data the orientation data
    /// @return true if we played the sound
    bool AudioManager::PlaySound( const std::string& absPath, const PlayAudioData& data )
    {
        SoundBufferPtr buffer = GetSoundBuffer(absPath);
        if( buffer )
        {
            // get a free sound source
            SoundSourcePtr soundSrc = GetFreeSource();
            Assert(soundSrc);

            // TODO : Do we want this functionality?
            // project the sound AT the player
            Vector3f listenerPos = GetListenerPosition();
            Vector3f soundAt = data.mPosition - listenerPos;
            soundAt = soundAt.normalize();

            // attach buffer and add to playing list
            soundSrc->AttachSoundBuffer(buffer);

            // set the properties of the sound source for positional sound
            soundSrc->SetPosition( data.mPosition );
            soundSrc->SetOrientation( soundAt, data.mUp );
            //soundSrc->SetOrientation( data.mAt, data.mUp );
            soundSrc->SetVelocity( data.mVelocity );
            soundSrc->SetSoundDistance(mStandardSoundDistance);
            mPlayingList.push_back(soundSrc);

            LOG_F_MSG( "audio", "Playing sound source - " << absPath );

            // play sound
            return soundSrc->PlaySource();
        }

        // no good
        return false;
    }

    /// Attempt to stop a sound that is playing
    /// @param absPath the path of the buffer to the sound
    /// @return true if we stopped a sound, false if we couldnt find the sound
    bool AudioManager::StopSound( const std::string& absPath )
    {
        // do we even have this buffer?
        SoundMap::iterator itr = mSoundMap.find(absPath);
        if( itr != mSoundMap.end() )
        {   
            SoundBufferPtr soundBuffer = itr->second;

            SoundSourceList::iterator sItr = mPlayingList.begin();
            SoundSourceList::iterator sEnd = mPlayingList.end();

            // try to find a playing sound with a matching buffer
            for( ; sItr != sEnd; ++sItr )
            {
                if( (*sItr)->mSoundBuffer == soundBuffer )
                {
                    // stop the source if it is currently playing
                    if( (*sItr)->IsPlaying() )
                        (*sItr)->StopSource();

                    // move from the playing list to the free list
                    mFreeList.push_back(*sItr);
                    mPlayingList.erase(sItr);

                    return true;
                }
            }
        }

        AssertWarnMsg( false, "Trying to stop a sound that is not playing" );
        return false;
    }

    /// Update any per-frame related audio things by dt
    void AudioManager::ProcessTick( float32_t dt )
    {
        // check to see which of the playing list sources are still playing
        SoundSourceList::iterator itr = mPlayingList.begin();
        SoundSourceList::iterator end = mPlayingList.end();

        while( itr != end )
        {
            // if not playing, move to free list
            if( !(*itr)->IsPlaying() )
            {
                mFreeList.push_back(*itr);
                itr = mPlayingList.erase(itr);
            }
            else
            {
                ++itr;
            }                
        }

        Assert( mSoundListener );
        Assert( mMusicSource );

        // check to see if the music has stopped and optionally restart it
        if( mMusicSource && mMusicSource->IsStopped() && mLoopMusic )
            mMusicSource->PlaySource();        

        // update the music source position to be that of the listener if playing
        if( mMusicSource && mMusicSource->IsPlaying() )
        {
            Vector3f pos, vel;
            Vector3f orientation[2];

            alGetListenerfv( AL_POSITION, (float32_t*)&pos );
            alGetListenerfv( AL_ORIENTATION, (float32_t*)&orientation[0] );
            alGetListenerfv( AL_VELOCITY, (float32_t*)&vel );

            // music should be at the player, so at = 0
            orientation[0] = Vector3f(0,0,0);

            mMusicSource->SetPosition(pos);
            mMusicSource->SetOrientation( orientation[0], orientation[1] );
            mMusicSource->SetVelocity(vel);        
        }
    }

    /// Set the position of the listener
    void AudioManager::SetListenerPosition( const Vector3f& pos )
    {   
        alListenerfv( AL_POSITION, (float32_t*)&pos );
    }

    /// Set the orientation of the listener
    void AudioManager::SetListenerOrientation( const Vector3f& at, const Vector3f& up )
    {
        static Vector3f orientation[2];
        orientation[0] = at;
        orientation[1] = up;
        alListenerfv( AL_ORIENTATION, (float32_t*)&orientation[0] );
    }

    /// Set the velocity of the listener
    void AudioManager::SetListenerVelocity( const Vector3f& vel )
    {
        alListenerfv( AL_VELOCITY, (float32_t*)&vel );
    }

    void AudioManager::SetStandardSoundDistance( float32_t distance )
    {
        mStandardSoundDistance = distance;
    }

    void AudioManager::SetMusicVolumeLevel( float32_t volumeLevel )
    {
        Assert( mMusicSource );
        mMusicSource->SetVolume(volumeLevel);
    }

    void AudioManager::SetMasterVolumeLevel( float32_t volumeFactor )
    {
        Assert( volumeFactor >= 0.0f );
        alListenerf( AL_GAIN, volumeFactor + 0.00001f );
    }

    float32_t AudioManager::GetMasterVolumeLevel() const
    {
        float32_t volume;
        alGetListenerf( AL_GAIN, &volume );
        return volume;
    }

    AudioManager::AudioManager()
        : mStandardSoundDistance(kDefaultSoundDistance)
    {}

    /// Generate as many sources as the software and hardware will allow us to manage
    uint32_t AudioManager::GenerateSources()
    {
        // generate all many sources as we can to manage
        mFreeList.clear();
        mPlayingList.clear();

        // generate until we hit our software max or our hardware max
        while( mFreeList.size() < GetHardwareSourceMax() )
        {
            SourceId sourceId;
            alGenSources( 1, &sourceId );

            ALenum error = alGetError();
            if( error != AL_NO_ERROR )
            {
                suHardwareSourceLimit = (uint32_t)mFreeList.size();
                break;
            }
            
            mFreeList.push_back( SoundSourcePtr( new SoundSource(sourceId) ) );
        }

        // if we have more than one, reserve one for the music source
        if( mFreeList.size() > 1 )
        {
            mMusicSource = mFreeList.front();
            mFreeList.pop_front();
        }

        return (uint32_t)mFreeList.size();
    }

    /// Get a free source either from the free list or by killing an old source
    /// @return a free source ptr
    SoundSourcePtr AudioManager::GetFreeSource()
    {
        // do we have anything in the free list?
        if( mFreeList.size() > 0 )
        {
            SoundSourcePtr ptr = mFreeList.front();
            mFreeList.pop_front();
            return ptr;
        }

        // we need to kill the oldest playing sound
        Assert( mPlayingList.size() > 0 );
        SoundSourcePtr ptr = mPlayingList.front();
        mPlayingList.pop_front();
        Assert( ptr );

        // stop the sound if it is currently playing
        if( ptr->IsPlaying() )
            ptr->StopSource();
        
        // return back this pointer
        return ptr;
    }

    /// Get a sound buffer either from our cache or from loading it from file
    /// @param absPath the absolute path to the sound file
    /// @return a ptr to a sound buffer, possibly null on fail
    SoundBufferPtr AudioManager::GetSoundBuffer( const std::string& absPath )
    {
        // try to find the sound buffer in the map
        SoundMap::iterator itr = mSoundMap.find(absPath);
        if( itr != mSoundMap.end() )
            return itr->second;

        // otherwise, we need to load it
        SoundBufferPtr soundBuffer( new SoundBuffer(absPath) );
        if( soundBuffer->LoadBuffer() )
        {
            mSoundMap[absPath] = soundBuffer;
            return soundBuffer;
        }

        // could not find or load
        return SoundBufferPtr();
    }

    /// Get the position of the listener
    Vector3f AudioManager::GetListenerPosition() const
    {
        Vector3f pos;
        alGetListenerfv( AL_POSITION, (float32_t*)&pos );
        return pos;            
    }

    void AudioManager::UnitTest()
    {
        AudioManager& am = instance();

        const std::string soundPath = "main/data/rocket_launcher.ogg";

        PlayAudioData data;
        data.mPosition = Vector3f(0,0,0);
        data.mAt       = Vector3f(0,0,0);
        data.mUp       = Vector3f(0,0,1);
        data.mVelocity = Vector3f(0,0,0);

        am.SetListenerPosition( Vector3f(0,0,0) );
        am.SetListenerVelocity( Vector3f(0,0,0) );
        am.SetListenerOrientation( Vector3f(1,0,0), Vector3f(0,0,1) );
        
        // center
        am.mMusicSource->SetPosition( data.mPosition );
        am.mMusicSource->AttachSoundBuffer( am.GetSoundBuffer( soundPath ) );
        am.mMusicSource->PlaySource();
        while( am.mMusicSource->IsPlaying() );

        // left
        data.mPosition = Vector3f( 0, 10, 0 );
        am.mMusicSource->SetPosition( data.mPosition );
        am.mMusicSource->AttachSoundBuffer( am.GetSoundBuffer( soundPath ) );
        am.mMusicSource->PlaySource();
        while( am.mMusicSource->IsPlaying() );

        // right
        data.mPosition = Vector3f( 0, -10, 0 );
        am.mMusicSource->SetPosition( data.mPosition );
        am.mMusicSource->AttachSoundBuffer( am.GetSoundBuffer( soundPath ) );
        am.mMusicSource->PlaySource();
        while( am.mMusicSource->IsPlaying() );

        // front
        data.mPosition = Vector3f( 10, 0, 0 );
        am.mMusicSource->SetPosition( data.mPosition );
        am.mMusicSource->AttachSoundBuffer( am.GetSoundBuffer( soundPath ) );
        am.mMusicSource->PlaySource();
        while( am.mMusicSource->IsPlaying() );

        // back
        data.mPosition = Vector3f( -10, 0, 0 );
        am.mMusicSource->SetPosition( data.mPosition );
        am.mMusicSource->AttachSoundBuffer( am.GetSoundBuffer( soundPath ) );
        am.mMusicSource->PlaySource();
        while( am.mMusicSource->IsPlaying() );
    }

} // end OpenNero

#endif // NERO_BUILD_AUDIO
