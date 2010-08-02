//--------------------------------------------------------
// OpenNero : Simulation
//  our physically simulated world
//--------------------------------------------------------

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

        /// Remove all SimEntity's scheduled for removal
        void RemoveAllScheduled();

        // remove all entities
        void clear();

        /// find an entity by its simulation ID
        SimEntityPtr Find( SimId id ) const;

        /// find an entity by its SceneObject ID
        SimEntityPtr FindBySceneObjectId( SceneObjectId id ) const;

        SimId GetNextFreeId() const { return mMaxId + 1; }

        ///@}

        // move the simulation forward by time dt
        void ProcessWorld( float32_t dt );
        
        // detect and deal with collisions
        void DoCollisions();

        /** @name AI Functions */
        ///@{

        /// Connect to the AI world (server only)
        inline void setWorld(EnvironmentPtr world) { mWorld = world; }

        /// Get the AI world (server only)
        inline EnvironmentPtr getWorld() { return mWorld; }

        ///@}
    protected:

        /// hash by simulation ID
        typedef hash_map< SimId, SimEntityPtr > SimIdHashMap;

        /// list of simulation IDs to be removed
        typedef std::set<SimId> SimIdSet;

    protected:

        IrrHandles          mIrr;                   ///< Copy of Irrlicht handles

        SimIdHashMap        mSimIdHashedEntities;   ///< Our entities hashed by SimId

        SimIdSet            mRemoveSet;             ///< SimId's to remove after next ProcessWorld

        EnvironmentPtr      mWorld;                 ///< The AI World interface

        SimId               mMaxId;                 ///< The maximum id of the objects in the simulation
    };

} //end OpenNero

#endif // _GAME_SIMULATION_H_
