#version 330

uniform struct Matrices
{
	mat4 mProjection;
	mat4 mModel;
	mat4 mView;                                                                           
	mat4 mNormal;
} matrices;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

smooth out vec2 vTexCoord;
smooth out vec3 vNormal;
smooth out vec3 vWorldPos;
smooth out vec4 vEyeSpacePos;

uniform mat4 HeightmapScaleMatrix;
uniform mat3 NormalScaleMatrix;

void main()
{
	vec4 inPositionScaled = HeightmapScaleMatrix*vec4(inPosition, 1.0);
	mat4 mMVP = matrices.mProjection*matrices.mView*matrices.mModel;
   
	gl_Position = mMVP*inPositionScaled;
	vEyeSpacePos = matrices.mView*matrices.mModel*vec4(inPosition, 1.0);
  
	vTexCoord = inCoord;
	vNormal = mat3(NormalScaleMatrix) * inNormal;
   
	vec4 vWorldPosLocal = matrices.mModel*inPositionScaled;
	vWorldPos = vWorldPosLocal.xyz;
  
}