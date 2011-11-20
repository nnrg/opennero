//--------------------------------------------------------
// OpenNero : IrrFactory
//  irrilicht factory
//--------------------------------------------------------

#include "core/Common.h"
#include <irrlicht.h>

#include "game/objects/PropertyMap.h" // keep TinyXml from complaining

#include "core/Common.h"
#include "core/IrrSerialize.h"
#include "game/factories/IrrFactory.h"
#include "game/factories/SimFactory.h"
#include "game/Kernel.h"

#include "render/Shader.h"

#include "core/LogConnections.h"
#include "core/Log.h"

namespace OpenNero
{
    namespace
    {
        /// Scalable axes with R,G,B for X,Y,Z respectively
        class AxesSceneNode : public scene::ISceneNode
        {
            scene::SMeshBuffer ZMeshBuffer;
            scene::SMeshBuffer YMeshBuffer;
            scene::SMeshBuffer XMeshBuffer;

            video::SColor ZColor;
            video::SColor YColor;
            video::SColor XColor;

        public:
            AxesSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);

            virtual ~AxesSceneNode()
            {}

            virtual void OnRegisterSceneNode();

            virtual void render();
            void setAxesCoordinates();

            virtual const core::aabbox3d<f32>& getBoundingBox() const
            {
                return ZMeshBuffer.BoundingBox;
            }

            void setAxesScale(f32 scale);
        };


    }

    /// CTOR
    IrrFactory::IrrFactory( const IrrHandles& irr )
            : mIrr(irr)
    {}

    /// DTOR
    IrrFactory::~IrrFactory()
    {}

    /**
     * Load an Irrlicht model file
     * @param modelFile the file to load from
     * @return a ptr to the IAnimatedMesh class
    */
    IAnimatedMesh* IrrFactory::LoadAniMesh( const std::string& modelFile )
    {
        Assert( mIrr.mpSceneManager );
        return mIrr.mpSceneManager->getMesh( SimFactory::TransformPath(modelFile).c_str() );
    }

    /**
     * Load an Irrlicht texture file
     * @param textureFile the file to load from
     * @return a void ptr to the ITexture class
    */
    ITexture* IrrFactory::LoadTexture( const std::string& textureFile )
    {
        Assert( mIrr.mpVideoDriver );

        // load the texture
        ITexture* tex = mIrr.mpVideoDriver->getTexture( SimFactory::TransformPath(textureFile).c_str() );

        if( tex )
        {
            LOG_D_MSG( "factory_resource_log", "Loaded texture " << textureFile );
        }

        return tex;
    }

    IAnimatedMeshSceneNode* IrrFactory::addAnimatedMeshSceneNode( IAnimatedMesh* mesh )
    {
        Assert( mesh );
        Assert( mIrr.mpSceneManager );
        return mIrr.mpSceneManager->addAnimatedMeshSceneNode( mesh );
    }

    ISceneNode* IrrFactory::addAxes()
    {
        AxesSceneNode* scene = new AxesSceneNode(mIrr.mpSceneManager->getRootSceneNode(), mIrr.mpSceneManager.get(), -1);
        Assert(scene);
        scene->setAxesScale(10);
        scene->setScale(vector3df(5,5,5));
        return scene;
    }


    ITerrainSceneNode* IrrFactory::addTerrainSceneNode( const std::string& heightmap )
    {
        Assert( mIrr.mpSceneManager );

        return mIrr.mpSceneManager->addTerrainSceneNode( SimFactory::TransformPath(heightmap).c_str() );
    }

    IParticleSystemSceneNode* IrrFactory::addParticleSystemNode( const std::string& particleSystemFile )
    {
        Assert( mIrr.mpSceneManager );

        PropertyMap propMap;

        if( propMap.constructPropertyMap( SimFactory::TransformPath(particleSystemFile) ) )
        {
            // default values
            const vector3df pos( 0, 0, 0 );
            const vector3df rot( 0, 0, 0 );
            const vector3df scale( 1, 1, 1 );

            // create the particle system node
            IParticleSystemSceneNode* pSystem = mIrr.mpSceneManager->addParticleSystemSceneNode( false, 0, -1, pos, rot, scale );
            Assert( pSystem );

            // read some custom properties
            bool             globalMovement = true;
            dimension2df     particleSize( 5.0f, 5.0f );

            propMap.getValue( globalMovement, "ParticleSystem.System.GlobalMovement" );
            propMap.getValue( particleSize,   "ParticleSystem.System.ParticleSize" );

            pSystem->setParticlesAreGlobal(globalMovement);
            pSystem->setParticleSize(particleSize);

            /// ---- load the effectors ----

            // get the gravity properties
            if( propMap.hasSection( "ParticleSystem.Gravity" ) )
            {
                vector3df gravDir( 0, -1, 0 );
                uint32_t affectorTimeMS(1000);

                // try to read params
                propMap.getValue( gravDir,   "ParticleSystem.Gravity.Direction" );
                propMap.getValue( affectorTimeMS, "ParticleSystem.Gravity.AffectorTimeMS" );

                gravDir = ConvertNeroToIrrlichtPosition(gravDir);

                IParticleAffector* pGravity = pSystem->createGravityAffector( gravDir, affectorTimeMS );
                Assert( pGravity );

                pSystem->addAffector( pGravity );
                pGravity->drop();
            }

            // get the fade properties
            if( propMap.hasSection( "ParticleSystem.Fade" ) )
            {
                SColor targetColor(0,0,0,0);
                uint32_t affectorTimeMS(1000);

                // try to read params
                propMap.getValue( targetColor,  "ParticleSystem.Fade.TargetColor" );
                propMap.getValue( affectorTimeMS, "ParticleSystem.Fade.AffectorTimeMS" );

                IParticleAffector* pFade = pSystem->createFadeOutParticleAffector( targetColor, affectorTimeMS );
                Assert( pFade );

                pSystem->addAffector( pFade );
                pFade->drop();
            }


            // ---- load the emitters ----
            // --- NOTE: WE ONLY DO BOX EMITTERS ---

            if( propMap.hasSection( "ParticleSystem.Emitter" ) )
            {
                // default params
                aabbox3df box(-10, 28,-10, 10, 30, 10);
                vector3df dir( 0, .03f, 0 );
                uint32_t  minParticlesPerSecond = 5;
                uint32_t  maxParticlesPerSecond = 10;
                SColor    minStartColor( 255, 0, 0, 0 );
                SColor    maxStartColor( 255, 255, 255, 255 );
                uint32_t  lifeTimeMin = 2000;
                uint32_t  lifeTimeMax = 4000;
                int32_t   maxAngleDegrees = 0;

                // try to read custom params

                propMap.getValue( box,      "ParticleSystem.Emitter.Box" );
                propMap.getValue( dir,      "ParticleSystem.Emitter.Direction" );
                propMap.getValue( minParticlesPerSecond, "ParticleSystem.Emitter.MinParticlesPerSecond" );
                propMap.getValue( maxParticlesPerSecond, "ParticleSystem.Emitter.MaxParticlesPerSecond" );
                propMap.getValue( minStartColor,   "ParticleSystem.Emitter.MinStartColor" );
                propMap.getValue( maxStartColor,   "ParticleSystem.Emitter.MaxStartColor" );
                propMap.getValue( lifeTimeMin,    "ParticleSystem.Emitter.LifetimeMin" );
                propMap.getValue( lifeTimeMax,    "ParticleSystem.Emitter.LifetimeMax" );
                propMap.getValue( maxAngleDegrees,   "ParticleSystem.Emitter.MaxAngleDegrees" );

                // do coordinate system transformations
                dir = ConvertNeroToIrrlichtPosition(dir);

                // create the emitter
                IParticleEmitter* emit = pSystem->createBoxEmitter
                                         ( box,
                                           dir,
                                           minParticlesPerSecond,
                                           maxParticlesPerSecond,
                                           minStartColor,
                                           maxStartColor,
                                           lifeTimeMin,
                                           lifeTimeMax,
                                           maxAngleDegrees );
                Assert( emit );

                pSystem->setEmitter( emit );
                emit->drop();
            }
            else
            {
                LOG_F_WARNING( "render", "No emitter in particle system file - " << particleSystemFile );
            }

            return pSystem;
        }

        return NULL;
    }

	ITextSceneNode* IrrFactory::addTextSceneNode(const std::string& text, const SColor& color, const Vector3f pos, ISceneNode* parent)
	{
        Assert( mIrr.mpSceneManager );

		ITextSceneNode* node = mIrr.mpSceneManager->addTextSceneNode(mIrr.mpIrrDevice->getGUIEnvironment()->getFont( "common/data/gui/fonthaettenschweiler.bmp" ), L"TEST", color, parent, pos, 1);
		Assert(node);
		return node;
	}

    /**
     * Load a glsl shader into the cache.
     * @param vertFile the file containing the vertex program
     * @param fragFile the file containing the fragment program
     * @return-1 if failed, otherwise the index of the irrlicht material
    */
    int32_t IrrFactory::LoadGlslShader( const std::string& vertFile, const std::string& fragFile )
    {
        // combine the names to do lookup
        std::string lookupName = vertFile+fragFile;

        // do we already have this cache?
        ShaderMap::iterator itr = mShaderCache.find(lookupName);
        if( itr != mShaderCache.end() )
        {
            return itr->second;
        }

        // create the callback (dont delete, irrlicht will manage it for us)
        ShaderCallback* cb = new ShaderCallback(mIrr);

        // convert the files to mod paths
        std::string modVertFile = SimFactory::TransformPath(vertFile);
        std::string modFragFile = SimFactory::TransformPath(fragFile);

        // attempt to create the shader
        int32_t shader = createGlslShader( mIrr, cb, modVertFile, "main", modFragFile, "main", EMT_SOLID );

        // if the load succeeded, cache it for future calls
        if( shader >= 0 )
        {
            mShaderCache[lookupName] = shader;
        }

        return shader;
    }

    void IrrFactory::addSphere(
        F32 radius,
        const vector3df& position,
        const vector3df& rotation,
        const vector3df& scale)
    {
        mIrr.mpSceneManager->addSphereSceneNode(
            radius, // size of the sphere
            16, // poly-count
            mIrr.mpSceneManager->getRootSceneNode(), // parent node
            -1, // id
            position, rotation, scale); // pose
    }

    namespace
    {

        AxesSceneNode::AxesSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id): ISceneNode(parent, mgr, id)
        {
#ifdef _DEBUG
            setDebugName("AxesSceneNode");
#endif

            u16 u[36] = { 0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,   7,3,0,   7,6,3,   3,5,2,   3,6,5,   0,1,4,   0,4,7,};
            ZMeshBuffer.Indices.set_used(36);
            YMeshBuffer.Indices.set_used(36);
            XMeshBuffer.Indices.set_used(36);

            // Color Settings
            // jsheblak - note i fiddle with these to meet our OpenNero->Irrlicht coordinate frame conversion expectations
            XColor = video::SColor(255,255,0,0);
            YColor = video::SColor(255,0,0,255);
            ZColor = video::SColor(255,0,255,0);

            for (s32 i=0; i<36; ++i)
            {
                ZMeshBuffer.Indices[i] = u[i];
                YMeshBuffer.Indices[i] = u[i];
                XMeshBuffer.Indices[i] = u[i];
            }
            // Default Position, Rotation and Scale
            this->setPosition(core::vector3df(0,0,0));
            this->setRotation(core::vector3df(0,0,0));
            this->setScale(core::vector3df(1,1,1));
            // Axes Box Coordinates Settings
            setAxesCoordinates();
        }

        void AxesSceneNode::OnRegisterSceneNode()
        {
            if (IsVisible)
            {
                SceneManager->registerNodeForRendering(this);
                ISceneNode::OnRegisterSceneNode();
            }
        }

        void AxesSceneNode::render()
        {
            video::IVideoDriver* driver = SceneManager->getVideoDriver();
            driver->setMaterial(ZMeshBuffer.Material);
            driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
            driver->drawMeshBuffer(&ZMeshBuffer);

            driver->setMaterial(YMeshBuffer.Material);
            driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
            driver->drawMeshBuffer(&YMeshBuffer);

            driver->setMaterial(XMeshBuffer.Material);
            driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
            driver->drawMeshBuffer(&XMeshBuffer);
        }

        void AxesSceneNode::setAxesCoordinates()
        {
            ZMeshBuffer.Vertices.set_used(8);
            ZMeshBuffer.Material.Wireframe = false;
            ZMeshBuffer.Material.Lighting = false;
            ZMeshBuffer.Vertices[0]  = video::S3DVertex(-0.25,-0.25,0, -1,-1,-1, ZColor, 0, 1);
            ZMeshBuffer.Vertices[1]  = video::S3DVertex(0.25,-0.25,0,  1,-1,-1, ZColor, 1, 1);
            ZMeshBuffer.Vertices[2]  = video::S3DVertex(0.25,0.25,0,  1, 1,-1, ZColor, 1, 0);
            ZMeshBuffer.Vertices[3]  = video::S3DVertex(-0.25,0.25,0, -1, 1,-1, ZColor, 0, 0);
            ZMeshBuffer.Vertices[4]  = video::S3DVertex(0.25,-0.25,25,  1,-1, 1, ZColor, 0, 1);
            ZMeshBuffer.Vertices[5]  = video::S3DVertex(0.25,0.25,25,  1, 1, 1, ZColor, 0, 0);
            ZMeshBuffer.Vertices[6]  = video::S3DVertex(-0.25,0.25,25, -1, 1, 1, ZColor, 1, 0);
            ZMeshBuffer.Vertices[7]  = video::S3DVertex(-0.25,-0.25,25, -1,-1, 1, ZColor, 1, 1);
            ZMeshBuffer.BoundingBox.reset(0,0,0);

            YMeshBuffer.Vertices.set_used(8);
            YMeshBuffer.Material.Wireframe = false;
            YMeshBuffer.Material.Lighting = false;
            YMeshBuffer.Vertices[0]  = video::S3DVertex(-0.25,0,0.25, -1,-1,-1, YColor, 0, 1);
            YMeshBuffer.Vertices[1]  = video::S3DVertex(0.25,0,0.25,  1,-1,-1, YColor, 1, 1);
            YMeshBuffer.Vertices[2]  = video::S3DVertex(0.25,0,-0.25,  1, 1,-1, YColor, 1, 0);
            YMeshBuffer.Vertices[3]  = video::S3DVertex(-0.25,0,-0.25, -1, 1,-1, YColor, 0, 0);
            YMeshBuffer.Vertices[4]  = video::S3DVertex(0.25,25,0.25,  1,-1, 1, YColor, 0, 1);
            YMeshBuffer.Vertices[5]  = video::S3DVertex(0.25,25,-0.25,  1, 1, 1, YColor, 0, 0);
            YMeshBuffer.Vertices[6]  = video::S3DVertex(-0.25,25,-0.25, -1, 1, 1, YColor, 1, 0);
            YMeshBuffer.Vertices[7]  = video::S3DVertex(-0.25,25,0.25, -1,-1, 1, YColor, 1, 1);
            YMeshBuffer.BoundingBox.reset(0,0,0);

            XMeshBuffer.Vertices.set_used(8);
            XMeshBuffer.Material.Wireframe = false;
            XMeshBuffer.Material.Lighting = false;
            XMeshBuffer.Vertices[0]  = video::S3DVertex(0,-0.25,0.25, -1,-1,-1, XColor, 0, 1);
            XMeshBuffer.Vertices[1]  = video::S3DVertex(0,-0.25,-0.25,  1,-1,-1, XColor, 1, 1);
            XMeshBuffer.Vertices[2]  = video::S3DVertex(0,0.25,-0.25,  1, 1,-1, XColor, 1, 0);
            XMeshBuffer.Vertices[3]  = video::S3DVertex(0,0.25,0.25, -1, 1,-1, XColor, 0, 0);
            XMeshBuffer.Vertices[4]  = video::S3DVertex(25,-0.25,-0.25,  1,-1, 1, XColor, 0, 1);
            XMeshBuffer.Vertices[5]  = video::S3DVertex(25,0.25,-0.25,  1, 1, 1, XColor, 0, 0);
            XMeshBuffer.Vertices[6]  = video::S3DVertex(25,0.25,0.25, -1, 1, 1, XColor, 1, 0);
            XMeshBuffer.Vertices[7]  = video::S3DVertex(25,-0.25,0.25, -1,-1, 1, XColor, 1, 1);
            XMeshBuffer.BoundingBox.reset(0,0,0);
        }
        void AxesSceneNode::setAxesScale(f32 scale)
        {
            ZMeshBuffer.Vertices.set_used(8);
            ZMeshBuffer.Material.Wireframe = false;
            ZMeshBuffer.Material.Lighting = false;
            ZMeshBuffer.Vertices[0]  = video::S3DVertex(-0.25,-0.25,0, -1,-1,-1, ZColor, 0, 1);
            ZMeshBuffer.Vertices[1]  = video::S3DVertex(0.25,-0.25,0,  1,-1,-1, ZColor, 1, 1);
            ZMeshBuffer.Vertices[2]  = video::S3DVertex(0.25,0.25,0,  1, 1,-1, ZColor, 1, 0);
            ZMeshBuffer.Vertices[3]  = video::S3DVertex(-0.25,0.25,0, -1, 1,-1, ZColor, 0, 0);
            ZMeshBuffer.Vertices[4]  = video::S3DVertex(0.25,-0.25,scale,  1,-1, 1, ZColor, 0, 1);
            ZMeshBuffer.Vertices[5]  = video::S3DVertex(0.25,0.25,scale,  1, 1, 1, ZColor, 0, 0);
            ZMeshBuffer.Vertices[6]  = video::S3DVertex(-0.25,0.25,scale, -1, 1, 1, ZColor, 1, 0);
            ZMeshBuffer.Vertices[7]  = video::S3DVertex(-0.25,-0.25,scale, -1,-1, 1, ZColor, 1, 1);
            ZMeshBuffer.BoundingBox.reset(0,0,0);

            YMeshBuffer.Vertices.set_used(8);
            YMeshBuffer.Material.Wireframe = false;
            YMeshBuffer.Material.Lighting = false;
            YMeshBuffer.Vertices[0]  = video::S3DVertex(-0.25,0,0.25, -1,-1,-1, YColor, 0, 1);
            YMeshBuffer.Vertices[1]  = video::S3DVertex(0.25,0,0.25,  1,-1,-1, YColor, 1, 1);
            YMeshBuffer.Vertices[2]  = video::S3DVertex(0.25,0,-0.25,  1, 1,-1, YColor, 1, 0);
            YMeshBuffer.Vertices[3]  = video::S3DVertex(-0.25,0,-0.25, -1, 1,-1, YColor, 0, 0);
            YMeshBuffer.Vertices[4]  = video::S3DVertex(0.25,scale,0.25,  1,-1, 1, YColor, 0, 1);
            YMeshBuffer.Vertices[5]  = video::S3DVertex(0.25,scale,-0.25,  1, 1, 1, YColor, 0, 0);
            YMeshBuffer.Vertices[6]  = video::S3DVertex(-0.25,scale,-0.25, -1, 1, 1, YColor, 1, 0);
            YMeshBuffer.Vertices[7]  = video::S3DVertex(-0.25,scale,0.25, -1,-1, 1, YColor, 1, 1);
            YMeshBuffer.BoundingBox.reset(0,0,0);

            XMeshBuffer.Vertices.set_used(8);
            XMeshBuffer.Material.Wireframe = false;
            XMeshBuffer.Material.Lighting = false;
            XMeshBuffer.Vertices[0]  = video::S3DVertex(0,-0.25,0.25, -1,-1,-1, XColor, 0, 1);
            XMeshBuffer.Vertices[1]  = video::S3DVertex(0,-0.25,-0.25,  1,-1,-1, XColor, 1, 1);
            XMeshBuffer.Vertices[2]  = video::S3DVertex(0,0.25,-0.25,  1, 1,-1, XColor, 1, 0);
            XMeshBuffer.Vertices[3]  = video::S3DVertex(0,0.25,0.25, -1, 1,-1, XColor, 0, 0);
            XMeshBuffer.Vertices[4]  = video::S3DVertex(scale,-0.25,-0.25,  1,-1, 1, XColor, 0, 1);
            XMeshBuffer.Vertices[5]  = video::S3DVertex(scale,0.25,-0.25,  1, 1, 1, XColor, 0, 0);
            XMeshBuffer.Vertices[6]  = video::S3DVertex(scale,0.25,0.25, -1, 1, 1, XColor, 1, 0);
            XMeshBuffer.Vertices[7]  = video::S3DVertex(scale,-0.25,0.25, -1,-1, 1, XColor, 1, 1);
            XMeshBuffer.BoundingBox.reset(0,0,0);
        }
    };

}
;//end OpenNero
