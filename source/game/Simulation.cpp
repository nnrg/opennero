#include "core/Common.h"
#include "utils/Config.h"

#include <vector>

#include "game/Simulation.h"
#include "game/SimEntity.h"

#include "render/SceneObject.h"

#include "ai/AIManager.h"

namespace OpenNero
{
    /// Constructor - initialize variables
    Simulation::Simulation( const IrrHandles& irr )
        : mIrr(irr)
        , mMaxId(kFirstSimId)
        , mFrameDelay(GetAppConfig().FrameDelay)
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
        AssertMsg( ent, "Adding a null entity to the simulation!" );
        AssertMsg( !Find( ent->GetSimId() ), "Entity with id " << ent->GetSimId() << " already exists in the simulation" );
        mSimIdHashedEntities[ ent->GetSimId() ] = ent;
        mEntities.insert(ent);
        mEntitiesAdded.push_back(ent);
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
        SimEntityPtr ent = Find(id);
        if (ent) {
            ent->SetRemoved();
        }
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
        SimIdHashMap::const_iterator itr;
        
        // render all objects
        for(itr = entities_to_tick.begin() ; itr != entities_to_tick.end(); ++itr ) {
			SimEntityPtr ent = itr->second;
            if (!ent->IsRemoved()) {
                ent->BeforeTick(dt);
                ent->TickScene(dt);
            }
        }
        
        // make AI decisions
        if (AIManager::instance().IsEnabled())
        {
            for(itr = entities_to_tick.begin() ; itr != entities_to_tick.end(); ++itr ) {
                SimEntityPtr ent = itr->second;
                if (!ent->IsRemoved()) {
                    ent->TickAI(dt);
                }
            }
            SimEntityList::const_iterator added_itr;
            
            // iterate over the freshly added entities as well to ensure that they move if they need to
            for (added_itr = mEntitiesAdded.begin(); added_itr != mEntitiesAdded.end(); ++added_itr)
            {
                SimEntityPtr ent = *added_itr;
                if (!ent->IsRemoved())
                {
                    ent->BeforeTick(dt);
                    ent->TickAI(dt);
                }
            }
        }                
        
        mEntitiesAdded.clear();
        
        // iterate over all the entities deleting those marked for removal
        // also make a list of all the entities that need to be check for collision
        SimEntitySet colliders; // set of objects that could be colliding with something
        for (itr = entities_to_tick.begin(); itr != entities_to_tick.end(); ++itr) {
            SimId id = itr->first;
            SimEntityPtr ent = itr->second;
            if (ent->IsRemoved()) {
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
            } else {
                // need to check for collision?
                if (ent->CanCollide())
                    colliders.insert(ent);
                ent->SetBumped(false);
            }
        }
        
        // TODO: implement collision
        //{
        //    SimEntitySet::iterator itr = colliders.begin();
        //    SimEntitySet::iterator end = colliders.end();
        //    for (; itr != end; ++itr)
        //    {
        //        SimEntityPtr ent = *itr;
        //        Vector3f desired_position = ent->GetState().GetPosition();
        //        Vector3f current_position = ent->GetState().GetPrevious().GetPosition();
        //        
        //    }
        //}
    }
    
    void Simulation::ProcessAnimationTick( float32_t frac )
    {
        // this step will allow mSimIdHashedEntities to be modified during the ticks
        SimIdHashMap entities_to_tick(mSimIdHashedEntities.begin(), mSimIdHashedEntities.end());
        SimIdHashMap::const_iterator itr = entities_to_tick.begin();
        SimIdHashMap::const_iterator end = entities_to_tick.end();
        
        for( ; itr != end; ++itr ) {
			SimEntityPtr ent = itr->second;
            ent->ProcessAnimationTick(frac); // tick only if not removed
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
