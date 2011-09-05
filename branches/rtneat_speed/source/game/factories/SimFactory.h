//--------------------------------------------------------
// OpenNero : SimFactory
//  Simulation Factory
//--------------------------------------------------------

#ifndef _GAME_FACTORIES_SIMFACTORY_H
#define _GAME_FACTORIES_SIMFACTORY_H

#include "IrrFactory.h"

namespace OpenNero
{
	class SimFactory;
	BOOST_SHARED_DECL(SimFactory);

    /// Loads resources for our simulation
    class SimFactory
    {
    public:

		SimFactory( const IrrHandles& irr );

		// get the irrlicht factory
		IrrFactory& getIrrFactory();
		const IrrFactory& getIrrFactory() const;

    public:

        // change a relative path to include the mod or other alterations
        static std::string TransformPath( const std::string& prePath );

    private:

		/// Factory for creating Irrlicht resources
		IrrFactory			mIrrFactory; 
    };

} //end OpenNero

#endif // _GAME_FACTORIES_SIMFACTORY_H
