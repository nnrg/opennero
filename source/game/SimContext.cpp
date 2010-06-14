//--------------------------------------------------------
// OpenNero : SimContext
//  the context of a simulated world
//--------------------------------------------------------

#include "core/Common.h"

#include "game/SimContext.h"
#include "game/SimEntity.h"
#include "game/Simulation.h"
#include "game/objects/TemplatedObject.h"

#if NERO_BUILD_PHYSICS
#include "physics/Physics.h"
#endif // NERO_BUILD_PHYSICS

#include "gui/GuiManager.h"

#include "render/SceneObject.h"
#include "render/Camera.h"
#include "render/NeroDrawNode.h"
#include "render/LineSet.h"

#include "factories/SimFactory.h"

#if NERO_BUILD_AUDIO
#include "audio/Ale.h"
#include "audio/AudioManager.h"
#endif // NERO_BUILD_AUDIO

#include "ai/AIManager.h"

#include "scripting/scriptIncludes.h"

#include "utils/Performance.h"
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
        Assert( device );

        // make copies of the irr handles
        mIrr.mpIrrDevice = device;
        mIrr.mpVideoDriver = IVideoDriver_IPtr(mIrr.mpIrrDevice->getVideoDriver() );
        mIrr.mpSceneManager = ISceneManager_IPtr( mIrr.mpIrrDevice->getSceneManager()->createNewSceneManager(), false);
        Assert(mIrr.mpSceneManager);
        mIrr.mpGuiEnv = IGuiEnvironment_IPtr(mIrr.mpIrrDevice->getGUIEnvironment() );

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
#if NERO_BUILD_PHYSICS
        IPhysicsEngine::instance().init();
#endif // NERO_BUILD_PHYSICS

        // reset the factory
        mpFactory.reset(new SimFactory( mIrr ));
        Assert( mpFactory );

        // create the gui manager
        mpGuiManager.reset( new GuiManager( mIrr, mpFactory ) );

        // add a custom draw node to the scene manager
        NeroDrawNode* neroNode = new NeroDrawNode( mIrr.mpSceneManager->getRootSceneNode(), mIrr.mpSceneManager.get(), -1);
        Assert( neroNode );

        // initialize the mod from script
        ScriptingEngine& scriptEngine = ScriptingEngine::instance();
        scriptEngine.ExecFile(Kernel::findResource("main.py"));
        scriptEngine.Call( "ModMain" );

        return true;
    }

    /// @param templateName location of the XML template for the object
    /// @param pos initial position of the object
    /// @param rot initial rotation (Euler angle) of the object
    /// @param scale initial scale of the object
    /// @param label initial text label of the object
    /// @param type initial type of the object (has to be smaller than BITMASK_SIZE
    /// @return SimId of the newly added object
    SimId SimContext::AddObject( const std::string& templateName, 
                                const Vector3f& pos, 
                                const Vector3f& rot, 
                                const Vector3f& scale,
                                const std::string& label,
                                uint32_t type)
    {
        Assert( mpSimulation );

        // initialize the creation data
        SimId new_id = GetNextFreeId();
        SimEntityData data(pos, rot, scale, label, type, new_id);
        data.SetDirtyBits();
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
        mIrr.mpVideoDriver->setFog();
    }

    /// Kill the current context
    void SimContext::KillGame()
    {
        AssertMsg( mIrr.mpIrrDevice, "Invalid irrlicht device" );
        mIrr.mpIrrDevice->closeDevice();
    }

    /// Python interface method for adding a python controlled camera to the context
    CameraPtr SimContext::AddCamera( float32_t rotateSpeed, float32_t moveSpeed, float32_t zoomSpeed )
    {
        mpCamera.reset( new Camera( mIrr, rotateSpeed, moveSpeed, zoomSpeed ) );
        return mpCamera;
    }

    void SimContext::AddLightSource( const Vector3f& position, float32_t radius, const SColor& color )
    {
        mIrr.mpSceneManager->addLightSceneNode(NULL, ConvertNeroToIrrlichtPosition(position), color, radius);
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
        ITexture* up = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_up." + extension).c_str() );
        ITexture* dn = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_dn." + extension).c_str() );
        ITexture* lf = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_lf." + extension).c_str() );
        ITexture* rt = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_rt." + extension).c_str() );
        ITexture* ft = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_ft." + extension).c_str() );
        ITexture* bk = mIrr.mpVideoDriver->getTexture( Kernel::findResource(box_base_name + "_bk." + extension).c_str() );
        if ( !( up && dn && lf && rt && ft && bk ) )
        {
            LOG_F_ERROR( "graphics", "could not load a sky box texture starting with '" << box_base_name << "' and with extension '" << extension << "'.");
        }
        mpSkyBox = mIrr.mpSceneManager->addSkyBoxSceneNode(up, dn, lf, rt, ft, bk);
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

        // clear the old scripts
        ScriptingEngine::instance().destroy();

        // shut down the AI system
        AIManager::instance().destroy();

#if NERO_BUILD_PHYSICS
        // destroy the physical world (warning: potentially dangerous =)
        IPhysicsEngine::instance().destroy();
#endif // NERO_BUILD_PHYSICS

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
		return mIrr.mpSceneManager;
	}

    irr::gui::IGUIFont* SimContext::GetFont()
    {
        return mIrr.mpIrrDevice->getGUIEnvironment()->getFont( "common/data/gui/fonthaettenschweiler.bmp" );
    }

    /// Update all the objects and render
    /// @param dt the time to increment by
    void SimContext::ProcessTick(float32_t dt)
    {
        UpdateInputSystem(dt);
        UpdateSimulation(dt);
        UpdateScriptingSystem(dt);

#if NERO_BUILD_PHYSICS
        // udpate the physics world
        IPhysicsEngine::instance().step(dt);
#endif // NERO_BUILD_PHYSICS

        // update our local systems
#if NERO_BUILD_AUDIO
        UpdateAudioSystem(dt);
#endif // NERO_BUILD_AUDIO
        UpdateRenderSystem(dt);
    }

#if NERO_BUILD_AUDIO
    /// Update the audio manager system
    void SimContext::UpdateAudioSystem(float32_t dt)
    {
        NERO_PERF_EVENT_SCOPED( SimContext__UpdateAudioSystem );

        // update the audio system
        AudioManager& audioMan = AudioManager::instance();

        if( mpCamera )
        {
            Vector3f soundDir = mpCamera->getTarget() - mpCamera->getPosition();
            soundDir = soundDir.normalize();

            audioMan.SetListenerPosition( mpCamera->getPosition());
            audioMan.SetListenerOrientation( soundDir, Vector3f(0,0,1) );
            audioMan.SetListenerVelocity( Vector3f(0,0,0) );
            audioMan.ProcessTick(dt);
        }
    }
#endif // NERO_BUILD_AUDIO

    /// Update the input system
    void SimContext::UpdateInputSystem(float32_t dt)
    {
        NERO_PERF_EVENT_SCOPED( SimContext__UpdateInputSystem );

        if( mInputReceiver == kIR_Game )
        {
            // act on any user input
            mIOMap.ActOnUserInput();
        }
    }

    /// Update the rendering system
    void SimContext::UpdateRenderSystem(float32_t dt)
    {
        NERO_PERF_EVENT_SCOPED( SimContext__UpdateRenderSystem );

        // draw the scene
        Assert( mIrr.mpVideoDriver );
        Assert( mIrr.mpSceneManager );
        Assert( mIrr.mpGuiEnv );

        static bool sClearBackBuffer = true;
        static bool sClearZBuffer    = true;

        mIrr.mpVideoDriver->beginScene( sClearBackBuffer, sClearZBuffer, mClearColor );

        mIrr.mpSceneManager->drawAll();
        mIrr.mpGuiEnv->drawAll();

        uint32_t y = 35;

        float32_t fps = (dt > 0 )? 1.0f/dt : 1000.0f;
        std::stringstream sstr;
        sstr << "FPS: " << fps;
        DrawText( Vector2i( 5, y ), SColor(255,255,255,255), sstr.str(), mIrr.mpIrrDevice );
        y += 20;

        mIrr.mpVideoDriver->endScene();
    }

    /// Update the scripting system by a bit
    void SimContext::UpdateScriptingSystem(float32_t dt)
    {
        // update the scripting scheduler
        ScriptingEngine::instance().GetScheduler().ProcessEvents();
    }

    /// update the local simulations
    void SimContext::UpdateSimulation(float32_t dt)
    {
        // update the simulation
        if( mpSimulation )
            mpSimulation->ProcessWorld(dt);
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
        if( mIrr.mpSceneManager )
            mIrr.mpSceneManager->clear();

        // remove all of our elements from the gui
        if( mpGuiManager )
            mpGuiManager->RemoveAll();
    }
    
    SimId SimContext::GetNextFreeId() const
    {
        SimId result = mpSimulation->GetNextFreeId();
        return result;
    }
        
    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return the SimEntity intersected first by the ray from the camera origin through the view plane
    SimEntityPtr SimContext::GetEntityUnderMouse(const int32_t& x, const int32_t& y) const
    {
        Pos2i pos(x,y);
        // get scene node
        ISceneNode* node = mIrr.mpSceneManager->getSceneCollisionManager()->getSceneNodeFromScreenCoordinatesBB(pos);
        SimEntityPtr result;
        if (node != NULL)
        {
            result = getSimulation()->FindBySceneObjectId(node->getID());
        }
        return result;
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return the Id of the SimEntity intersected first by the ray from the camera origin through the view plane
    SimId SimContext::GetEntityIdUnderMouse(const int32_t& x, const int32_t& y) const
    {
        return GetEntityUnderMouse(x, y)->GetSimId();
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return 3d ray from the camera origin through the view plane at the screen coordinates (note, this is PAST the actual click coord)
    Line3f SimContext::GetRayUnderMouse(const int32_t& x, const int32_t& y) const
    {
        Pos2i pos(x,y);
        // get ray
        Line3f line = mIrr.mpSceneManager->getSceneCollisionManager()->getRayFromScreenCoordinates(pos);
        // convert to NERO coords
        line.setLine(ConvertIrrlichtToNeroPosition(line.start), ConvertIrrlichtToNeroPosition(line.end));
        return line;
    }

#if NERO_BUILD_PHYSICS
    SimDataVector SimContext::FindInSphere( const Vector3f& origin, F32 radius ) const
    {
        return getSimulation()->findInSphere(origin, radius);
    }
#endif // NERO_BUILD_PHYSICS

    /// @param origin origin of the ray to cast
    /// @param target target of the ray to cast
    /// @param type bitmask of the objects to care about or 0 for 'check all'
    /// @param vis show rays?
    /// @return first intersection info tuple(sim, hit) with SimEntityData sim and Vector3f hit (hit location) or ()
    boost::python::tuple SimContext::FindInRay(const Vector3f& origin, 
                                               const Vector3f& target,
                                               const uint32_t& type,
                                               const bool vis) const
	{
        namespace py = boost::python;
        ISceneCollisionManager* collider = mIrr.mpSceneManager->getSceneCollisionManager();
        Assert(collider);
        Line3f ray(ConvertNeroToIrrlichtPosition(origin), ConvertNeroToIrrlichtPosition(target));
        Vector3f outPosition;
        Triangle3f outTriangle;
        ISceneNode* node = collider->getSceneNodeAndCollisionPointFromRay
            (ray, outPosition, outTriangle, type);
        // convert back into our coord system
        outPosition = ConvertIrrlichtToNeroPosition(outPosition);
        // TODO: the color should be a parameter along with vis
        static const LineSet::LineColor kYellow( 255, 255, 255, 0 );
        static const LineSet::LineColor kRed( 255, 255, 0, 0 );
        if (node)
        {
            // we found a sim node, so return its data
            SimEntityPtr ent = getSimulation()->FindBySceneObjectId(node->getID());
            AssertMsg(ent, "Could not figure out the entity for id: " << node->getID());
            if (ent)
            {
                // draw a ray if requested
                if(vis)
                {
                    LineSet::instance().AddSegment(origin, outPosition, kRed);
                }
                // return the result: (sim, hit)
                return py::make_tuple(ent->GetState(), outPosition);
            }
        } else {
            if (vis)
            {
                LineSet::instance().AddSegment(origin, target, kYellow );
            }
        }
        return py::make_tuple();
	}

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return Approximate 3d position of the click
    Vector3f SimContext::GetClickPosition(const int32_t& x, const int32_t& y) const
    {
        // TODO: hide this Irrlicht-specific code
        Pos2i pos(x,y);
        ISceneCollisionManager* collider = mIrr.mpSceneManager->getSceneCollisionManager();
        Assert(collider != NULL);
        // get ray
        Line3f ray = collider->getRayFromScreenCoordinates(pos);
        Vector3f resultPosition;
        Triangle3f resultTriangle;
        ISceneNode* node = collider->getSceneNodeAndCollisionPointFromRay(ray, resultPosition, resultTriangle);
        if (node == NULL)
        {
            return ConvertIrrlichtToNeroPosition(ray.end);
        }
        else
        {
            return ConvertIrrlichtToNeroPosition(resultPosition);
        }
    }

    /// @param start Start point of the ray
    /// @param end End point of the ray
    /// @param id The id of the object to collide with
    /// @param outCollisionPoint The collision point of the ray with the object
    /// @return true if collision occurs; false otherwise
    bool SimContext::GetCollisionPoint(const Vector3f& start, const Vector3f& end, SimId id, Vector3f& outCollisionPoint) const
    {
        Line3f ray(ConvertNeroToIrrlichtPosition(start), ConvertNeroToIrrlichtPosition(end));

        if (mpSimulation->Find(id)->GetCollisionPoint(ray, mIrr, outCollisionPoint))
        {
            outCollisionPoint = ConvertIrrlichtToNeroPosition(outCollisionPoint);
            return true;
        }
        else
        {
            return false;
        }
    }

    /// @param x screen x-coordinate for active camera
    /// @param y screen y-coordinate for active camera
    /// @return the point of intersection of the 3d ray from the camera origin through the view plane
    Vector3f SimContext::GetPointUnderMouse(const int32_t& x, const int32_t& y) const
    {
        return GetRayUnderMouse(x, y).end;
    }

    /// @return the current position of the mouse
    Pos2i SimContext::GetMousePosition() const
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

	/// @breif Sets the animation on the SimEntity specified by id
	bool SimContext::SetObjectAnimation( SimId id, const std::string& animation_type )
	{
        SimEntityPtr ent = mpSimulation->Find(id);
        AssertMsg(ent, kCouldNotFindObjectWithId << id);
		return ent->GetSceneObject()->SetAnimation(animation_type);
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

    // --------------------------------------------------------------------
    //  Python Binding Procedures

    /// get the current simulation context
    SimContext& GetSimContext()
    {
        return *(Kernel::GetSimContext());
    }

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addObject_overloads, AddObject, 2, 6)

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addSkyBox_overloads, AddSkyBox, 1, 2)

    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLightSource_overloads, AddLightSource, 2, 3)
    
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(findInRay_overloads, FindInRay, 2, 4)

    PYTHON_BINDER( SimContext )
    {
        using namespace boost::python;            

        class_<SimContext>("SimContext", "The simulation context from an XML file", no_init )
            .def("addAxes",              &SimContext::AddAxes, "Add a set of Cartesian axes from an XML file")
            .def("setFog",               &SimContext::SetFog, "Set the fog mode")
            .def("addCamera",            &SimContext::AddCamera, "Create and add a camera to the context and return camera")
            .def("addLightSource",       &SimContext::AddLightSource, addLightSource_overloads("Add a light source to the scene"))
            .def("addSkyBox",            &SimContext::AddSkyBox, addSkyBox_overloads("Add a sky box consisting of 6 images starting with arg0 and ending with arg1"))
            .def("addObject",      &SimContext::AddObject, "Create an object on the server and broadcast to clients")
            .def("removeObject",   &SimContext::RemoveObject, "Remove an object from the server and broadcast to clients")
            .def("getGuiManager",        &SimContext::GetGuiManager, "Return the gui manager for the context")
            .def("killGame",             &SimContext::KillGame, "Kill the game")
            .def("setInputMapping",      &SimContext::SetInputMapping, "Set the io map to use" )
            .def("getNextFreeId",        &SimContext::GetNextFreeId, "Get the next available network ID" )
#if NERO_BUILD_PHYSICS
			.def("findInSphere",         &SimContext::FindInSphere, "Find all the objects within the specified sphere (origin:Vector3f, radius:float)" )
#endif // NERO_BUILD_PHYSICS
			.def("findInRay",            &SimContext::FindInRay, findInRay_overloads("Find all the objects within the specified ray (origin:Vector3f, target:Vector3f, [int])") )
            .def("getEntityIdUnderMouse", &SimContext::GetEntityIdUnderMouse, "Get the id of the entity intersected first by the ray from the camera origin through the view plane")
            .def("getClickPosition",    &SimContext::GetClickPosition, "Approximate 3d position of the mouse click")
            .def("getCollisionPoint",   &SimContext::GetCollisionPoint, "Find the collision point of the specified ray and object")
            .def("getPointUnderMouse",  &SimContext::GetPointUnderMouse, "Get the point of intersection of the 3d ray from the camera origin through the view plane")
            .def("getMousePosition",    &SimContext::GetMousePosition, "Get the current position of the mouse")
            .def("setObjectPosition",   &SimContext::SetObjectPosition, "Set the position of an object specified by its id")
            .def("setObjectRotation",   &SimContext::SetObjectRotation, "Set the rotation of an object specified by its id")
            .def("setObjectScale",      &SimContext::SetObjectScale, "Set the scale of an object specified by its id")
            .def("setObjectLabel",      &SimContext::SetObjectLabel, "Set the label of an object specified by its id")
            .def("setObjectColor",      &SimContext::SetObjectColor, "Set the color of an object specified by its id")
			.def("setObjectAnimation",  &SimContext::SetObjectAnimation, "Set the animation of the object specified by its id")
            .def("getObjectPosition",   &SimContext::GetObjectPosition, "Get the position of an object specified by its id")
            .def("getObjectRotation",   &SimContext::GetObjectRotation, "Get the rotation of an object specified by its id")
            .def("getObjectScale",      &SimContext::GetObjectScale, "Get the scale of an object specified by its id")
            .def("getObjectLabel",      &SimContext::GetObjectLabel, "Get the label of an object specified by its id")
            .def("getObjectColor",      &SimContext::GetObjectColor, "Get the color of an object specified by its id")
	    .def("getActiveCamera",	&SimContext::getActiveCamera, "Get the active camera")
        ;

        // this is how Python can access the C++ reference to SimContext
        def("getSimContext", &GetSimContext, return_value_policy<reference_existing_object>());
    }

} //end OpenNero
