//--------------------------------------------------------
// OpenNero : SimEntity
//  a 'thing' that can be simulated
//--------------------------------------------------------

#ifndef _GAME_OBJ_SIMENTITY_H_
#define _GAME_OBJ_SIMENTITY_H_

#include <set>
#include <vector>
#include <ostream>
#include "core/ONTypes.h"
#include "core/IrrUtil.h"
#include "core/BoostCommon.h"
#include "game/SimEntityData.h"

namespace OpenNero
{
	/// @cond
    BOOST_PTR_DECL( Simulation );
    BOOST_PTR_DECL( SimContext );
    BOOST_PTR_DECL( SimEntity );
    BOOST_PTR_DECL( SceneObject );
    BOOST_PTR_DECL( AIObject );
    BOOST_PTR_DECL( SensorObject );
    /// @endcond

    /// Set of SimEnities
    typedef std::set<SimEntityPtr> SimEntitySet;

    /// List of SimEntities
    typedef std::list<SimEntityPtr> SimEntityList;

    /// an unique identifier that used to identify objects locally
    typedef uint32_t SimId;

    const SimId kInvalidSimId = 0; ///< Signifies that an object is not valid
    const SimId kFirstSimId   = 1; ///< The first id...

    /**
    * A simulation entity is able to update itself in the world. After it updates
    * itself it can send its changes on the server to all of the clients
    */
    class SimEntity
    {
        // Allow Simulation to manage SimEntity ids and parent sim.
        friend class Simulation;

    public:
        /// Create a new sim entity using given creation data
        static SimEntityPtr CreateSimEntity(
            SimEntityData& data,
            const std::string& templateName,
            SimContextPtr context);
        // This method is supposed to refine and extend the current data in the entity
        static void InitializeSimEntity(
            SimEntityPtr ent,
            SimEntityData& data,
            const std::string& templateName,
            SimContextPtr context);
    public:

        /// default constructor
        SimEntity(const SimEntityData& data, const std::string& templateName);

		/// default destructor
        ~SimEntity();

        /// called before the simulation tick takes place
        void BeforeTick( float32_t incAmt );

        /// called to tick the rendering component
        void TickScene( float32_t incAmt);

        /// called to tick the AI component
        void TickAI( float32_t incAmt);

        /// update our entity during an animation frame (no AI tick)
        void ProcessAnimationTick( float32_t frac );

        /// Set the template that we were loaded from
        std::string GetCreationTemplate() const;

        // get the id
        SimId GetSimId() const;

	/// @{
	/// setters for component objects
        void SetSceneObject( SceneObjectPtr obj );
        void SetAIObject( AIObjectPtr obj );
	/// @}

		/// getters for SimEntity components
		/// @{
        SceneObjectPtr GetSceneObject() { return mSceneObject; }
        AIObjectPtr GetAIObject() { return mAIObject; }
		/// @}

        /// make sure you explicitly call this method
        /// in a derived classes' assignment operator
        SimEntity& operator=( const SimEntity& obj );

		/// Get a constant reference to the shared data for this SimEntity
        const SimEntityData& GetState() const { return mSharedData; }

        /// Getters for properties
        /// @{
        const Vector3f& GetPosition() const;
        const Vector3f& GetRotation() const;
        const Vector3f& GetVelocity() const;
        const Vector3f& GetScale() const;
        const std::string& GetLabel() const;
        const SColor& GetColor() const;
        uint32_t GetType() const;
        uint32_t GetCollision() const;
        /// @}

		/// Setters for properties
        /// @{
        void SetPosition( const Vector3f& pos );
        void SetRotation( const Vector3f& rot );
        void SetVelocity( const Vector3f& vel );
        void SetScale( const Vector3f& scale );
        void SetLabel( const std::string& label );
        void SetColor( const SColor& color );
        void SetCollision( uint32_t mask );
        /// @}

        /// Make the state update immediate, "teleporting" the agent
        /// to the current state and ignoring collisions
        void UpdateImmediately();

        /// Is the object marked for removal
        bool IsRemoved() const { return mRemoved; }

        /// Mark the object for removal
        void SetRemoved() { mRemoved = true; }
    private:
        /// output human-readable information about this SimEntity
        friend std::ostream& operator<<(std::ostream& stream, const SimEntityPtr&);

    private:

        /// Set the template that we were created from
        void SetCreationTemplate( const std::string& creationTemplate );

    private:

        /// Sim entity components
		/// @{
		/// hook into AI
        AIObjectPtr         mAIObject;

        /// hook into rendering
        SceneObjectPtr      mSceneObject;
		/// @}

        /// the data that is shared between components
        SimEntityData   mSharedData;

        /// the template that we were loaded from
        std::string     mCreationTemplate;

        /// removed flag
        bool            mRemoved;
    };

} //end OpenNero

#endif // _GAME_OBJ_SIMENTITY_H_
