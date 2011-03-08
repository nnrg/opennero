//--------------------------------------------------------
// OpenNero : TemplatedObject
//  An object that can make use of an object template
//--------------------------------------------------------

#include "core/Common.h"
#include "TemplatedObject.h"

namespace OpenNero
{
	/// copy costructor
	ObjectTemplate::ObjectTemplate( const ObjectTemplate& objTempl )
	{
		mTemplateName = objTempl.mTemplateName;
		mpSimFactory  = objTempl.mpSimFactory;
	}

	/**
	 * Construct the object using a factory and a property map
	 * @param factory the factory to load things from
	 * @param propMap property map to extract values from
	*/
	ObjectTemplate::ObjectTemplate( SimFactoryPtr factory, const PropertyMap& propMap ) :
														mTemplateName("DefaultTemplate")
													,	mpSimFactory(factory)
	{
		propMap.getValue( mTemplateName, "Template.ObjectTemplate.Name" );
	}

} //end OpenNero
