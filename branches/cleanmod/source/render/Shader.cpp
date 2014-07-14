//--------------------------------------------------------
// OpenNero : Shader
//  A shader
//--------------------------------------------------------

#include "core/Common.h"
#include "Shader.h"

namespace OpenNero 
{
    using namespace irr;
    using namespace irr::core;
    using namespace irr::scene;
    using namespace irr::video;
    using namespace irr::gui;



	ShaderCallback::ShaderCallback( const IrrHandles_Weak& handles )
		: mIrrHandles(handles)
	{}

    /// called when shader's constants are set
	void ShaderCallback::OnSetConstants( video::IMaterialRendererServices* services, s32 userData )
	{
		IVideoDriver* driver	  = mIrrHandles.mpVideoDriver;

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.
		core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		invWorld.makeInverse();

		services->setVertexShaderConstant("mInvWorld", &invWorld[0], 16);    

		// set clip matrix
		core::matrix4 worldViewProj;
		worldViewProj = driver->getTransform(video::ETS_PROJECTION);			
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);

		services->setVertexShaderConstant("mWorldViewProj", &worldViewProj[0], 16);

		// set camera position
		Assert( mIrrHandles.mpSceneManager );
		Assert( mIrrHandles.mpSceneManager->getActiveCamera() );
		core::vector3df pos = mIrrHandles.mpSceneManager->getActiveCamera()->getAbsolutePosition();		
		services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);		

		// set light color 
		video::SColorf col(0.0f,1.0f,0.0f,0.0f);		
		services->setVertexShaderConstant("mLightColor", reinterpret_cast<f32*>(&col), 4);
		
		// set transposed world matrix
		core::matrix4 world = driver->getTransform(video::ETS_WORLD);
		world = world.getTransposed();		
		services->setVertexShaderConstant("mTransWorld", &world[0], 16);		
	}


	/**
	 * Have irrlicht load and create a glsl shader for us
	 * @param irr the irrlicht handles we may need to use
	 * @param callback the shader callback to use for this shader
	 * @param vertexProgram .vert file
	 * @param vertexMain the entrance function to the vertex program
	 * @param pixelProgram the .frag file
	 * @param pixelMain the entrance function to the vertex program
	 * @param baseMaterial the material for irrlicht to use as a base
	 * @return -1 if failed, index to material if succeeded
	*/
	int32_t createGlslShader(	IrrHandles& irr, ShaderCallback* callback,
								const std::string& vertexProgram, const std::string& vertexMain,
								const std::string& pixelProgram , const std::string& pixelMain,
								E_MATERIAL_TYPE baseMaterial )
	{
		Assert( callback );

		IVideoDriver_IPtr driver = irr.getVideoDriver();
        
        Assert(driver);

		if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) &&
			!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1))
			return -1;	
	
		if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1) &&
			!driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))
			return -1;

        Assert( driver );
        
		video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();			

		if (gpu)
		{	
			// create material from high level shaders (hlsl or glsl)

			int32_t newMat = gpu->addHighLevelShaderMaterialFromFiles(
								vertexProgram.c_str(), vertexMain.c_str(), video::EVST_VS_1_1,
								pixelProgram.c_str(), pixelMain.c_str(), video::EPST_PS_1_1,
								callback, baseMaterial );

			// let irrlicht store this, not us
			callback->drop();

			return newMat;
		}

		// nope, this shader won't work
		return -1;
	}
    
} //end OpenNero
