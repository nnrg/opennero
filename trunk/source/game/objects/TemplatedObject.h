//--------------------------------------------------------
// OpenNero : TemplatedObject
//  An object that can make use of an object template
//--------------------------------------------------------

#ifndef _GAME_OBJECTS_TEMPLATED_H_
#define _GAME_OBJECTS_TEMPLATED_H_

#include "PropertyMap.h"
#include "core/IrrSerialize.h"
#include "core/Common.h"
#include "game/SimEntityData.h"

namespace OpenNero
{
	class SimFactory;
    class ObjectTemplate;
	BOOST_SHARED_DECL(SimFactory);
    BOOST_SHARED_DECL(ObjectTemplate);

	/// An object template is the basic structure for storing data that will not vary
	/// from instance to instance. It is the basic flyweight pattern
    class ObjectTemplate : public BOOST_SHARED_THIS(ObjectTemplate)
	{
	public:

		ObjectTemplate( const ObjectTemplate& objTempl );
		ObjectTemplate( SimFactoryPtr factory, const PropertyMap& propMap );
		virtual ~ObjectTemplate() = 0;

	public:

		/// The template name
		std::string mTemplateName;

		/// A factory for loading things
		SimFactoryPtr mpSimFactory;
	};
	
	inline ObjectTemplate::~ObjectTemplate() { } // defined for speed

	/// A templated objects is responsible for loading itself from a template
	class TemplatedObject
	{
	public:
	    virtual ~TemplatedObject() = 0;
		/// load this object from a template
		virtual bool LoadFromTemplate( ObjectTemplatePtr objTemplate, const SimEntityData& data ) = 0;
	};
	
    inline TemplatedObject::~TemplatedObject() { } // defined for speed

} //end OpenNero

#endif // _GAME_OBJECTS_TEMPLATED_H_
