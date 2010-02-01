#ifndef _GAME_OBJECTS_SIMENTITYCOMPONENT_H_
#define _GAME_OBJECTS_SIMENTITYCOMPONENT_H_

#include "game/SimEntity.h"
#include "game/SimEntityData.h"

namespace OpenNero {
	
    /// A base class for components of SimEntity
	class SimEntityComponent
	{
	protected:
		SimEntityWPtr mParent;                                                              ///< Parent entity
		SimEntityData* mSharedData;                                                         ///< Shared data for entity 
	public:
		SimEntityComponent(SimEntityPtr parent) : mParent(parent), mSharedData(NULL) {}     ///< SimEntityComponent constructor
		virtual ~SimEntityComponent() {}                                                    ///< virtual destructor
		void SetSharedState(SimEntityData* data) { mSharedData = data; }                     ///< Set the entity's shared data
		SimEntityData* GetSharedState() { return mSharedData; }                              ///< Return the entity's shared data
		SimEntityPtr GetEntity() { return mParent.lock(); }                                 ///< Return the entity
	};
}

#endif /*_GAME_OBJECTS_SIMENTITYCOMPONENT_H_*/
