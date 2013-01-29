#ifndef _GAME_SIMULATION_H_
#define _GAME_SIMULATION_H_

#include <set>
#include <list>
#include "core/HashMap.h"
#include "core/Common.h"
#include "core/IrrUtil.h"
#include "game/SimEntity.h"
#include "render/SceneObject.h"

namespace OpenNero
{
	/// @cond
    BOOST_SHARED_DECL( SimEntity );
    BOOST_SHARED_DECL( Environment );
    BOOST_SHARED_DECL( Simulation );
    /// @endcond

    /// The Simulation manages every object in the game that needs to be updated in any sort of way (local or remote).
    /// It manages all of its objects with a given SimId with which the Simulation and the NetConnections can reference
    /// it on any machine
    class Simulation
    {
    public:

        /// Constructor
        /// @param irr irrlicht handles
        Simulation( const IrrHandles& irr );

        /// Deconstructor
        virtual ~Simulation();

        /** @name Container Methods - Management */
        ///@{

        /// Schedule a SimEntity to be added to the simulation
        /// @param ent the entity to add
        void AddSimEntity( SimEntityPtr ent );

        /// Schedule a SimEntity for removal
        /// @param id the id of type to remove by
        void Remove( SimId id );

        // remove all entities
        void clear();

        /// find an entity by its simulation ID
        SimEntityPtr Find( SimId id ) const;

        /// find an entity by its SceneObject ID
        SimEntityPtr FindBySceneObjectId( SceneObjectId id ) const;

        /// Get the set of all the entities in the simulation
        const SimEntitySet GetEntities() const { return mEntities; }

        /// Get the set of all the entities of the specified type
        const SimEntitySet GetEntities( size_t types ) const;

        /// Get the next free SimId
        SimId ReserveNewId() { mMaxId += 1; return mMaxId; }

        ///@}

        /// move the simulation forward by time dt
        void ProcessTick( float32_t dt );

        /// update for animation only
        void ProcessAnimationTick( float32_t frac );

        /// get the time (in seconds) to animate for between AI frames
        float32_t GetFrameDelay() const { return mFrameDelay; }

        /// set the time (in seconds) to animate for between AI frames
        void SetFrameDelay(float32_t delay) { mFrameDelay = delay; }
        
        /// get a triangle selector for all the objects matching the types mask
        IMetaTriangleSelector_IPtr GetCollisionTriangleSelector( size_t types );

    protected:

        /// hash map of SimEntities indexed by SimId
        typedef hash_map< SimId, SimEntityPtr > SimIdHashMap;

        /// a set of simulation IDs
        typedef std::set<SimId> SimIdSet;

    protected:

        IrrHandles          mIrr;                   ///< Copy of Irrlicht handles

        SimIdHashMap        mSimIdHashedEntities;   ///< Our entities hashed by SimId

        SimEntitySet        mEntities;              ///< Set of all the sim entities

        SimEntityList       mEntitiesAdded;         ///< Entities are added to this list at first, so that they can be ticked immediately

        hash_map<uint32_t, SimEntitySet> mEntityTypes; ///< entity sets by type

        /// the triangle selectors for objects to collide with (by type)
        mutable hash_map<uint32_t, IMetaTriangleSelector_IPtr> mCollisionSelectors;

        EnvironmentPtr      mWorld;                 ///< The AI World interface

        SimId               mMaxId;                 ///< The maximum id of the objects in the simulation

        float32_t           mFrameDelay;            ///< The time (in seconds) to animate for between AI frames

    };

} //end OpenNero

#endif // _GAME_SIMULATION_H_
