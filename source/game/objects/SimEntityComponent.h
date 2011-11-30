#ifndef _GAME_OBJECTS_SIMENTITYCOMPONENT_H_
#define _GAME_OBJECTS_SIMENTITYCOMPONENT_H_

#include "game/SimEntity.h"
#include "game/SimEntityData.h"

namespace OpenNero {

    /// A base class for components of SimEntity
	class SimEntityComponent
	{
	protected:
		SimEntityWPtr mParent;      ///< Parent entity
		SimEntityData* mSharedData; ///< Shared data for entity
	public:
        /// SimEntityComponent constructor
		SimEntityComponent(SimEntityPtr parent) : mParent(parent), mSharedData(NULL) {}
        /// virtual destructor
		virtual ~SimEntityComponent() {}
        /// Set the entity's shared data
		void SetSharedState(SimEntityData* data) { mSharedData = data; }
        /// Return the entity's shared data
		SimEntityData* GetSharedState() { return mSharedData; }
        /// Return the entity
		SimEntityPtr GetEntity() { return mParent.lock(); }
        /// Return the id of the entity
        SimId GetId() { return GetEntity()->GetSimId(); }
	};
}

#endif /*_GAME_OBJECTS_SIMENTITYCOMPONENT_H_*/
