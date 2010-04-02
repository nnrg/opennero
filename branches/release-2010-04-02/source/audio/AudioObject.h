//--------------------------------------------------------
// OpenNero : AudioObject
//  a template for audio resources
//--------------------------------------------------------

#ifndef _AUDIO_OBJECT_H_
#define _AUDIO_OBJECT_H_

#include "game/objects/TemplatedObject.h"
#include "core/ONTypes.h"
#include "core/IrrUtil.h"
#include "core/BoostCommon.h"

namespace OpenNero
{
    /// @cond
    BOOST_SHARED_DECL( AudioObjectTemplate );
    BOOST_SHARED_DECL( AudioObject );
    /// @endcond

    /// The audio object template holds all the necessary information to load
    /// an actual audio object for the objects in game to use
    class AudioObjectTemplate : public ObjectTemplate
    {
    public:
        /// Constructor for an AudioObjectTemplate given another AudioObjectTemplate
        AudioObjectTemplate( const AudioObjectTemplate& objTempl );
        /// Constructor for an AudioObjectTemplate given a factory and property map
        AudioObjectTemplate( SimFactoryPtr factory, const PropertyMap& propMap );
        virtual ~AudioObjectTemplate();

        /// create the concrete type of template we need
        static shared_ptr<AudioObjectTemplate> createTemplate(SimFactoryPtr factory, const PropertyMap& propMap);

        /// Return the template type of an AudioObjectTemplate
        static const std::string TemplateType() { return "AudioObject"; }

    public:

        /// < AudioName, AudioPath >
        typedef std::pair< std::string, std::string > AudioPair;

        /// vector of AudioPairs
        typedef std::vector< AudioPair > AudioPairVector;

    public:

        AudioPairVector     mAudioPairs;   ///< Name/Path pairs vector
    };

    class SimEntityData;

    /// The audio object is the actual holder of all of an object's audio buffers
    class AudioObject : public TemplatedObject
    {
    public:

        AudioObject();
        /// Copy constructor for an AudioObject
        AudioObject( const AudioObject& obj );
        ~AudioObject();

        /// load the scene object from a template
        virtual bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data );

        /// get the template
        ObjectTemplatePtr GetObjectTemplate();

        /// set the shared data for this AudioObject
        void SetSharedState( SimEntityData* sharedData );

        /// process a tick for this AudioObject
        void ProcessTick( float32_t dt );

		/// play a sound through this AudioObject
        bool PlaySound( const std::string& soundName );

    private:

        // < AudioName, AudioPath >
        typedef std::map< std::string, std::string > AudioSoundMap;

    private:

        AudioSoundMap           mSounds;        ///< The name/sound map
        AudioObjectTemplatePtr  mAudioTemplate; ///< The audio template we loaded from

        SimEntityData*          mSharedData;
    };

} // end OpenNero


#endif // _AUDIO_OBJECT_H_
