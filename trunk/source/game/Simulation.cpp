#include "core/Common.h"
#include <vector>

#include "game/Simulation.h"
#include "game/SimEntity.h"

#include "render/SceneObject.h"

namespace OpenNero
{
    /// Constructor - initialize variables
    Simulation::Simulation( const IrrHandles& irr )
        : mIrr(irr), mMaxId(kFirstSimId)
    {
        // initialize entity types
        for (size_t i = 0; i < sizeof(uint32_t); ++i)
        {
            SimEntitySet entitites;
            mEntityTypes[1 << i] = entitites;
        }
    }

    /// Deconstructor - remove everything
    Simulation::~Simulation()
    {
        clear();
    }

    /**
     * Add an entity to the simulation and assign it the required id. If we cannot assign
     * that id (ie: it is already taken) we should fail.
     * @param ent entity to add
     * @return true if add was successful and ids didnt not conflict
    */
    void Simulation::AddSimEntity( SimEntityPtr ent )
    {
        if (ent->GetSimId() >= mMaxId) {
            mMaxId = ent->GetSimId();
        }
        AssertMsg( ent, "Adding a null entity to the simulation!" );
        AssertMsg( !Find( ent->GetSimId() ), "Entity with id " << ent->GetSimId() << " already exists in the simulation" );
        mSimIdHashedEntities[ ent->GetSimId() ] = ent;
        mEntities.insert(ent);
        uint32_t ent_type = ent->GetType();
        for (size_t i = 0; i < sizeof(uint32_t); ++i) {
            uint32_t t = 1 << i;
            if (ent_type & t) {
                mEntityTypes[t].insert(ent);
            }
        }
        AssertMsg( Find(ent->GetSimId()) == ent, "The entity with id " << ent->GetSimId() << " could not be properly added" );
    }

    /**
     * Request a SimEntity to be removed from the simulation.
     * The actual removal takes place during the RemoveNow call at
     * the end of the next ProcessTick.
     */
    void Simulation::Remove( SimId id )
    {
        mRemoveSet.insert(id);
    }

    /**
     * Remove the SimEntity's currently scheduled for removal
     */
	void Simulation::RemoveAllScheduled()
	{
        SimIdSet::const_iterator removeItr = mRemoveSet.begin();

        for (; removeItr != mRemoveSet.end(); ++removeItr) {

            SimId id = *removeItr;
            
            SimIdHashMap::iterator simItr = mSimIdHashedEntities.find(id);
            
            if( simItr != mSimIdHashedEntities.end() ) {
                SimEntityPtr simE = simItr->second;
                AssertMsg( simE, "Invalid SimEntity stored in our simulation!" );
                // remove also from entities set
                SimEntitySet::iterator simInSet = mEntities.find(simE);
                if (simInSet != mEntities.end()) {
                    mEntities.erase(simInSet);
                }
                // remove also from the type-indexed set
                uint32_t ent_type = simE->GetType();
                for (size_t i = 0; i < sizeof(uint32_t); ++i) {
                    uint32_t t = 1 << i;
                    if (ent_type & t) {
                        simInSet = mEntityTypes[t].find(simE);
                        if (simInSet != mEntityTypes[t].end()) {
                            mEntityTypes[t].erase(simE);
                        }
                    }
                }

                mSimIdHashedEntities.erase(simItr);
            }            

            AssertMsg( !Find(id), "Did not properly remove entity from simulation!" );
        }

        mRemoveSet.clear();
    }

    /// Remove all sim entities from our simulation
    void Simulation::clear()
    {
        // clear our internal containers
        mSimIdHashedEntities.clear();
        mEntities.clear();
    }

    /**
     * Find an object by a given sim id. Lookup in the appropriate table.
     * @param id the sim id to lookup
     * @return the sim entity we found
    */
    SimEntityPtr Simulation::Find( SimId id ) const
    {
        SimIdHashMap::const_iterator itr = mSimIdHashedEntities.find(id);
        return ( itr == mSimIdHashedEntities.end() ) ? SimEntityPtr() : itr->second;
    }

    /**
     * Find an object by a given scene object id. Lookup in the appropriate table.
     * @param id the scene object id to lookup
     * @return the sim entity we found
    */
    SimEntityPtr Simulation::FindBySceneObjectId( SceneObjectId id ) const
    {
        return Find( SimId( ConvertSceneIdToSimId(id) ) );
    }


    /// move the simulation forward by time dt
    void Simulation::ProcessTick( float32_t dt )
    {
        // this step will allow mSimIdHashedEntities to be modified during the ticks
        SimIdHashMap entities_to_tick(mSimIdHashedEntities.begin(), mSimIdHashedEntities.end());
        SimIdHashMap::const_iterator itr = entities_to_tick.begin();
        SimIdHashMap::const_iterator end = entities_to_tick.end();
        
        for( ; itr != end; ++itr ) {
			SimEntityPtr ent = itr->second;
			bool was_removed = mRemoveSet.find(itr->first) != mRemoveSet.end();
			if (!was_removed)
			{
				ent->ProcessTick(dt); // tick only if not removed
			}
        }
        
        // the last step is to remove all the objects that were scheduled during the ticks
        RemoveAllScheduled();
    }
    
    /// @brief Detect and deal with collisions
    /// At this point, mSharedData has already been set to the new position, but
    /// mSceneObject has not. So, if any objects collide, we reset them by reverting
    /// mSharedData's version back to mSceneObject's.
    void Simulation::DoCollisions() 
    {
        SimEntitySet colliders; // set of objects that could be colliding with something
        {
            SimIdHashMap::const_iterator itr = mSimIdHashedEntities.begin();
            SimIdHashMap::const_iterator end = mSimIdHashedEntities.end();
            for (; itr != end; ++itr)
            {
                SimEntityPtr ent = itr->second;
                if (ent->CanCollide())
                    colliders.insert(ent);
            }
        }

		SimEntitySet colliding; // set of objects that are colliding with something

		// while there are any potential collisions, check and resolve
        SimEntitySet::const_iterator itr;
        SimEntitySet colliding_new;
        
        // add any colliding entities to colliding_new
        for (itr = colliders.begin(); itr != colliders.end(); ++itr)
        {
            SimEntityPtr ent = *itr;
            if (ent->IsColliding(GetEntities(ent->GetCollision())))
            {
                colliding_new.insert(ent);
            }
        }

        if (colliding_new.size() > 0) {
            // move the newly marked entities from colliders to colliding
            for (itr = colliding_new.begin(); itr != colliding_new.end(); ++itr) {
                colliders.erase(*itr);
                colliding.insert(*itr);
            }
        }

		if (colliding.size() > 0) {
			for (itr = colliding.begin(); itr != colliding.end(); ++itr) {
				(*itr)->ResolveCollision();
			}
		}
    }
    
    const SimEntitySet Simulation::GetEntities(size_t types) const
    {
        SimEntitySet result;
        for (size_t i = 0; i < sizeof(uint32_t); ++i) {
            uint32_t t = 1 << i;
            if (types & t) {
                hash_map<uint32_t, SimEntitySet>::const_iterator type_set = mEntityTypes.find(t);
                Assert(type_set != mEntityTypes.end());
                result.insert(type_set->second.begin(), type_set->second.end());
            }
        }
        return result;
    }

} //end OpenNero
