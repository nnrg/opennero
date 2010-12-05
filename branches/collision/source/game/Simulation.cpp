#include "core/Common.h"
#include <vector>

#include "game/Simulation.h"
#include "game/SimEntity.h"

#include "render/SceneObject.h"

#include "kdtree++/kdtree.hpp"

namespace OpenNero
{
    /// Constructor - initialize variables
    Simulation::Simulation( const IrrHandles& irr )
        : mIrr(irr), mMaxId(kFirstSimId)
    {}

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
        AssertMsg( Find(ent->GetSimId()) == ent, "The entity with id " << ent->GetSimId() << " could not be properly added" );
    }

    /**
     * Request a SimEntity to be removed from the simulation.
     * The actual removal takes place during the RemoveNow call at
     * the end of the next ProcessWorld.
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
        return ( itr == mSimIdHashedEntities.end() ) ? SimEntityPtr( (SimEntity*)0 ) : itr->second;
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
    void Simulation::ProcessWorld( float32_t dt )
    {
        // this step will allow mSimIdHashedEntities to be modified during the ticks
        SimIdHashMap entities_to_tick(mSimIdHashedEntities.begin(), mSimIdHashedEntities.end());
        SimIdHashMap::const_iterator itr = entities_to_tick.begin();
        SimIdHashMap::const_iterator end = entities_to_tick.end();
        
        // TODO: consider adding a "tick" method to python that gets called every ProcessWorld
        
        for( ; itr != end; ++itr ) {
            /*
               FIXME: whether or not something that is removed is still ticked depends on the
               order of iteration, which doesn't even have to depend on IDs!
             */
            if (mRemoveSet.count(itr->first) == 0) {
                AssertMsg( itr->second, "Attempting to process an invalid SimEntity with id: " << itr->first);
                itr->second->ProcessTick(dt);
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
        SimEntitySet not_colliding;
        // assume no-one is colliding at first, put everyone in not_colliding
        {
            SimIdHashMap::const_iterator itr = mSimIdHashedEntities.begin();
            SimIdHashMap::const_iterator end = mSimIdHashedEntities.end();
            
            for( ; itr != end; ++itr ) {
                SimEntityPtr ent = itr->second;
                if (ent->CanCollide())
                    not_colliding.insert(ent);
            }
        }

		bool anyCollisions = true;
		SimEntitySet colliding;

		// while there are any potential collisions, check and resolve
		// TODO: check for collisions with resolved objects?
		while (anyCollisions) {
	        SimEntitySet::const_iterator itr;
			SimEntitySet colliding_new;
			
			// add any colliding entities to colliding_new
		    for (itr = not_colliding.begin(); itr != not_colliding.end(); ++itr)
			{
				SimEntityPtr ent = *itr;
				SimEntitySet my_collisions = ent->GetCollisions(not_colliding);
				if (my_collisions.size() > 0) {
					colliding_new.insert(my_collisions.begin(), my_collisions.end());
				}
            }

			anyCollisions = (colliding_new.size() > 0);

			if (anyCollisions) {
				LOG_F_DEBUG("collision", colliding_new.size() << " new collisions");

				// move the newly marked entities from not_colliding to colliding
				for (itr = colliding_new.begin(); itr != colliding_new.end(); ++itr) {
					not_colliding.erase(*itr);
					colliding.insert(*itr);
				}
			}
        }

		// now all the objects that collided are in the "colliding" set
		// we need to resolve all these collisions
		SimEntitySet::const_iterator itr;

		if (colliding.size() > 0) {
			LOG_F_DEBUG("collision", colliding.size() << " total collisions");
			for (itr = colliding.begin(); itr != colliding.end(); ++itr) {
				(*itr)->ResolveCollision();
			}
		}
    }
    
    const SimEntitySet Simulation::GetEntities(size_t types) const
    {
        SimEntitySet result;
        SimEntitySet::const_iterator entIter;
        for (entIter = mEntities.begin(); entIter != mEntities.end(); ++entIter)
        {
            if ((*entIter)->GetType() & types) result.insert(*entIter);
        }
        return result;
    }

} //end OpenNero
