//--------------------------------------------------------
// OpenNero : AudioManager
//  manages all of the audio buffers
//--------------------------------------------------------

#ifndef _AUDIO_MANAGER_H_
#define _AUDIO_MANAGER_H_

#if NERO_BUILD_AUDIO

#include "Ale.h"
#include "core/BoostCommon.h"
#include "core/IrrUtil.h"
#include <vector>

namespace OpenNero
{
    typedef ALuint BufferId;
    typedef ALuint SourceId;

    /// SoundBuffer is a wrapper around the ALuint identifier for audio buffers.
    class SoundBuffer
    {
        // let the manager touch our stuffs
        friend class AudioManager;
        friend class AudioSource;

    public:

        // ctor/dtor
        SoundBuffer( const std::string& absPath );
        ~SoundBuffer();

        // get the absolute path for this resource
        const std::string& GetAbsolutePath() const;

        // is this buffer loaded into memory?
        bool IsLoaded() const;

    private:

        // loading and freeing of buffers
        bool  LoadBuffer();
        void  ReleaseBuffer();

    private:

        BufferId        mALid;              ///< The OpenAL sound buffer id
        std::string     mAbsolutePath;      ///< The absolute path this was loaded from
        bool            mLoaded;            ///< Determines if we are loaded in memory or flat

    };

    /// @cond
    BOOST_SHARED_DECL( SoundBuffer );
    BOOST_SHARED_DECL( AudioSource );
    BOOST_SHARED_DECL( SoundSource );
    BOOST_SHARED_DECL( AudioManager );
    BOOST_SHARED_DECL( SoundListener );
    /// @endcond

    /// An audio source is the base class of both a music and sound source.
    /// It gives the basic functionality to these two classes
    class AudioSource
    {
        friend class AudioManager;

    public:

        // assign an id to this audio source
        AudioSource( SourceId id );

        // buffer management methods
        bool AttachSoundBuffer( SoundBufferPtr buffer );
        bool ReleaseSoundBuffer();

        // status methods
        bool IsPlaying() const;
        bool IsStopped() const;

        // playback methods
        bool PlaySource() const;
        bool StopSource() const;

        // volume methods
        void SetVolume( float32_t volumeLevel );
        float32_t GetVolume() const;

    protected:

        // get our identifier
        SourceId    GetSourceId() const;

    private:

        SourceId                mSourceId;      ///< our identifier to the manager
        SoundBufferPtr          mSoundBuffer;   ///< the buffer of sound we are currently playing
    };

    /// A sound source is a positional audio source
    class SoundSource : public AudioSource
    {
    public:

        SoundSource( SourceId id );

        // position component methods
        void SetPosition( const Vector3f& pos );
        void SetOrientation( const Vector3f& at, const Vector3f& up );
        void SetVelocity( const Vector3f& vel );

        /// Set the distance at which the sound will begin to attenuate. In other words, you
        /// will hear this sound until this distance
        /// @param distance the distance to begin attenuation
        void SetSoundDistance( float32_t distance );

        /// Set how quickly the attenuation occurs once past the sound distance [0.0 - R]
        /// @param rolloff the rolloff factor
        void SetRolloffFactor( float32_t rolloff );
    };

    /// A sound listener is a receiver of audio data from the world
    class SoundListener
    {
    public:

        // positional component methods
        void SetPosition( const Vector3f& pos );
        void SetOrientation( const Vector3f& at, const Vector3f& up );
        void SetVelocity( const Vector3f& vel );
    };

    /// The audio manager is responsible for storage and management of all sound buffers as well as sound playback
    class AudioManager
    {
        // allow this singleton_default class to allocate us
		friend struct boost::details::pool::singleton_default<AudioManager>;

    public:

        // get the number of source ids we are managing
        static uint32_t GetHardwareSourceMax();

    public:

        /// Data related to playing a positional source
        struct PlayAudioData
        {
            Vector3f        mPosition;      ///< where the source is coming from
            Vector3f        mAt;            ///< the direction the source is going
            Vector3f        mUp;            ///< the up direction relative to the source
            Vector3f        mVelocity;      ///< the velocity of the source
        };

    public:

        // singleton pattern
        static AudioManager& instance();
        static const AudioManager& const_instance();

        // initialize the manager
        bool Init();

        // shut down the manager
        bool Shutdown();

        // music playback methods
        bool PlayMusic( const std::string& absPath, bool loop = false );
        bool PlaySound( const std::string& absPath, const PlayAudioData& data );
        bool StopSound( const std::string& absPath );

        // update method
        void ProcessTick( float32_t dt );

        // listener properties
        void SetListenerPosition( const Vector3f& pos );
        void SetListenerOrientation( const Vector3f& at, const Vector3f& up );
        void SetListenerVelocity( const Vector3f& vel );

        /// Set the distance at which sounds begin to attenuate
        /// @see SoundSource
        /// @param distance the distance which sound sources attenuate
        void SetStandardSoundDistance( float32_t distance );

        /// Set the volume level of the music alone
        void SetMusicVolumeLevel( float32_t volumeLevel );

        /// Set the volume control for the audio manager
        /// @param volumeFactor a percentage to set the voluem at [0.0f - 1.0f]
        void SetMasterVolumeLevel( float32_t volumeFactor );

        /// Get the current volume level
        float32_t GetMasterVolumeLevel() const;

    protected:

        AudioManager();

        // generate all the sources we can manage
        uint32_t GenerateSources();

        // source and buffer accessors
        SoundSourcePtr GetFreeSource();
        SoundBufferPtr GetSoundBuffer( const std::string& absPath );

        // get the position of the listener
        Vector3f GetListenerPosition() const;

    public:

        static void UnitTest();

    private:

        typedef std::map< std::string, SoundBufferPtr > SoundMap;
        typedef std::list< SoundSourcePtr > SoundSourceList;

    private:

        SoundMap                 mSoundMap;       ///< Our map from paths to sounds

        SoundSourcePtr           mMusicSource;    ///< A source reserved for music playback
        SoundSourceList          mFreeList;       ///< A list of free sound sources
        SoundSourceList          mPlayingList;    ///< A list of currently playing sources

        SoundListenerPtr         mSoundListener;  ///< The sound listener

        bool                     mLoopMusic;      ///< flag that tells us whether or not to loop the music
        float32_t                mStandardSoundDistance; /// the default sound distance to use for sound sources

    private:

        static const float32_t kDefaultSoundDistance;
    };

} // end OpenNero

#endif // NERO_BUILD_AUDIO

#endif // _AUDIO_MANAGER_H_
