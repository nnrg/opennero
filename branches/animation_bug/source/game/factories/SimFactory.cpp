//--------------------------------------------------------
// OpenNero : SimFactory
//  Simulation Factory
//--------------------------------------------------------

#include "core/Common.h"
#include "SimFactory.h"
#include "game/Kernel.h"

namespace OpenNero
{
	/**
	 * Constructor - initializes the factories
	 * @param irr a copy of the irrlicht handles to use in loading
	*/
	SimFactory::SimFactory( const IrrHandles& irr )
									: mIrrFactory(irr)

	{}

	/// Get the Irrlicht Factory
	/// @return reference to the irrlicht factory
	IrrFactory& SimFactory::getIrrFactory()
	{
		return mIrrFactory;
	}

	/// Get the Irrlicht Factory
	/// @return const reference to the irrlicht factory
	const IrrFactory& SimFactory::getIrrFactory() const
	{
		return mIrrFactory;
	}

    /**
     * To determine if we should add the current mod to this path, check for the
     * existance of the ~/ prefix on the path. So,
     * data/img.png should become myMod/data/img.png        but
     * ~/data/img.png should become data/img.png
     * @param prePath the path to modify
     * @return the modified path
     */
    std::string SimFactory::TransformPath( const std::string& prePath )
    {
        // make a mod path
        if( prePath.length() <= 2 || prePath[0] != '~' || prePath[1] != '/' )
            return Kernel::findResource( prePath );

        // return the absolute path
        return prePath.substr(2);
    }

} //end OpenNero
