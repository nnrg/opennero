#include "core/Common.h"
#include "utils/Config.h"

#include <vector>

#include "game/Simulation.h"
#include "game/SimEntity.h"

#include "render/SceneObject.h"

#include "ai/AIManager.h"
#include "ai/AIObject.h"
#include "ai/AgentBrain.h"

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
            if (t > ent_type) break; // shortcut
            if (ent_type & t) {
                mEntityTypes[t].insert(ent);
            }
        }

        { // also make sure to add the triangle selector for this object to 
          // all relevant meta selectors
            hash_map<uint32_t, IMetaTriangleSelector_IPtr>::iterator iter;
            for (iter = mCollisionSelectors.begin(); iter != mCollisionSelectors.end(); ++iter) {
                // if the entity type matches the stored mask
                if (iter->first & ent_type) {
                    // add the triangles to that selector
                    ITriangleSelector_IPtr tri_selector = ent->GetSceneObject()->GetTriangleSelector();
                    iter->second->addTriangleSelector(tri_selector.get());
                    LOG_F_DEBUG("collision", "added " << tri_selector->getTriangleCount() << " triangles for a total of " << iter->second->getTriangleCount() << " triangles that collide with type " << iter->first);
                }
            }
        }
        
        GetCollisionTriangleSelector(ent_type);
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

        // clear out entities hashed by id
        mSimIdHashedEntities.clear();

        // clear out iteration order list
        mEntities.clear();

        // clear out triangle selector cache
        {
            hash_map<uint32_t, IMetaTriangleSelector_IPtr>::iterator iter;
            for (iter = mCollisionSelectors.begin(); iter != mCollisionSelectors.end(); ++iter) {
                iter->second->removeAllTriangleSelectors();
            }
            mCollisionSelectors.clear();
        }

        // clear out type set cache
        {
            hash_map<uint32_t, SimEntitySet>::iterator iter;
            for (iter = mEntityTypes.begin(); iter != mEntityTypes.end(); ++iter) {
                iter->second.clear();
            }
            mEntityTypes.clear();
        }
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
        for (itr = entities_to_tick.begin(); itr != entities_to_tick.end(); ++itr) {
            SimId id = itr->first;
            SimEntityPtr ent = itr->second;
            if (ent->IsRemoved()) {
                SimIdHashMap::iterator simItr = mSimIdHashedEntities.find(id);
                
                if( simItr != mSimIdHashedEntities.end() ) {
                    SimEntityPtr simE = simItr->second;
					AIObjectPtr brain = simE->GetAIObject();
					if (brain) {
						brain->getBrain()->destroy();
					}
                    AssertMsg( simE, "Invalid SimEntity on delete, id: " << id );
                    // remove also from entities set
                    SimEntitySet::iterator simInSet = mEntities.find(simE);
                    if (simInSet != mEntities.end()) {
                        mEntities.erase(simInSet);
                    }
                    
                    // remove also from the type-indexed set
                    uint32_t ent_type = simE->GetType();
                    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
                        uint32_t t = 1 << i;
                        if (t > ent_type) break; // shortcut
                        if (ent_type & t) {
                            simInSet = mEntityTypes[t].find(simE);
                            if (simInSet != mEntityTypes[t].end()) {
                                mEntityTypes[t].erase(simE);
                            }
                        }
                    }

                    { // also make sure to remove the triangle selector for this object from 
                      // all relevant meta selectors
                        hash_map<uint32_t, IMetaTriangleSelector_IPtr>::iterator iter;
                        for (iter = mCollisionSelectors.begin(); iter != mCollisionSelectors.end(); ++iter) {
                            // if the entity type matches the stored mask
                            if (iter->first & ent_type) {
                                // remove the triangles from that selector
                                iter->second->removeTriangleSelector(ent->GetSceneObject()->GetTriangleSelector().get());
                            }
                        }
                    }

                    mSimIdHashedEntities.erase(simItr);
                }

                AssertMsg( !Find(id), "Did not properly remove entity from simulation!" );
            }
        }
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
            // gone past the possible type masks?
            if (t > types) break;
            if (types & t) {
                hash_map<uint32_t, SimEntitySet>::const_iterator type_set = mEntityTypes.find(t);
                Assert(type_set != mEntityTypes.end());
                result.insert(type_set->second.begin(), type_set->second.end());
            }
        }
        return result;
    }
    
    /// get a triangle selector for all the objects matching the types mask
    IMetaTriangleSelector_IPtr Simulation::GetCollisionTriangleSelector( size_t types )
    {
        IMetaTriangleSelector_IPtr meta_selector;
        
        // first, lookup the type mask in our table
        hash_map<uint32_t, IMetaTriangleSelector_IPtr>::const_iterator needle;
        needle = mCollisionSelectors.find(types);
        
        if (needle != mCollisionSelectors.end()) {
            // if found, return the selector
            meta_selector = needle->second;
        } else {
            // if not found, create the selector
            meta_selector = mIrr.getSceneManager()->createMetaTriangleSelector();
            SimEntitySet ents = GetEntities(types);
            // iterate over all entities of that type and add them to the selector
            for (SimEntitySet::const_iterator iter = ents.begin(); iter != ents.end(); ++iter)
            {
                SimEntityPtr ent = *iter;
                ITriangleSelector_IPtr tri_selector = ent->GetSceneObject()->GetTriangleSelector();
                meta_selector->addTriangleSelector(tri_selector.get());
            }
            // remember for future reuse
            mCollisionSelectors[types] = meta_selector;
            LOG_F_DEBUG("collision", "created triangle selector for mask " 
                << types << " with: " << meta_selector->getTriangleCount() << " triangles");        
        }
        
        return meta_selector;
    }


} //end OpenNero
