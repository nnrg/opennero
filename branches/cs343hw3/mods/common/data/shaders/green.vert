
// unused - to be removed at a later time...
uniform mat4 mWorldViewProj;
uniform mat4 mInvWorld;
uniform mat4 mTransWorld;
uniform vec3 mLightPos;
uniform vec4 mLightColor;

void main(void)
{
	gl_Position = mWorldViewProj * gl_Vertex;	
}
