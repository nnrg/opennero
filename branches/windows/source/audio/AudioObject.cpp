//--------------------------------------------------------
// OpenNero : AudioObject
//  a template for audio resources
//--------------------------------------------------------

#include "core/Common.h"

#if NERO_BUILD_AUDIO

#include "audio/AudioObject.h"
#include "audio/AudioManager.h"
#include "game/SimEntityData.h"
#include "game/factories/SimFactory.h"

namespace OpenNero
{
   
    // Ctor
    // @param objTempl the audio object template to load from
    AudioObjectTemplate::AudioObjectTemplate( const AudioObjectTemplate& objTempl ) : 
    	ObjectTemplate(objTempl),
        mAudioPairs(objTempl.mAudioPairs)
    {}

    /**
     * Factory method to create the appropriate type of AudioObjectTemplate depending on the XML data provided
     * @param factory SimFactory used to grab resources
     * @param propMap property map that contains the information from the XML file
     * @return an instance of an AudioObjectTemplate child suitable for generating new Audio objects
     */
    AudioObjectTemplatePtr AudioObjectTemplate::createTemplate(SimFactoryPtr factory, const PropertyMap& propMap)
    {
        // we only have one type of AudioObjectTemplate for now, so return that type unconditionally
        AudioObjectTemplatePtr p(new AudioObjectTemplate(factory, propMap));
        return p;
    }

    // Ctor
    // @param factory the factory to load things with
    // @param propMap the property map holding our information
    AudioObjectTemplate::AudioObjectTemplate( SimFactoryPtr factory, const PropertyMap& propMap )
        : ObjectTemplate(factory, propMap)
    {
        // find all of the audio paths
        PropertyMap::ChildPropVector audioPairs;
        propMap.getPropChildren( audioPairs, "Template.Audio" );

        // save all the paths
        PropertyMap::ChildPropVector::const_iterator itr = audioPairs.begin();
        PropertyMap::ChildPropVector::const_iterator end = audioPairs.end();

        for( ; itr != end; ++itr )
        {
            // format < AudioName, AudioPath >
            mAudioPairs.push_back( AudioPair( itr->first, itr->second ) );          
        }
    }
    
    /// Dtor
    AudioObjectTemplate::~AudioObjectTemplate() {}

    /// Ctor - does nothing
    AudioObject::AudioObject()
        : mSharedData(0)
    {}

    /// Ctor
    /// @param obj the object to copy
    AudioObject::AudioObject( const AudioObject& obj )
        : mSounds(obj.mSounds)
        , mAudioTemplate(obj.mAudioTemplate)
        , mSharedData(0)
         
    {}

    /// Dtor
    /// clear our sound map
    AudioObject::~AudioObject()
    {
        mSounds.clear();        
    }

    /// Load a udio object from an object template
    /// @param objTemplate the template to load from
    /// @return true if successful
    bool AudioObject::LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data )
    {
        // cast to object template to the type we expect
        AudioObjectTemplatePtr audioTemplate = shared_static_cast< AudioObjectTemplate, ObjectTemplate>( objTemplate );

        // if we have no pairs, dont load
        if( !audioTemplate->mAudioPairs.size() )
            return false;

        // save all of our sounds
        AudioObjectTemplate::AudioPairVector::const_iterator itr = audioTemplate->mAudioPairs.begin();
        AudioObjectTemplate::AudioPairVector::const_iterator end = audioTemplate->mAudioPairs.end();        

        for( ; itr != end; ++itr )
        {   
            mSounds[itr->first] = itr->second;
        }

        // success
        return true;
    }

    /// Set the data that we shared with other components
    /// @param sharedData ptr to shared data
    void AudioObject::SetSharedState( SimEntityData* sharedData )
    {
        Assert( sharedData );
        mSharedData = sharedData;
    }

    /// Move the component forward a given amount
    void AudioObject::ProcessTick( float32_t dt )
    {
        Assert( mSharedData );

        static int i = 0;
        ++i;

        // test - do some sounds
        //if( i % 379 == 0 )
            //PlaySound("Rocket");
    }

    /// Play a sound that we contain
    /// @param soundName the general sound name to play (not the path)
    bool AudioObject::PlaySound( const std::string& soundName )
    {
        // lookup the path from the name
        AudioSoundMap::iterator itr = mSounds.find(soundName);
        if( itr == mSounds.end() )
            return false;

        Assert( mSharedData );

        const Vector3f& rot = mSharedData->GetRotation();

        // play the sound
        AudioManager::PlayAudioData playData;
        playData.mPosition = mSharedData->GetPosition();
        playData.mVelocity = mSharedData->GetVelocity();
        playData.mUp       = Vector3f(0,0,1);
        playData.mAt       = Vector3f( (float32_t)cos(rot.Z), (float32_t)sin(rot.Z), 0 );

        return AudioManager::instance().PlaySound( itr->second, playData );
    }
    
} // end OpenNero

#endif // NERO_BUILD_AUDIO
