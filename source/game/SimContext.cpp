//--------------------------------------------------------
// OpenNero : SimContext
//  the context of a simulated world
//--------------------------------------------------------

#include "core/Common.h"

#include "game/SimContext.h"
#include "game/SimEntity.h"
#include "game/Simulation.h"
#include "game/objects/TemplatedObject.h"

#include "gui/GuiManager.h"

#include "render/SceneObject.h"
#include "render/Camera.h"
#include "render/NeroDrawNode.h"
#include "render/LineSet.h"

#include "factories/SimFactory.h"

#include "ai/AIManager.h"

#include "input/IOMapping.h"

#include <sstream>

namespace OpenNero
{
    bool SimContext::HandleEvent( const irr::SEvent& event )
    {
        if( mpGuiManager )
        {
            // feed the event to the gui first
            mpGuiManager->OnEvent(event);

            // update the input receiver
            mInputReceiver = mpGuiManager->HasFocus() ? kIR_Gui : kIR_Game;
        }

        if( mInputReceiver == kIR_Game )
        {
            // update the io state
            mIOMap.GetUserInput(event);

            if( mpCamera )
            {
                // give the camera input (need to disable this for gui focus)
                mpCamera->getCamera()->OnEvent(event);
            }
        }

        // return false so the Irr components can read events
        return false;
    }

    bool SimContext::Initialize(IrrlichtDevice_IPtr device)
    {
        // make copies of the irr handles
        mIrr = IrrHandles(device);

        mpSimulation.reset( new Simulation( mIrr ) );

        return true;
    }

    /**
     * Basic constructor, allocates copies of all the irr handles.
     */
    SimContext::SimContext()
        : mClearColor(255, 100, 101, 140)
        , mInputReceiver( kIR_Game )
    {}

    /// basic destructor
    SimContext::~SimContext()
    {
        FlushContext();
    }

    /// onPush initialization code
    bool SimContext::onPush(int argc, char** argv)
    {
        // initialize our base systems
        ScriptingEngine::instance().init(argc, argv);

        // reset the factory
        mpFactory.reset(new SimFactory( mIrr ));
        Assert( mpFactory );

        // create the gui manager
        mpGuiManager.reset( new GuiManager( mIrr, mpFactory ) );

        // add a custom draw node to the scene manager
        mpNeroNode = new NeroDrawNode( mIrr.getSceneManager()->getRootSceneNode(), mIrr.getSceneManager(), -1);
        Assert( mpNeroNode );

        // initialize the mod from script
        ScriptingEngine& scriptEngine = ScriptingEngine::instance();
        scriptEngine.ExecFile(Kernel::findResource("main.py"));
        const std::string& mode = Kernel::instance().getMod()->mode;
        if (mode.empty())
            scriptEngine.Call("ModMain");
        else
            scriptEngine.Call("ModMain", mode);

        return true;
    }

    /// @param templateName location of the XML template for the object
    /// @param pos initial position of the object
    /// @param rot initial rotation (Euler angle) of the object
    /// @param scale initial scale of the object
    /// @param label initial text label of the object
    /// @param type initial type of the object (has to be smaller than BITMASK_SIZE)
    /// @param collision collision mask of the new object
    /// @return SimId of the newly added object
    SimId SimContext::AddObject( const std::string& templateName, 
                                const Vector3f& pos, 
                                const Vector3f& rot, 
                                const Vector3f& scale,
                                const std::string& label,
                                uint32_t collision, 
                                uint32_t type)
    {
        Assert( mpSimulation );

        // initialize the creation data
        SimId new_id = ReserveNewId();
        SimEntityData data(pos, rot, scale, label, type, collision, new_id);
        data.SetAllDirtyBits();
        SimEntityPtr simEnt = SimEntity::CreateSimEntity(data, templateName, shared_from_this());
        if( simEnt )
        {
            mpSimulation->AddSimEntity(simEnt);
            return new_id;
        }
        return kInvalidSimId;
    }

    bool SimContext::RemoveObject(SimId id )
    {
        Assert( mpSimulation );
        mpSimulation->Remove(id);
        return true;
    }

    /// Add a set of cartesian axes to the world
    void SimContext::AddAxes()
    {
        IrrFactory& irrFactory = this->mpFactory->getIrrFactory();
        irrFactory.addAxes();
    }

    /// set the fog mode
    void SimContext::SetFog()
    {
        mIrr.getVideoDriver()->setFog();
    }

    /// Kill the current context
    void SimContext::KillGame()
    {
        AssertMsg( mIrr.getDevice(), "Invalid irrlicht device" );
        mIrr.getDevice()->closeDevice();
    }

    /// Python interface method for adding a python controlled camera to the context
    CameraPtr SimContext::AddCamera( float32_t rotateSpeed, float32_t moveSpeed, float32_t zoomSpeed )
    {
        mpCamera.reset( new Camera( mIrr, rotateSpeed, moveSpeed, zoomSpeed ) );
        return mpCamera;
    }

    void SimContext::AddLightSource( const Vector3f& position, float32_t radius, const SColor& color )
    {
        mIrr.getSceneManager()->addLightSceneNode(NULL, ConvertNeroToIrrlichtPosition(position), color, radius);
    }

    /**
     * Add a sky box to display a background view. This requires six images: up, down, left, right, front and back.
     * The images are assumed to be of the form box_base_name + x + extension, where x is "_up.", "_dn.", "_lf.",
     * "_rt.", "_ft." and "_bk." respectively.
     * @param box_base_name base name of the images - converted using findResource internally
     * @param extension image file extesion (such as jpg or png)
     */
    void SimContext::AddSkyBox( const std::string& box_base_name, const std::string& extension )
    {
        ITexture* up = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_up." + extension).c_str() );
        ITexture* dn = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_dn." + extension).c_str() );
        ITexture* lf = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_lf." + extension).c_str() );
        ITexture* rt = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_rt." + extension).c_str() );
        ITexture* ft = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_ft." + extension).c_str() );
        ITexture* bk = mIrr.getVideoDriver()->getTexture( Kernel::findResource(box_base_name + "_bk." + extension).c_str() );
        if ( !( up && dn && lf && rt && ft && bk ) )
        {
            LOG_F_ERROR( "graphics", "could not load a sky box texture starting with '" << box_base_name << "' and with extension '" << extension << "'.");
        }
        mpSkyBox = mIrr.getSceneManager()->addSkyBoxSceneNode(up, dn, lf, rt, ft, bk);
    }

    GuiManagerPtr SimContext::GetGuiManager()
    {
        Assert( mpGuiManager );
        return mpGuiManager;
    }

    void SimContext::SetInputMapping( const PyIOMap& ioMap )
    {
        mIOMap = ioMap;
    }

    CameraPtr SimContext::getActiveCamera() const
    {
        return mpCamera;
    }

    /// onPop - destruction code for teh state
    bool SimContext::onPop()
    {
        // --- clear out our stuff
        FlushContext();

        // shut down the AI system
        AIManager::instance().destroy();
        
        // forget everything imported in script
        ScriptingEngine::instance().destroy();

        // forget about the nero scene node
        mpNeroNode.reset();

        return true;
    }

    static void DrawText( const Vector2i& topLeft, const SColor& color, const std::string& msg, IrrlichtDevice_IPtr device )
    {
        Assert( device );

        const std::wstring wMsg = boost::lexical_cast<std::wstring>(msg.c_str());

        // grab the drawing tools we need as well
        irr::gui::IGUIFont* font = device->getGUIEnvironment()->getFont( "common/data/gui/fonthaettenschweiler.bmp" );

        irr::core::dimension2du optionsSize = font->getDimension(wMsg.c_str());

        irr::core::rect<int32_t> msgPos;
        msgPos.UpperLeftCorner.X = topLeft.X;
        msgPos.UpperLeftCorner.Y = topLeft.Y;
        msgPos.LowerRightCorner.X = topLeft.X + optionsSize.Width;
        msgPos.LowerRightCorner.Y = topLeft.Y + optionsSize.Height;

        font->draw( wMsg.c_str(), msgPos, color, false, false, 0 );
    }

	const ISceneManager_IPtr SimContext::GetSceneManager()
	{
		return mIrr.getSceneManager();
	}

    irr::gui::IGUIFont* SimContext::GetFont()
    {
        return mIrr.getDevice()->getGUIEnvironment()->getFont( "common/data/gui/fonthaettenschweiler.bmp" );
    }

    /// Update all the objects and render
    /// @param dt the time to increment by
    void SimContext::ProcessTick(float32_t dt)
    {
        // This will cause Irrlicht to render the objects
        UpdateRenderSystem(dt);
        
        // clear our lineset
        LineSet::instance().ClearSegments();

        // This will look at any input from the user that happened since the 
        // previous call and run the corresponding (Python) actions. This can
        // potentially change a lot of things such as which mod we want to run.
        UpdateInputSystem(dt);

        // Call the ProcessTick method of the global AI manager
        AIManager::instance().ProcessTick(dt);

        // This will loop through all the objects in the simulation, calling
        // their ProcessTick method. We need to know the actual position of
        // each object before this, and we will know the desired position after this.
        UpdateSimulation(dt);

        // This will trigger scheduled events in the Python script,
        // as well as ModTick(dt) if it is defined
        UpdateScriptingSystem(dt);
    }
    
    /// Update all the objects and render
    /// @param dt the time to increment by
    void SimContext::ProcessAnimationTick(float32_t dt, float32_t frac)
    {
        // This will cause Irrlicht to render the objects
        UpdateRenderSystem(dt);

        // This will look at any input from the user that happened since the 
        // previous call and run the corresponding (Python) actions. This can
        // potentially change a lot of things such as which mod we want to run.
        UpdateInputSystem(dt);
        
        // update the simulation
        if( mpSimulation )
        {
            mpSimulation->ProcessAnimationTick(frac);
        }
    }
    
    /// Update the input system
    void SimContext::UpdateInputSystem(float32_t dt)
    {
        if( mInputReceiver == kIR_Game )
        {
            // act on any user input
            mIOMap.ActOnUserInput();
        }
    }

    /// Update the rendering system
    void SimContext::UpdateRenderSystem(float32_t dt)
    {
        // draw the scene
        static bool sClearBackBuffer = true;
        static bool sClearZBuffer    = true;

        mIrr.getVideoDriver()->beginScene( sClearBackBuffer, sClearZBuffer, mClearColor );
        mIrr.getSceneManager()->drawAll();
        mIrr.getGuiEnv()->drawAll();

        mFPSCounter.registerFrame();
        std::stringstream sstr;
        sstr << mFPSCounter.getFPS();
        DrawText( Vector2i( 780, 5 ), SColor(255,255,255,255), sstr.str(), mIrr.getDevice() );

        mIrr.getVideoDriver()->endScene();

    }

    /// Update the scripting system by a bit
    void SimContext::UpdateScriptingSystem(float32_t dt)
    {
        // update the scripting scheduler
        ScriptingEngine::instance().GetScheduler().ProcessEvents();
        
        ScriptingEngine::instance().Tick(dt);
    }

    /// Update the simulation
    void SimContext::UpdateSimulation(float32_t dt)
    {
        // update the simulation
        if( mpSimulation )
        {
            mpSimulation->ProcessTick(dt);
        }
	}

    /// clear out data stored within the sim context
    void SimContext::FlushContext()
    {
        if( mpSimulation )
            mpSimulation->clear();

        // clear out our object templates and factory
        mpFactory.reset();
        mObjectTemplates.clear();

        // clear the irrlicht structures
        if( mIrr.getSceneManager() )
            mIrr.getSceneManager()->clear();

        // remove all of our elements from the gui
        if( mpGuiManager )
            mpGuiManager->RemoveAll();
    }
    
    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return the SimEntity intersected first by the ray from the camera origin through the view plane
    SimEntityPtr SimContext::GetClickedEntity(const int32_t& x, const int32_t& y)
    {
        Pos2i pos(x,y);
        // get scene node
        ISceneNode* node = mIrr.getSceneManager()->getSceneCollisionManager()->getSceneNodeFromScreenCoordinatesBB(pos);
        SimEntityPtr result;
        if (node != NULL)
        {
            result = getSimulation()->FindBySceneObjectId(node->getID());
        }
        else
        {
            LOG_F_WARNING("core", "unable to find clicked entity at location " << x << ", " << y);
        }
        return result;
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return the Id of the SimEntity intersected first by the ray from the camera origin through the view plane
    SimId SimContext::GetClickedEntityId(const int32_t& x, const int32_t& y)
    {
        SimEntityPtr ent = GetClickedEntity(x, y);
        if (ent) {
            return ent->GetSimId();
        } else {
            return 0;
        }
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return 3d ray from the camera origin through the view plane at the screen coordinates (note, this is PAST the actual click coord)
    Line3f SimContext::GetRayUnderMouse(const int32_t& x, const int32_t& y)
    {
        Pos2i pos(x,y);
        // get ray
        Line3f line = mIrr.getSceneManager()->getSceneCollisionManager()->getRayFromScreenCoordinates(pos);
        // convert to NERO coords
        line.setLine(ConvertIrrlichtToNeroPosition(line.start), ConvertIrrlichtToNeroPosition(line.end));
        return line;
    }

    /// @param hitEntity the entity intersected by the ray
    /// @param hitPos the position of the intersection
    /// @param origin origin of the ray to cast
    /// @param target target of the ray to cast
    /// @param type bitmask of the objects to care about or 0 for 'check all'
    /// @param vis show rays?
    /// @param foundColor the color to use if vis is true and an intersection is found
    /// @param noneColor the color to use if vis is true 
    /// @return first intersection info tuple(sim, hit) with SimEntityData sim and Vector3f hit (hit location) or ()
    /// Find the first object that intersects the specified ray
    bool SimContext::FindInRay( SimEntityData& hitEntity,
                                Vector3f& hitPos,
                                const Vector3f& origin, 
                                const Vector3f& target, 
                                const uint32_t& type, 
                                const bool vis, 
                                const SColor& foundColor,
                                const SColor& noneColor
                              )
	{
        ISceneCollisionManager* collider = mIrr.getSceneManager()->getSceneCollisionManager();
        Assert(collider);
        Line3f ray(ConvertNeroToIrrlichtPosition(origin), ConvertNeroToIrrlichtPosition(target));
        Triangle3f outTriangle;
        ISceneNode* node = collider->getSceneNodeAndCollisionPointFromRay
            (ray, hitPos, outTriangle, type);
        // convert back into our coord system
        hitPos = ConvertIrrlichtToNeroPosition(hitPos);
        if (node && node->getID() >= kFirstSimId)
        {
            // we found a sim node, so return its data
            SimEntityPtr ent = getSimulation()->FindBySceneObjectId(node->getID());
            if (ent)
            {
                // return the result: (sim, hit)
                hitEntity = ent->GetState();
                // draw a ray if requested
                if (vis)
                {
                    LineSet::instance().AddSegment(origin, hitPos, foundColor);
                }
                return true;
            }
        }
        if (vis)
        {
            LineSet::instance().AddSegment(origin, target, noneColor);
        }
        return false;
	}
    
    /// Find the first object that intersects the specified ray
    boost::python::tuple SimContext::PyFindInRay( const Vector3f& origin, 
                                      const Vector3f& target, 
                                      const uint32_t& type, 
                                      const bool val, 
                                      const SColor& foundColor,
                                      const SColor& noneColor
                                    )
    {
        SimEntityData hitEntity;
        Vector3f hitPos;
        if (FindInRay(hitEntity, hitPos, origin, target, type, val, foundColor, noneColor))
        {
            return boost::python::make_tuple(hitEntity, hitPos);
        }
        else
        {
            return boost::python::make_tuple();
        }        
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return Approximate 3d position of the click
    Vector3f SimContext::GetClickedPosition(const int32_t& x, const int32_t& y)
    {
        Pos2i pos(x,y);
        ISceneCollisionManager* collider = mIrr.getSceneManager()->getSceneCollisionManager();
        // get ray
        Line3f ray = collider->getRayFromScreenCoordinates(pos);
        LOG_F_DEBUG("collision", "screen coordinates: " << pos << ", ray: " << ray.start << " - " << ray.end << ", length: " << ray.getLength());
        Vector3f collision_point;
        Triangle3f collision_triangle;
        ISceneNode* collision = collider->getSceneNodeAndCollisionPointFromRay(ray, collision_point, collision_triangle);
        if (collision) {
            SimEntityPtr ent = mpSimulation->FindBySceneObjectId(collision->getID());
            LOG_F_DEBUG("collision", "screen coordinates: " << pos << " colliding with: " << ent << " at: " << collision_point);
            return ConvertIrrlichtToNeroPosition(collision_point);
        } else {
            LOG_F_WARNING("collision", "screen coordinates: " << pos << " did not collide with any geometry!");
            return ConvertIrrlichtToNeroPosition(ray.end);
        }
    }

    /// @return the current position of the mouse
    Pos2i SimContext::GetMousePosition()
    {
        return mIOMap.GetMousePosition();
    }

    const std::string kCouldNotFindObjectWithId("Could not find object with id: ");

    /// @brief Sets the position of the SimEntity specified by id
    void SimContext::SetObjectPosition( SimId id, const Vector3f& pos ) 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->SetPosition(pos);
    }

    /// @brief Sets the rotation of the SimEntity specified by id
    void SimContext::SetObjectRotation( SimId id, const Vector3f& rot ) 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->SetRotation(rot);
    }

    /// @brief Sets the scale of the SimEntity specified by id
    void SimContext::SetObjectScale( SimId id, const Vector3f& scale ) 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->SetScale(scale);
    }

    /// @brief Sets the label of the SimEntity specified by id
    void SimContext::SetObjectLabel( SimId id, const std::string& label ) 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->SetLabel(label);
    }

    /// @brief Sets the override color of the SimEntity specified by id
    void SimContext::SetObjectColor( SimId id, const SColor& color ) 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->SetColor(color);
    }

	/// @brief Sets the animation on the SimEntity specified by id
	bool SimContext::SetObjectAnimation( SimId id, const std::string& animationType )
	{
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
		return ent->GetSceneObject()->SetAnimation(animationType);
	}
    
    void SimContext::SetObjectAnimationSpeed( SimId id, float32_t framesPerSecond  )
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->GetSceneObject()->SetAnimationSpeed(framesPerSecond);
    }

    /// Push the changes made to the object without animation
    void SimContext::UpdateObjectImmediately( SimId id )
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        ent->UpdateImmediately();
    }

    /// @brief Return the position of the SimEntity specified by id
    Vector3f SimContext::GetObjectPosition( SimId id ) const 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetPosition();
    }

    /// @brief Return the rotation of the SimEntity specified by id
    Vector3f SimContext::GetObjectRotation( SimId id ) const 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetRotation();
    }

    /// @brief Return the scale of the SimEntity specified by id
    Vector3f SimContext::GetObjectScale( SimId id ) const 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetScale();
    }

    /// @brief Return the label of the SimEntity specified by id
    std::string SimContext::GetObjectLabel( SimId id ) const 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetLabel();
    }

    /// @brief Return the override color of the SimEntity specified by id
    SColor SimContext::GetObjectColor( SimId id ) const 
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetColor();
    }
    
    std::string SimContext::GetObjectAnimation( SimId id ) const
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetSceneObject()->GetAnimation();
    }
    
    F32 SimContext::GetObjectAnimationSpeed( SimId id ) const
    {
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
        return ent->GetSceneObject()->GetAnimationSpeed();
    }

    /// @brief Return the bounding box min edge of the SimEntity specified by id
    Vector3f SimContext::GetObjectBBMinEdge( uint32_t id ) const {
        return mpSimulation->Find(id)->GetSceneObject()->getBoundingBox().MinEdge;
    }

    /// @brief Return the bounding box max edge of the SimEntity specified by id
    Vector3f SimContext::GetObjectBBMaxEdge( uint32_t id ) const {
        return mpSimulation->Find(id)->GetSceneObject()->getBoundingBox().MaxEdge;
    }

    /// @brief Transform the given vector by the matrix of the SimEntity specified by id
    Vector3f SimContext::TransformVector( uint32_t id, const Vector3f& vect ) const {
        return mpSimulation->Find(id)->GetSceneObject()->transformVector(vect);
    }

} //end OpenNero
