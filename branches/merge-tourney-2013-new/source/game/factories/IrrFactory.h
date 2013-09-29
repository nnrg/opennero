//--------------------------------------------------------
// OpenNero : IrrFactory
//  irrilicht factory
//--------------------------------------------------------

#ifndef _GAME_FACTORIES_IRRFACTORY_H
#define _GAME_FACTORIES_IRRFACTORY_H

#include "core/IrrUtil.h"
#include "core/Common.h"

namespace OpenNero
{
    using namespace irr;
    using namespace irr::core;
    using namespace irr::scene;
    using namespace irr::video;

    namespace Log
    {
        class ILogConnection;
        BOOST_SHARED_DECL(ILogConnection);
    }

    /// Loads resources from Irrlicht
    class IrrFactory
    {
    public:

        IrrFactory( const IrrHandles& irr );
        ~IrrFactory();

        /// load a model in some fasion (caller will need to cast to format that is expected)
        IAnimatedMesh* LoadAniMesh( const std::string& meshFile );
        
        /// load a texture
        ITexture* LoadTexture( const std::string& textureFile );

        /// load an animated scene node from a mesh
        IAnimatedMeshSceneNode* addAnimatedMeshSceneNode( IAnimatedMesh* mesh );

        /// add a visible axis to the world
        ISceneNode* addAxes();

        /// add a sphere
        void addSphere(
            F32 size,
            const vector3df& position = vector3df(0,0,0),
            const vector3df& rotation = vector3df(0,0,0),
            const vector3df& scale = vector3df(1.0f,1.0f,1.0f)
        );

        /// create a terrain scene node from a heightmap
        ITerrainSceneNode* addTerrainSceneNode(const std::string& heightmap );

        // add a particle system from an xml file
        IParticleSystemSceneNode* addParticleSystemNode( const std::string& particleSystemFile );

		/// add a text scene node
		ITextSceneNode* addTextSceneNode( const std::string& text, 
			const SColor& color, 
			const Vector3f pos = Vector3f(0,0,0), 
			ISceneNode* parent = NULL );

        // load a glsl shader
        int32_t LoadGlslShader( const std::string& vertFile, const std::string& fragFile );

    private:

        typedef std::map< std::string, int32_t > ShaderMap;

    private:

        ShaderMap mShaderCache;                     ///< A Cache of all the shaders we have loaded
        IrrHandles mIrr;                           ///< Handles to do Irr Specific loads
    };

} //end OpenNero

#endif // _GAME_FACTORIES_IRRFACTORY_H
