//--------------------------------------------------------
// OpenNero : Shader
//  A shader
//--------------------------------------------------------

#ifndef _RENDER_SHADER_H_
#define _RENDER_SHADER_H_

#include "core/IrrUtil.h"
#include "core/Common.h"

namespace OpenNero 
{
    using namespace irr;
    using namespace irr::core;
    using namespace irr::gui;
    using namespace irr::video;

	/**
	 * An object callback functor that is called during the constants setting
	 * portion of a shader pass. Inherit this to define your own shaders
	*/

	/// WE COULD PROBABLY DO WITHOUT THIS INDIRECTION, just put all this is child classes...
	class ShaderCallback : public IShaderConstantSetCallBack	
	{
	public:

		/// constructor
		ShaderCallback( const IrrHandles_Weak& handles );		

		/// constant callback (REMOVE ME AND PLACE IN CHILD)
		void OnSetConstants( IMaterialRendererServices* services, s32 userData );

	private:

		// weak ptrs to avoid circular depedencies with Driver
		IrrHandles_Weak mIrrHandles;
	};
	
	// create a shader
	int32_t createGlslShader(	IrrHandles& irr, ShaderCallback* callback,
								const std::string& vertexProgram, const std::string& vertexMain,
								const std::string& pixelProgram , const std::string& pixelMain,
								E_MATERIAL_TYPE baseMaterial );

} //end OpenNero

#endif // _RENDER_SHADER_H_
